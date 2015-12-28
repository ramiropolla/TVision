/*
 * system.cc
 *
 * Copyright (c) 1998 Sergio Sigala, Brescia, Italy.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Modified by Sergey Clushin <serg@lamport.ru>, <Clushin@deol.ru> */
/* Modified by Dmitrij Korovkin <tkf@glasnet.ru> */
/* Modified by Ilfak Guilfanov, <ig@datarescue.com> */

#ifdef __LINUX__
#define Uses_TButton
#define Uses_TColorSelector
#define Uses_TDeskTop
#define Uses_TDirListBox
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TEventQueue
#define Uses_TFrame
#define Uses_THistory
#define Uses_TIndicator
#define Uses_TKeys
#define Uses_TListViewer
#define Uses_TMenuBox
#define Uses_TOutlineViewer
#define Uses_TScreen
#define Uses_TScrollBar
#define Uses_TStatusLine
#define Uses_TProgram
#include <tv.h>

#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <errno.h>

#define qnumber(x) (sizeof(x)/sizeof(x[0]))

static FILE *fp;        // output screen handle
static FILE *fpkb;      // input keyboard handle

static bool inited = false;
static struct termios old_in_termios;
static struct termios new_in_termios;
static struct termios old_out_termios;
static struct termios new_out_termios;

static int lastMods;
static int current_color;
static int current_page;        // 0-G0, 1-G1


#include <term.h>
#undef buttons

/*
 * This is the delay in ms before the first evMouseAuto is generated when the
 * user holds down a mouse button.
 */
#define DELAY_AUTOCLICK_FIRST   400

/*
 * This is the delay in ms between next evMouseAuto events.  Must be greater
 * than DELAY_SIGALRM (see below).
 */
#define DELAY_AUTOCLICK_NEXT    100

/*
 * This is the delay in ms between consecutive SIGALRM signals.  This
 * signal is used to generate evMouseAuto and cmSysWakeup events.
 */
#define DELAY_SIGALRM           100

/*
 * This broadcast event is used to update the StatusLine.
 */
#define DELAY_WAKEUP            200

/*
   ms to get the continuation of an escape sequence
*/

#define DELAY_ESC               100

static FILE *logfp = NULL;
void LOG(const char *format, ...)
{
  if ( logfp != NULL )
  {
    va_list va;
    va_start(va, format);
    vfprintf(logfp, format, va);
    va_end(va);
    fprintf(logfp, "\n");
  }
}

static void error(const char *format, ...)
{
  va_list va;
  va_start(va, format);
  vfprintf(stderr, format, va);
  va_end(va);
  abort();
}

static int qgetch(void)
{
  int c = fgetc(fpkb);
#if 0
  if ( c != -1 )
  {
    printf("c=%x\n", c);
    LOG("IN: %x %c\n", c, c);
  }
#endif
  return c;
}

inline void outstr(const char *str)
{
  fputs(str, fp);
}

static void qclear(void)
{
  outstr("\E[22;37;40m\E[2J");
}

static void qbeep(void)
{
  outstr("\e\x07");
}

// show/hide cursor
static void qcurs_set(bool show)
{
  if ( show )
    outstr("\E[?25h");
  else
    outstr("\E[?25l");
}

static char *add(char *ptr, char chr)
{
  if ( ptr[-1] != ';' && ptr[-1] != '[' )
    *ptr++ = ';';
  sprintf(ptr, "%d", chr);
  ptr = strchr(ptr, 0);
  return ptr;
}

static int switch_color_to_buf(int color, char *outbuf, int outbufsize)
{
  static const char map[] = {0, 4, 2, 6, 1, 5, 3, 7};

  int old_back = (current_color >> 4) & 15;
  int old_fore = (current_color >> 0) & 15;
  int back = (color >> 4) & 15;
  int fore = (color >> 0) & 15;

  char buf[20];
  char *ptr = buf;

  *ptr++ = '\e';
  *ptr++ = '[';

  if ( fore != old_fore )
  {
    if ( (fore ^ old_fore) & 8 )
      ptr = add(ptr, fore > 7 ? 1 : 22);
    ptr = add(ptr, 30+map[fore & 7]);
  }

  if ( back != old_back )
  {
    if ( (back ^ old_back) & 8 )
      ptr = add(ptr, back > 7 ? 5 : 25);
    ptr = add(ptr, 40+map[back & 7]);
  }

  *ptr++ = 'm';
  *ptr = '\0';

//    LOG("color: %s", buf);

  int outlen = strlen(buf);
  if (outlen > outbufsize - 1)
    error("switch_color() needs at least %d bytes buffer", outlen);
  else
    strcpy(outbuf, buf);

  current_color = color;
  return outlen;
}

inline int moveto_to_buf(int x, int y, char *buf, int bufsize)
{
  snprintf(buf, bufsize, "\e[%d;%dH", y+1, x+1);
  return strlen(buf);
}

inline void moveto(int x, int y)
{
  char tmp[128];
  moveto_to_buf(x, y, tmp, sizeof(tmp));
  outstr(tmp);
}

static int output_char_to_buf(int code, int color, char *buf, int bufsize)
{
  char *ptr = buf;

  /* Call this only when a color change is needed */
  /* It's a redundant check but helps profiling */
  if (color != current_color)
    ptr += switch_color_to_buf(color, buf, bufsize);

  if ( code < 0x20 )
  {
    struct trans { uchar o, n; };
    static const trans arrows[] =
    {
      { 0x07, 0xFE}, // thick dot
      { 0x10, '>' }, // thick right arrow
      { 0x11, '<' }, // thick left arrow
      { 0x17, 'o' }, // thin up-down arrow
      { 0x18, '^' }, // thin up arrow
      { 0x19, 'v' }, // thin down arrow
      { 0x1A, '<' }, // thin left arrow
      { 0x1B, '>' }, // thin right arrow
      { 0x1D, '-' }, // thin left-right arrow
      { 0x1E, '^' }, // thick up arrow
      { 0x1F, 'v' }, // thick down arrow
    };
    int i;
    for ( i=0; i < qnumber(arrows); i++ )
    {
      if ( arrows[i].o == code )
      {
        code = arrows[i].n;
        break;
      }
    }
    if ( i == qnumber(arrows) ) // display unknown chars as a thick dot
      code = 0xFE;
  }

  static const unsigned char trans[] =
  {
  /* B0 */ 0x61, 0x61, 0x61, 0x78, 0x75, 0x75, 0x75, 0x6B,
  /* B8 */ 0x6B, 0x75, 0x78, 0x6B, 0x6A, 0x6A, 0x6A, 0x6B,
  /* C0 */ 0x6D, 0x76, 0x77, 0x74, 0x71, 0x6E, 0x74, 0x74,
  /* C8 */ 0x6D, 0x6C, 0x76, 0x77, 0x74, 0x71, 0x6E, 0x76,
  /* D0 */ 0x76, 0x77, 0x77, 0x6D, 0x6D, 0x6C, 0x6C, 0x6E,
  /* D8 */ 0x6E, 0x6A, 0x6C, 0x20, 0x20, 0x20, 0x20, 0x20,
  /* E0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* E8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* F0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* F8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00,
  };

  if ( code < 0x40 || code >= 0x60 )
  {
    int need_page = 0;
    if ( code >= 0xB0 && code <= 0xFF && trans[code-0xB0] != 0 )
    {
      code = trans[code-0xB0];
      if ( code == 0x20 )
      {
        ptr = stpcpy(ptr, "\e[7m \e[27m");
        goto RET;
      }
      need_page = 1;
    }
    if ( need_page != current_page )
    {
      *ptr++ = 15 - need_page;
      current_page = need_page;
    }
  }
  *ptr++ = code;
  *ptr = '\0';
RET:
  int outlen = ptr - buf;
  if ( outlen >= bufsize - 1 )
    error("tv:output_char_to_buf() needs bigger buffer, please tell developers about it\n");
  return outlen;
}

ushort TEventQueue::doubleDelay = 8;
Boolean TEventQueue::mouseReverse = False;

ushort TScreen::screenMode;
int TScreen::screenWidth;
int TScreen::screenHeight;
ushort *TScreen::screenBuffer;
fd_set TScreen::fdSetRead;
fd_set TScreen::fdActualRead;

static TEvent *evIn;            /* message queue system */
static TEvent *evOut;
static TEvent evQueue[eventQSize];
static char env[PATH_MAX];      /* value of the TVOPT environment variable */
static int curX, curY;          /* current cursor coordinates */
static bool cursor_displayed;   /* is cursor visible? */
static int currentTime;         /* current timer value */
static int doRepaint;           /* should redraw the screen ? */
static int doResize;            /* resize screen ? */
static int evLength;            /* number of events in the queue */
static int msOldButtons;        /* mouse button status */
static TEvent msev;             /* last mouse event */

/*
 * A simple class which implements a timer.
 */

class Timer
{
        int limit;
public:
        Timer() { limit = -1; }
        int isExpired() { return limit != -1 && currentTime >= limit; }
        int isRunning() { return limit != -1; }
        void start(int timeout) { limit = currentTime + timeout; }
        void stop() { limit = -1; }
};

static Timer msAutoTimer;       /* time when generate next cmMouseAuto */
static Timer wakeupTimer;       /* time when generate next cmWakeup */
static Timer escTimer;          /* Esc sequence timer */

/*
 * GENERAL FUNCTIONS
 */

inline int range(int test, int min, int max)
{
        return test < min ? min : test > max ? max : test;
}

/*
 * KEYBOARD FUNCTIONS
 */

struct node
{
  char value;           // pressed key
  char islast;          // is last key
  union
  {
    struct
    {
      ushort tvcode;       // tvision code
      uchar mods;
    };
    ushort next;         // next node in the list
  };
};

static node keymap[500] = { {0} };   // array of nodes, first slot not used
static int keymap_size = 1;
static ushort *states[200] = { NULL }; // 1,2..., first slot not used
static int states_size = 1;

static ushort findstateidx(int state, char key)
{
  const ushort *nodes = states[state];
  if ( nodes != NULL )
  {
    while ( *nodes != 0 )
    {
      if ( keymap[*nodes].value == key )
        return *nodes;
      nodes++;
    }
  }
  return 0;
}

static void addstateidx(int state, ushort idx)
{
  ushort *p = states[state];
  size_t len = 0;
  if ( p != NULL )
  {
    for ( ushort *q=p; *q != 0; q++ )
      len++;
  }
  p = (ushort *)realloc(p, sizeof(ushort)*(len+2));
  // find the right place
  ushort *q = p;
  size_t i;
  char chr = keymap[idx].value;
  for ( i=0; i < len && keymap[*q].value < chr; i++ )
    q++;
  // make room
  int rest = len - i;
  if ( rest > 0 )
    memmove(q+1, q, rest*sizeof(ushort));
  *q = idx;
  p[len+1] = 0;
  states[state] = p;
}

static void addkey(const char *seq, ushort code, uchar mods)
{
  int state = 1;
  const char *saved = seq;
//  printf("%s => %02X%02X\n", seq, mods, code);
  while ( *seq )
  {
    char c = *seq++;
    ushort idx = findstateidx(state, c);
    if ( idx == 0 ) // not found
    {
      if ( keymap_size >= qnumber(keymap) )
        error("too many keys");
      idx = keymap_size++;
      keymap[idx].value = c;
      addstateidx(state, idx);
    }
    if ( *seq == 0 )
    {
      if ( !keymap[idx].islast && keymap[idx].next )
        error("short definition");
      if ( keymap[idx].islast )
      {
        if ( keymap[idx].tvcode != code || keymap[idx].mods != mods )
          error("duplicate definition %s", saved);
      }
      keymap[idx].islast = 1;
      keymap[idx].tvcode = code;
      keymap[idx].mods   = mods;
    }
    else
    {
      if ( keymap[idx].islast )
        error("long definition");
      state = keymap[idx].next;
      if ( state == 0 )
      {
        if ( states_size >= qnumber(states)-1 )
          error("too many states");
        state = ++states_size;
        keymap[idx].next = state;
      }
    }
  }
}

static void print_tree(void)
{
#if 0
  for ( int i=1; i < states_size; i++ )
  {
    printf("STATE %d\n", i);
    for ( ushort *pidx=states[i]; *pidx != 0; pidx++ )
    {
      ushort idx = *pidx;
      node *p = &keymap[idx];
      printf("\t%3d: CHAR: %c ", idx, p->value);
      if ( p->islast )
        printf(" CODE: %04X/%02X\n", p->tvcode, p->mods);
      else
        printf("              NEXT: %d\n", p->next);
    }
  }
#endif
}

const int kblNormal=0,kblShift=1,kblAltR=2,kblCtrl=4,kblAltL=8;
static const uchar xtMods[7]=
{
  kblShift,                     // 2
  kblAltL,                      // 3
  kblShift | kblAltL,           // 4
  kblCtrl,                      // 5
  kblShift | kblCtrl,           // 6
  kblCtrl | kblAltL,            // 7
  kblShift | kblCtrl | kblAltL  // 8
};

const uchar fgEterm=1, fgOnlyEterm=2;
struct stCsiKey
{
  uchar number;
  ushort code;
};

struct stEtKey
{
  uchar number;
  ushort code;
  uchar mods;
};

static const stCsiKey csiKeys1[]=
{
  {  1, kbHome },
  {  2, kbIns },
  {  3, kbDel },
  {  4, kbEnd },
  {  5, kbPgUp },
  {  6, kbPgDn },
  {  7, kbHome },
  {  8, kbEnd },
  { 15, kbF5 },
  { 17, kbF6 },
  { 18, kbF7 },
  { 19, kbF8 },
  { 20, kbF9 },
  { 21, kbF10 },
  { 23, kbF11 },
  { 24, kbF12 }
};
static const int cCsiKeys1 = qnumber(csiKeys1);

static const uchar csiFgKeys1[]=
{
  fgEterm,fgEterm,fgEterm,fgEterm,
  fgOnlyEterm|fgEterm,fgOnlyEterm|fgEterm, // Home & End
};

static const stCsiKey csiKeys2[]=
{
  { 'A', kbUp    },
  { 'B', kbDown  },
  { 'C', kbRight },
  { 'D', kbLeft  },
//  { 'E', kb5 }, // Key pad 5
  { 'F', kbEnd   },
  { 'H', kbHome  },
  { 'Z', kbShiftTab },
};
static const int cCsiKeys2 = qnumber(csiKeys2);

static const stCsiKey csiKeys3[]=
{
 { 'P', kbF1 },
 { 'Q', kbF2 },
 { 'R', kbF3 },
 { 'S', kbF4 }
};
static const int cCsiKeys3 = qnumber(csiKeys3);

static const stEtKey csiKeys4[]=
{
 { 'a', kbUp, kblCtrl },
 { 'b', kbDown, kblCtrl },
 { 'c', kbRight, kblCtrl },
 { 'd', kbLeft, kblCtrl },
 { 'A', kbUp, 0 },
 { 'B', kbDown, 0 },
 { 'C', kbRight, 0 },
 { 'D', kbLeft, 0 },
 { 'F', kbEnd, 0 },
 { 'H', kbHome, 0 },
 // Keypad
 { 'M', kbEnter, 0 },
// { 'j', kbAsterisk, 0 },
// { 'k', kbPlus, 0 },
// { 'm', kbMinus, 0 },
// { 'n', kbDelete, 0 },
// { 'o', kbSlash, 0 },
 { 'p', kbIns, 0 },
 { 'q', kbEnd, 0 },
 { 'r', kbDown, 0 },
 { 's', kbPgDn, 0 },
 { 't', kbLeft, 0 },
// { 'u', kb5, 0 },
 { 'v', kbRight, 0 },
 { 'w', kbHome, 0 },
 { 'x', kbUp, 0 },
 { 'y', kbPgUp, 0 }
};
static const int cCsiKeys4 = qnumber(csiKeys4);

static void build_tree(void)
{
  int i,j;
  char b[32];

  if ( keymap_size > 1 )
    return;

  for (i=0; i<cCsiKeys1; i++)
  {
    sprintf(b, "[%d~", csiKeys1[i].number);
    addkey(b, csiKeys1[i].code, 0);
    if ( (csiFgKeys1[i] & fgOnlyEterm) == 0 )
    for (j=0; j<7; j++)
    {
      sprintf(b,"[%d;%d~",csiKeys1[i].number,j+2);
      addkey(b, csiKeys1[i].code, xtMods[j]);
    }
    if (csiFgKeys1[i] & fgEterm)
    {
      sprintf(b,"[%d^",csiKeys1[i].number);
      addkey(b,csiKeys1[i].code,kblCtrl);
      sprintf(b,"[%d$",csiKeys1[i].number);
      addkey(b,csiKeys1[i].code,kblShift);
      sprintf(b,"[%d@",csiKeys1[i].number);
      addkey(b,csiKeys1[i].code,kblShift| kblCtrl);
    }
  }
  for (i=0; i<cCsiKeys2; i++)
  {
    sprintf(b,"[%c",csiKeys2[i].number);
    addkey(b,csiKeys2[i].code,0);
    for (j=0; j<7; j++)
    {
      sprintf(b,"[%d%c",j+2,csiKeys2[i].number);
      addkey(b,csiKeys2[i].code,xtMods[j]);
    }
  }
  for (i=0; i<cCsiKeys3; i++)
  {
   sprintf(b,"O%c",csiKeys3[i].number);
   addkey(b,csiKeys3[i].code,0);
   for (j=0; j<7; j++)
   {
     sprintf(b,"O%d%c",j+2,csiKeys3[i].number);
     addkey(b,csiKeys3[i].code,xtMods[j]);
   }
  }
  for (i=0; i<cCsiKeys4; i++)
  {
    sprintf(b,"O%c",csiKeys4[i].number);
    addkey(b,csiKeys4[i].code,csiKeys4[i].mods);
  }

  // Shift+arrows
  addkey("[a",kbUp,kblShift);
  addkey("[b",kbDown,kblShift);
  addkey("[c",kbRight,kblShift);
  addkey("[d",kbLeft,kblShift);
  // F1-F4
  addkey("[11~",kbF1,0);
  addkey("[12~",kbF2,0);
  addkey("[13~",kbF3,0);
  addkey("[14~",kbF4,0);
  // Ctrl+Fx
  addkey("[11^",kbF1,kblCtrl);
  addkey("[12^",kbF2,kblCtrl);
  addkey("[13^",kbF3,kblCtrl);
  addkey("[14^",kbF4,kblCtrl);
  addkey("[15^",kbF5,kblCtrl);
  addkey("[17^",kbF6,kblCtrl);
  addkey("[18^",kbF7,kblCtrl);
  addkey("[19^",kbF8,kblCtrl);
  addkey("[20^",kbF9,kblCtrl);
  addkey("[21^",kbF10,kblCtrl);
  // Shift+Fx
  // Shift F1 and F2 overlaps with F11 and F12
  addkey("[25~",kbF3,kblShift);
  addkey("[26~",kbF4,kblShift);
  addkey("[28~",kbF5,kblShift);
  addkey("[29~",kbF6,kblShift);
  addkey("[31~",kbF7,kblShift);
  addkey("[32~",kbF8,kblShift);
  addkey("[33~",kbF9,kblShift);
  addkey("[34~",kbF10,kblShift);
  addkey("[23$",kbF11,kblShift);
  addkey("[24$",kbF12,kblShift);
  // Ctrl+Shift+Fx
//  addkey("[23^",kbF1,kblCtrl | kblShift);
//  addkey("[24^",kbF2,kblCtrl | kblShift);
  addkey("[25^",kbF3,kblCtrl | kblShift);
  addkey("[26^",kbF4,kblCtrl | kblShift);
  addkey("[28^",kbF5,kblCtrl | kblShift);
  addkey("[29^",kbF6,kblCtrl | kblShift);
  addkey("[31^",kbF7,kblCtrl | kblShift);
  addkey("[32^",kbF8,kblCtrl | kblShift);
  addkey("[33^",kbF9,kblCtrl | kblShift);
  addkey("[34^",kbF10,kblCtrl | kblShift);
  addkey("[23@",kbF11,kblCtrl | kblShift);
  addkey("[24@",kbF12,kblCtrl | kblShift);
  addkey("[M", kbMouse, 0);

  print_tree();
}

//***************************************************************************
static int process_escape(int chr)
{
  // esc + escape sequence => alt + escape sequence
  int mods = 0;
  if ( chr == '\e' )
  {
    mods = kblAltL;
    chr = qgetch();
    if ( chr == EOF )     // Alt+ESC
    {
      lastMods = kbAltShift;
      return kbAltEsc;
    }
  }

  int state = 1;
  while ( true )
  {
    ushort idx = findstateidx(state, chr);
//    printf("state: %d chr=%c idx=%d\n", state, chr, idx);
    if ( idx == 0 )
    {
      mods = kblAltL;
      break;
    }

    if ( keymap[idx].islast )
    {
      chr = keymap[idx].tvcode;
      mods |= keymap[idx].mods;
      break;
    }

    state = keymap[idx].next;
    char c = qgetch();
    if ( c == -1 )
      break;
    chr = c;
  }
  lastMods = 0;
  if ( mods & kblShift ) lastMods |= kbShift;
  if ( mods & kblAltL  ) lastMods |= kbLeftAlt;
  if ( mods & kblAltR  ) lastMods |= kbRightAlt;
  if ( mods & kblCtrl  ) lastMods |= kbCtrlShift;
//  printf("return %x/%x\n", chr, lastMods);
  return chr;
}

/*
 * Gets information about modifier keys (Alt, Ctrl and Shift).  This can
 * be done only if the program runs on the system console.
 */

uchar getShiftState()
{
#ifdef __MACOSX__
	return 0;
#else
        int arg = 6;    /* TIOCLINUX function #6 */
        int shift = 0;

        if (ioctl(fileno(fpkb), TIOCLINUX, &arg) != -1)
        {
                if (arg & (2 | 8)) shift |= kbLeftAlt | kbRightAlt;
                if (arg & 4) shift |= kbLeftCtrl | kbRightCtrl;
                if (arg & 1) shift |= kbLeftShift | kbRightShift;
        }
        return shift;
#endif
}

struct trans_t
{
  ushort alt;
  ushort org;
};

static const trans_t shift_trans[] =
{
  { kbShiftIns, kbIns },
  { kbShiftDel, kbDel },
  { kbShiftTab, kbTab },
  { kbShiftF1 , kbF1  },
  { kbShiftF2 , kbF2  },
  { kbShiftF3 , kbF3  },
  { kbShiftF4 , kbF4  },
  { kbShiftF5 , kbF5  },
  { kbShiftF6 , kbF6  },
  { kbShiftF7 , kbF7  },
  { kbShiftF8 , kbF8  },
  { kbShiftF9 , kbF9  },
  { kbShiftF10, kbF10 },
  { kbShiftF11, kbF11 },
  { kbShiftF12, kbF12 },
};

static const trans_t alt_trans[] =
{
  { kbAltSpace      , ' '      },
  { kbAltQ          , 'Q'      },
  { kbAltW          , 'W'      },
  { kbAltE          , 'E'      },
  { kbAltR          , 'R'      },
  { kbAltT          , 'T'      },
  { kbAltY          , 'Y'      },
  { kbAltU          , 'U'      },
  { kbAltI          , 'I'      },
  { kbAltO          , 'O'      },
  { kbAltP          , 'P'      },
  { kbAltA          , 'A'      },
  { kbAltS          , 'S'      },
  { kbAltD          , 'D'      },
  { kbAltF          , 'F'      },
  { kbAltG          , 'G'      },
  { kbAltH          , 'H'      },
  { kbAltJ          , 'J'      },
  { kbAltK          , 'K'      },
  { kbAltL          , 'L'      },
  { kbAltZ          , 'Z'      },
  { kbAltX          , 'X'      },
  { kbAltC          , 'C'      },
  { kbAltV          , 'V'      },
  { kbAltB          , 'B'      },
  { kbAltN          , 'N'      },
  { kbAltM          , 'M'      },
  { kbAltF1         , kbF1     },
  { kbAltF2         , kbF2     },
  { kbAltF3         , kbF3     },
  { kbAltF4         , kbF4     },
  { kbAltF5         , kbF5     },
  { kbAltF6         , kbF6     },
  { kbAltF7         , kbF7     },
  { kbAltF8         , kbF8     },
  { kbAltF9         , kbF9     },
  { kbAltF10        , kbF10    },
  { kbAltF11        , kbF11    },
  { kbAltF12        , kbF12    },
  { kbAlt1          , '1'      },
  { kbAlt2          , '2'      },
  { kbAlt3          , '3'      },
  { kbAlt4          , '4'      },
  { kbAlt5          , '5'      },
  { kbAlt6          , '6'      },
  { kbAlt7          , '7'      },
  { kbAlt8          , '8'      },
  { kbAlt9          , '9'      },
  { kbAlt0          , '0'      },
  { kbAltMinus      , '-'      },
  { kbAltEqual      , '='      },
  { kbAltBack       , kbBack   },
//  { kbAltBksp       , kbBksp   },
  { kbAltEnter      , kbEnter  },
  { kbAltEsc        , kbEsc    },
  { kbAltTab        , kbTab    },
//  { kbAltKDiv       , kbKDiv   },
//  { kbAltKMul       , kbKMul   },
//  { kbAltKSub       , kbKSub   },
//  { kbAltKAdd       , kbKAdd   },
//  { kbAltKEnter     , kbKEnter },
  { kbAltLeft       , kbLeft   },
  { kbAltRight      , kbRight  },
  { kbAltUp         , kbUp     },
  { kbAltDown       , kbDown   },
  { kbAltOpenBraket , '['      },
  { kbAltCloseBraket, ']'      },
  { kbAltSemicolon  , ';'      },
  { kbAltApostrophe , '\''     },
  { kbAltBackApst   , '`'      },
  { kbAltBackslash  , '\\'     },
  { kbAltComma      , ','      },
  { kbAltDot        , '.'      },
  { kbAltSlash      , '/'      },
  { kbAltDel        , kbDel    },
  { kbAltEnd        , kbEnd    },
  { kbAltHome       , kbHome   },
  { kbAltIns        , kbIns    },
  { kbAltPgUp       , kbPgUp   },
  { kbAltPgDn       , kbPgDn   },
};

static const trans_t plain_trans[] =
{
  { kbEsc,   0x1b },
  { kbBack,  0x7f },
  { kbTab,   0x09 },
  { kbEnter, 0x0a },
};


static int apply_trans(int code, const trans_t *table, size_t size)
{
  int uc = toupper(code);
  for ( int i=0; i < size; i++ )
    if ( table[i].org == uc )
      return table[i].alt;
  return code;
}

struct scaninfo_t
{
  uchar ascii;
  uchar scan;
};

static const scaninfo_t scaninfo[] =
{
  { 0x8,  0x0E },       // backspace
  { 0x9,  0x0F },       // tab
  { 0xD,  0x1C },       // enter
  { 0x1B, 0x01 },       // esc
  { ' ',  0x39 },
  { '!',  0x02 },
  { '"',  0x28 },
  { '#',  0x04 },
  { '$',  0x05 },
  { '%',  0x06 },
  { '&',  0x08 },
  { '\'', 0x28 },
  { '(',  0x0A },
  { ')',  0x0B },
  { '*',  0x09 },
  { '+',  0x0D },
  { ',',  0x33 },
  { '-',  0x0C },
  { '.',  0x34 },
  { '/',  0x35 },
  { '0',  0x0B },
  { '1',  0x02 },
  { '2',  0x03 },
  { '3',  0x04 },
  { '4',  0x05 },
  { '5',  0x06 },
  { '6',  0x07 },
  { '7',  0x08 },
  { '8',  0x09 },
  { '9',  0x0A },
  { ':',  0x27 },
  { ';',  0x27 },
  { '<',  0x33 },
  { '=',  0x0D },
  { '>',  0x34 },
  { '?',  0x35 },
  { '@',  0x03 },
  { '[',  0x1A },
  { '\\', 0x2B },
  { ']',  0x1B },
  { '^',  0x07 },
  { '_',  0x0C },
  { '`',  0x29 },
  { 'a',  0x1E },
  { 'b',  0x30 },
  { 'c',  0x2E },
  { 'd',  0x20 },
  { 'e',  0x12 },
  { 'f',  0x21 },
  { 'g',  0x22 },
  { 'h',  0x23 },
  { 'i',  0x17 },
  { 'j',  0x24 },
  { 'k',  0x25 },
  { 'l',  0x26 },
  { 'm',  0x32 },
  { 'n',  0x31 },
  { 'o',  0x18 },
  { 'p',  0x19 },
  { 'q',  0x10 },
  { 'r',  0x13 },
  { 's',  0x1F },
  { 't',  0x14 },
  { 'u',  0x16 },
  { 'v',  0x2F },
  { 'w',  0x11 },
  { 'x',  0x2D },
  { 'y',  0x15 },
  { 'z',  0x2C },
  { '{',  0x1A },
  { '|',  0x2B },
  { '}',  0x1B },
  { '~',  0x29 },
};

static int apply_scaninfo(int code)
{
  if ( (code & 0xFF00) == 0 )
  {
    int uc = tolower(code);
    for ( size_t i=0; i < qnumber(scaninfo); i++ )
    {
      if ( scaninfo[i].ascii >= uc )
      {
        if ( scaninfo[i].ascii == uc )
          code |= (scaninfo[i].scan << 8);
        break;
      }
    }
  }
  return code;
}

static void msPutEvent(TEvent &event, int buttons, int flags, int what)
{
        event.mouse.buttons = 0;
        event.mouse.eventFlags = flags;
        event.what = what;
        if (TEventQueue::mouseReverse)  /* swap buttons ? */
        {
                if (buttons & mbLeftButton) event.mouse.buttons |=
                        mbRightButton;
                if (buttons & mbRightButton) event.mouse.buttons |=
                        mbLeftButton;
        }
        else event.mouse.buttons = buttons;     /* no swapping */
}

ulong getTicks(void)
{
//  struct timeval tv;
//  gettimeofday(&tv, NULL);
//  return (tv.tv_sec*100 + tv.tv_usec / 10000) * 18 / 100;
  return currentTime * 18 / 1000;
}

static void create_mouse_event(TEvent &ev)
{
  const int MouseB1Down=0x20,
            MouseB2Down=0x21,
            MouseB3Down=0x22,
            MouseB4Down=0x60,
            MouseB5Down=0x61,
            MouseUp=0x23;
  TPoint me;
  int event = qgetch();
  me.x = qgetch() - 0x21;
  me.y = qgetch() - 0x21;
  event &= ~0x1C;                               // Filter the modifiers
  if ( event>=0x60 ) // B4 and B5
    return;
  if ( event>=0x40 )
    event-=0x20; // motion values
  int flags = 0;
  static ulong lastTicks = 0;
  ulong nowTicks;
//  printf("event: %02X (%d,%d) buttons=%x\n", event, me.x, me.y, msOldButtons);
  switch ( event )
  {
    case MouseB1Down:
      event = (msOldButtons & mbLeftButton)
              ? (msev.mouse.where == me)
                ? evMouseAuto
                : evMouseMove
              : evMouseDown;
      msOldButtons |= mbLeftButton;
      if ( event == evMouseDown )
      {
        nowTicks = getTicks();
        if ( lastTicks != 0
          && (nowTicks-lastTicks) < TEventQueue::doubleDelay
          && msev.mouse.where == me )
        {
          lastTicks = 0;
          flags = meDoubleClick;
//          printf("double!\n");
        }
      }
      break;
    case MouseB2Down: // middle button
      return;
    case MouseB3Down:
      event = (msOldButtons & mbRightButton)
              ? (msev.mouse.where == me)
                ? evMouseAuto
                : evMouseMove
              : evMouseDown;
      msOldButtons |= mbRightButton;
      lastTicks = 0;
      break;
    case MouseUp:
      if ( msOldButtons == mbLeftButton )
        lastTicks = getTicks();
      msOldButtons = 0;
      event = evMouseUp;
      msAutoTimer.stop();
      break;
  }
  if ( event == evMouseMove )
  {
    flags |= meMouseMoved;
    msAutoTimer.start(DELAY_AUTOCLICK_NEXT);
  }
  if ( event == evMouseDown )
    msAutoTimer.start(DELAY_AUTOCLICK_FIRST);
  msev.mouse.controlKeyState = getShiftState();
  msev.mouse.where.x = range(me.x, 0, TScreen::screenWidth - 1);
  msev.mouse.where.y = range(me.y, 0, TScreen::screenHeight - 1);
//  printf("event: %d fl:%x (%d,%d) buttons:%x\n", event, flags, me.x, me.y, msOldButtons);
  msPutEvent(msev, msOldButtons, flags, event);
  ev = msev;
}

/*
 * Reads a key from the keyboard.
 */
void TScreen::get_key_mouse_event(TEvent &event)
{
  event.what = evNothing;

  /*
   * suspend until there is a signal or some data in file
   * descriptors
   *
   * if event_delay is zero, then do not suspend
   */
  if ( TProgram::event_delay != 0 )
  {
    fdActualRead = fdSetRead;
    select(FD_SETSIZE, &fdActualRead, NULL, NULL, NULL);
  }

  sigset_t alarmBlock, normalMask;

  sigemptyset(&alarmBlock);
  sigaddset(&alarmBlock, SIGALRM);

  /* see if there is data available */

  sigprocmask(SIG_BLOCK, &alarmBlock, &normalMask);
  int code = qgetch();
  sigprocmask(SIG_SETMASK, &normalMask, NULL);

  lastMods = 0;
  if ( code == -1 )
  {
    if ( !escTimer.isExpired() )
      return;
    escTimer.stop();
    code = '\e';
  }
  else
  {
    if ( escTimer.isRunning() )
    {
      escTimer.stop();
      code = process_escape(code);
    }
    else if ( code == 27 )
    {
      code = qgetch();
      if ( code == EOF )
      {
        escTimer.start(DELAY_ESC);  // within this period we have to get the rest
        return;
      }
      code = process_escape(code);
    }
  }

  if ( code == kbMouse )
  {
    create_mouse_event(event);
  }
  else
  {
    code = apply_trans(code, plain_trans, qnumber(plain_trans));
    int modifiers = getShiftState() | lastMods;
//    printf("modif=%x\n", modifiers);
    if ( modifiers & kbAltShift )
    {
      code = apply_trans(code, alt_trans, qnumber(alt_trans));
//      printf("apply_alt: %x\n", code);
    }
    if ( modifiers & kbShift )
    {
      code = apply_trans(code, shift_trans, qnumber(shift_trans));
//      printf("apply_shift: %x\n", code);
    }

    code = apply_scaninfo(code);
//    printf(" found: %x\n", code);
    LOG(" found: %x", code);
    event.what = evKeyDown;
    event.keyDown.keyCode = code;
    event.keyDown.controlKeyState = modifiers;
  }
}

static void selectPalette()
{
  TScreen::screenMode = TScreen::smCO80;
}

// try many times if the  system call is interrupted
// it happens on X11
static void qtcsetattr(int h, int func, struct termios *tm)
{
  for ( int i=0; i < 10; i++ )
  {
    if ( tcsetattr(h, func, tm) == 0 )
      break;
    if ( errno != EINTR )
      abort();
    usleep(100);
  }
}

static void startcurses()
{
  if ( inited )
    return;
  inited = true;

  build_tree();

  // SETUP INPUT
  fpkb = fopen(ttyname(fileno(stdin)),"r+b");
  if ( fpkb == NULL )
    abort();
  int h = fileno(fpkb);

  if ( tcgetattr(h,&old_in_termios) )
    abort();

  memcpy(&new_in_termios, &old_in_termios, sizeof(new_in_termios));
  new_in_termios.c_iflag |= (IGNBRK | BRKINT); // Ignore breaks
  new_in_termios.c_iflag &= ~(IXOFF | IXON);   // Disable Xon/off
  new_in_termios.c_lflag &= ~(ICANON | ECHO | ISIG); // Character oriented, no echo, no signals
  qtcsetattr(h, TCSAFLUSH,&new_in_termios);

  // set nonblocking mode
  int F = fcntl(h, F_GETFL, 0);
  fcntl(h, F_SETFL, F | O_NONBLOCK);

  // SETUP OUTPUT
  fp = stdout;
  setbuf(fp, NULL);
  h = fileno(fp);
  tcgetattr(h, &old_out_termios);

  outstr("\e)B\e)0\0x0f"); // set up character sets G0 and G1, select G0
  memcpy(&new_out_termios, &old_out_termios, sizeof(new_out_termios));
  new_out_termios.c_oflag |= OPOST;
  qtcsetattr(h, TCSAFLUSH, &new_out_termios);

  current_color = -1;
  current_page = 0;

  TScreen::drawCursor(0); /* hide the cursor */
  TScreen::drawMouse(1);  /* draw the mouse pointer */

  /* setup file descriptors */

  FD_ZERO(&TScreen::fdSetRead);
  FD_SET(fileno(fpkb), &TScreen::fdSetRead);

}

static void stopcurses()
{
  if ( !inited )
    return;
  inited = false;

  qclear();             /* blank the screen */
  outstr("\0x0f");      // select G0
  cursor_displayed = true;
  qcurs_set(true);
  moveto(0, TScreen::screenHeight-1);

  qtcsetattr(fileno(fpkb), TCSAFLUSH, &old_in_termios);
}

/*
 * Show a warning message.
 */

static int confirmExit()
{
        /* we need the buffer address */

        class MyBuffer: public TDrawBuffer
        {
        public:
                ushort *getBufAddr() { return data; }
        };
        MyBuffer b;
        static char msg[] = "Warning: are you sure you want to quit ?";

        b.moveChar(0, ' ', 0x4f, TScreen::screenWidth);
        b.moveStr(max((TScreen::screenWidth - (int) (sizeof(msg) - 1)) / 2,
                0), msg, 0x4f);
        TScreen::writeRow(0, b.getBufAddr(), TScreen::screenWidth);

//      timeout(-1);    /* set getch() in blocking mode */
        int key = qgetch();
//      timeout(0);     /* set getch() in non-blocking mode */
        return toupper(key) == 'Y';
}

static void freeResources()
{
        TScreen::drawMouse(0);
        stopcurses();
        delete[] TScreen::screenBuffer;
        LOG("terminated");
}

/*
 * General signal handler.
 */

static void sigHandler(int signo)
{
  struct sigaction dfl_handler;

  sigemptyset(&dfl_handler.sa_mask);
  dfl_handler.sa_flags = SA_RESTART;
  switch (signo)
  {
    case SIGALRM:
      /*
       * called every DELAY_SIGALRM ms
       */
      currentTime += DELAY_SIGALRM;
      break;
    case SIGCONT:
      /*
       * called when the user restart the process after a ctrl-z
       */
      LOG("continuing process");
      TScreen::resume();

      /* re-enable SIGTSTP */

      dfl_handler.sa_handler = sigHandler;
      sigaction(SIGTSTP, &dfl_handler, NULL);
      break;
    case SIGINT:
    case SIGQUIT:
      /*
       * These signals are now trapped.
       * Date: Wed, 12 Feb 1997 10:45:55 +0100 (MET)
       *
       * Ignore SIGINT and SIGQUIT to avoid recursive calls.
       */
      dfl_handler.sa_handler = SIG_IGN;
      sigaction(SIGINT, &dfl_handler, NULL);
      sigaction(SIGQUIT, &dfl_handler, NULL);

      /* ask the user what to do */

      if (confirmExit())
      {
              freeResources();        /* do a nice exit */
              exit(EXIT_FAILURE);
      }
      doRepaint++;

      /* re-enable SIGINT and SIGQUIT */

      dfl_handler.sa_handler = sigHandler;
      sigaction(SIGINT, &dfl_handler, NULL);
      sigaction(SIGQUIT, &dfl_handler, NULL);
      break;
    case SIGTSTP:
      /*
       * called when the user presses ctrl-z
       */
      TScreen::suspend();
      LOG("process stopped");

      /* use the default handler for SIGTSTP */

      dfl_handler.sa_handler = SIG_DFL;
      sigaction(SIGTSTP, &dfl_handler, NULL);
      raise(SIGTSTP);         /* stop the process */
      break;
    case SIGWINCH:
      doResize++;
      break;
  }
}

/*
 * CLASS FUNCTIONS
 */

/*
 * TScreen constructor.
 */

TScreen::TScreen()
{
        char *p = getenv("TVLOG");
        if (p != NULL && *p != '\0')
        {
                logfp = fopen(p, "w");
                LOG("using environment variable TVLOG=%s", p);
        }
        env[0] = '\0';
        if ((p = getenv("TVOPT")) != NULL)
        {
                LOG("using environment variable TVOPT=%s", p);
                for (char *d = env; *p != '\0'; p++) *d++ = tolower(*p);
        }

        /* acquire screen size */

        winsize win;
        ioctl(STDIN_FILENO, TIOCGWINSZ, &win);
        if (win.ws_col > 0 && win.ws_row > 0)
        {
                screenWidth = range(win.ws_col, 4, maxViewWidth);
                screenHeight = range(win.ws_row, 4, 80);
        }
        else
        {
                LOG("unable to detect screen size, using 80x25");
                screenWidth = 80;
                screenHeight = 25;
        }
        LOG("screen size is %dx%d", screenWidth, screenHeight);
        screenBuffer = new ushort[screenWidth * screenHeight];

        /* internal stuff */

        curX = curY = 0;
        cursor_displayed = true;
        currentTime = doRepaint = doResize = evLength = 0;
        evIn = evOut = &evQueue[0];
        msAutoTimer.stop();
        msOldButtons = 0;
        wakeupTimer.start(DELAY_WAKEUP);

        /* catch useful signals */

        struct sigaction dfl_handler;

        dfl_handler.sa_handler = sigHandler;
        sigemptyset(&dfl_handler.sa_mask);
        dfl_handler.sa_flags = SA_RESTART;

        sigaction(SIGALRM, &dfl_handler, NULL);
        sigaction(SIGCONT, &dfl_handler, NULL);
        sigaction(SIGINT, &dfl_handler, NULL);
        sigaction(SIGQUIT, &dfl_handler, NULL);
        sigaction(SIGTSTP, &dfl_handler, NULL);
        sigaction(SIGWINCH, &dfl_handler, NULL);

        /* generates a SIGALRM signal every DELAY_SIGALRM ms */

        struct itimerval timer;
        timer.it_interval.tv_usec = timer.it_value.tv_usec =
                DELAY_SIGALRM * 1000;
        timer.it_interval.tv_sec = timer.it_value.tv_sec = 0;
        setitimer(ITIMER_REAL, &timer, NULL);
}

/*
 * TScreen destructor.
 */

TScreen::~TScreen()
{
  freeResources();
}

void TScreen::resume()
{
  startcurses();
  doRepaint++;
}

void TScreen::suspend()
{
  stopcurses();
}

static bool find_event_in_queue(bool keyboard, TEvent *ev)
{

  TEvent *e = evOut;
  int i;
  for ( i=0; i < evLength; i++, e++ )
  {
    if ( e >= &evQueue[eventQSize] ) e = evQueue;
    if ( keyboard )
    {
      if ( e->what == evKeyDown )
        break;
    }
    else
    {
      if ( (e->what & evMouse) != 0 )
        break;
    }
  }
  if ( i == evLength )
    return false;
  *ev = *e;
  evLength--;
  while ( i < evLength )
  {
    TEvent *next = e + 1;
    if ( next >= &evQueue[eventQSize] ) next = evQueue;
    *e = *next;
    e = next;
  }
  return true;
}

void TEvent::_getKeyEvent(void)
{
  if ( !find_event_in_queue(true, this) )
  {
    TScreen::get_key_mouse_event(*this);
    if ( what != evKeyDown )
    {
      TScreen::putEvent(*this);
      what = evNothing;
    }
  }
}

void TEventQueue::getMouseEvent(TEvent &event)
{
//  if ( !find_event_in_queue(true, &event) )
  {
    TScreen::get_key_mouse_event(event);
    if ( (event.what & evMouse) == 0 )
    {
      TScreen::putEvent(event);
      event.what = evNothing;
    }
  }
}

/*
 * Gets an event from the queue.
 */

void TScreen::getEvent(TEvent &event)
{
  event.what = evNothing;
  if (doRepaint > 0)
  {
    doRepaint = 0;
    event.message.command = cmSysRepaint;
    event.what = evCommand;
  }
  else if (doResize > 0)
  {
    qclear();       /* blank the screen */
    stopcurses();
    startcurses();
    doResize = 0;
    winsize win;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &win);
    if (win.ws_col > 0 && win.ws_row > 0)
    {
      screenWidth = range(win.ws_col, 4, maxViewWidth);
      screenHeight = range(win.ws_row, 4, 100); // replace with a symbolic constant!
      delete[] screenBuffer;
      screenBuffer = new ushort[screenWidth * screenHeight];
      LOG("screen resized to %dx%d", screenWidth, screenHeight);
      drawCursor(0); /* hide the cursor */
    }
    event.message.command = cmSysResize;
    event.what = evCommand;
  }
  else if (evLength > 0)  /* handles pending events */
  {
    evLength--;
    event = *evOut;
    if (++evOut >= &evQueue[eventQSize]) evOut = &evQueue[0];
  }
  else if (msAutoTimer.isExpired())
  {
    msAutoTimer.start(DELAY_AUTOCLICK_NEXT);
    event = msev;
    event.what = evMouseAuto;
  }
  else if (wakeupTimer.isExpired())
  {
    wakeupTimer.start(DELAY_WAKEUP);
    event.message.command = cmSysWakeup;
    event.what = evCommand;
  }
  else
  {
    get_key_mouse_event(event);
  }
//  if ( event.what != evNothing )
//    printf("event: %d (%d,%d)\n", event.what, event.mouse.where.x, event.mouse.where.y);
}

/*
 * Generates a beep.
 */

void TScreen::makeBeep()
{
  qbeep();
}

/*
 * Puts an event in the queue.  If the queue is full the event will be
 * discarded.
 */

void TScreen::putEvent(TEvent &event)
{
        if (evLength < eventQSize)
        {
                evLength++;
                *evIn = event;
                if (++evIn >= &evQueue[eventQSize]) evIn = &evQueue[0];
        }
}

/*
 * Hides or shows the cursor.
 */

void TScreen::drawCursor(int show)
{
  if ( show )
    moveto(curX, curY);
  if ( show != cursor_displayed )
  {
    cursor_displayed = show;
    qcurs_set(show);
  }
}

/*
 * Hides or shows the mouse pointer.
 */

void TScreen::drawMouse(int show)
{
  // we do not draw the mouse cursor ourselves
}

/*
 * Moves the cursor to another place.
 */

void TScreen::moveCursor(int x, int y)
{
  moveto(x, y);
  curX = x;
  curY = y;
}

/*
 * Draws a line of text on the screen.
 */

void TScreen::writeRow(int dst, ushort *src, int len)
{
  char outbuf[4096];
  char *ptr = outbuf;
  char *const end = outbuf + sizeof(outbuf);

  int x = dst % TScreen::screenWidth;
  int y = dst / TScreen::screenWidth;
  ptr += moveto_to_buf(x, y, ptr, end-ptr);

  while ( --len >= 0 )
  {
    int code = *src & 0xff;           /* character code */
    int color = (*src & 0xff00) >> 8; /* color code */
//    LOG("%d: PUT %x\n", len, code);
    ptr += output_char_to_buf(code, color, ptr, end-ptr);
    src++;
  }
  ptr += moveto_to_buf(curX, curY, ptr, end-ptr);
  if ( current_page != 0 )
  {
    if ( ptr < end )
    {
      *ptr++ = 15;        // select G0
      *ptr = '\0';
    }
    current_page = 0;
  }
  outstr(outbuf);
}

/*
 * Expands a path into its directory and file components.
 */

void expandPath(const char *path, char *dir, char *file)
{
        char *tag = strrchr(path, '/');

        /* the path is in the form /dir1/dir2/file ? */

        if (tag != NULL)
        {
                strcpy(file, tag + 1);
                strncpy(dir, path, tag - path + 1);
                dir[tag - path + 1] = '\0';
        }
        else
        {
                /* there is only the file name */

                strcpy(file, path);
                dir[0] = '\0';
        }
}

void fexpand(char *path)
{
        char dir[PATH_MAX];
        char file[PATH_MAX];
        char oldPath[PATH_MAX];

        expandPath(path, dir, file);
        getcwd(oldPath, sizeof(oldPath));
        chdir(dir);
        getcwd(dir, sizeof(dir));
        chdir(oldPath);
        if (strcmp(dir, "/") == 0) sprintf(path, "/%s", file);
        else sprintf(path, "%s/%s", dir, file);
}

void TEventQueue::mouseInt() { /* no mouse... */ }

void THWMouse::resume()
{
  LOG("RESUME MOUSE\n");
  buttonCount=2;
  outstr("\E[?1002s\E[?1002h");
}

void THWMouse::suspend()
{
  outstr("\E[?1002l\E[?1002r");
}

void TV_CDECL THWMouse::show()
{
  LOG("SHOW MOUSE\n");
}

void TV_CDECL THWMouse::hide()
{
  LOG("HIDE MOUSE\n");
}

#endif // __LINUX__
