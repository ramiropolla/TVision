//    uncomment the next line to test the keyboard dispatcher
//#define KBD_DEBUG
//    uncomment the next line to create the dump of key definition (and exit)
//#define DUMP_TREE_ONLY  "key_tree.txt"
//    uncomment the next line to test winsize changing
//#define WINCHG_DEBUG
//    uncomment the next line to test the mouse(s)
//#define MOUSE_DEBUG
//    uncomment the next line to test Xmodifiers (Alt is undocumented)
//#define XTERM_DEBUG
//
/*
 *  The following environment variables are used:
 *     TVLOG - the name of the log-file
 *             if it is not defined, use syslog with LOG_WARNING priority
 *     TERM  - the terminal definition (see terminfo)
 *     TVOPT - the enduser flags. Has many subfields delimited by commas ','.
 *          noX11   - when libX11.so is not compatible
 *          noGPM   - when libgpm.so is not compatible
 *          ansi    -
 *               OR
 *          mono    - when the terminfo data of your display does not declare
 *                    it as haing the ANSI-color support
 *          ign8    - ignore '8bit as meta key' in the terminfo description
 *          xtrack  - if your xterm-emulator in telnet client does not support
 *                    mode 1002 (only 1000), set this flag
 *          alt866  - do not encode pseudographic symbols (for the console with
 *                    alt-font loaded)
 *          cyrcvt= - cyrilic conversion (oem/koi8r).
 *                    possible values are:
 *            linux   - for linux russian users and PuTTY (in/out koi8r)
 *            kwin    - output in koi8 and input in cp1251 - any telnet
 *            windows - for many telents and any linux users (in/out 1251)
 *
 *         The best choice for russian users (on the console) is:
 *              setfont alt-8x16.psf.gz -m koi2al
 *              loadkey ru-ms.map"
 *              export TVOPT=cyrcvt=linux,alt866
 *
 *        TVision currently makes some 'remapping's for the keyboard
 *        (when the keycodes are not described in the pty's terminal description).
 *
 *                Shift + [Home, End, PgUp, PgDn] => Ctrl + ...
 *
 *        Also (for telnet applications):
 *
 *           Alt + [Tab, Enter, Insert, Delete] => Ctrl + ...
 *
 *        For 'Alt' you can add a meta-key in telnet (Alt) before
 *        your key sequence (e.g remap '\e\n' to CtrlEnter)
 *
 *        you have a 'windows' mode and... Midnight Commander compatibility.
 *
 *        Added the keystrokes not described in the terminfo database.
 *          rxvt-extension keys (you can use it in your telnets)
 *
 *          \e[5^ => PgUp   + Ctrl
 *          \e[6^ => PgDwn  + Ctrl
 *          \e[7^ => Home   + Ctrl
 *          \e[8^ => End    + Ctrl
 *          \e[7$ => Home   + Shift
 *          \e[8$ => End    + Shift
 *
 *       If your terminal type is xterm(-xxx), Shifted Arrow keys
 *       are mapped to:
 *
 *            ShiftUp    => \eO2A
 *            ShiftDown  => \eO2B
 *            ShiftRight => \eO2C
 *            ShiftLeft  => \eO2D
 */
//
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
/* Rewritten by Yury Charon, <yjh@styx.cabel.net> */

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
#define Uses_Tkeys
#include <tv.h>

//---------------------------------------------------------------------------
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>   // realpath
#include <limits.h>
#include <syslog.h>
#include <dlfcn.h>    // dlopen/dlsym
#include <poll.h>
#include <gpm.h>
#include <curses.h>   // terminfo
#include <term.h>
#include <X11/Xlib.h> // X11console
#undef buttons    // term.h

#define qnumber(x) (sizeof(x)/sizeof(x[0]))

//---------------------------------------------------------------------------
/*
 * This is the delay in ms before the first evMouseAuto is generated when the
 * user holds down a mouse button.
 */
#define DELAY_AUTOCLICK_FIRST   400

/*
 * This is the delay in ms between next evMouseAuto events.
 */
#define DELAY_AUTOCLICK_NEXT    100

/*
 * This broadcast event is used to update the StatusLine.
 */
#define DELAY_WAKEUP            200

/*
   ms to get the continuation of an escape sequence
*/
#define DELAY_ESC               100


#define DELAY_GPM_START_CHECK   5000  // 5s (if not present at start)
#define MAX_GPM_REREAD_ATTEMPT  3     // attemp's to reread Gpm_Event

#define PRG_LOG_NAME  "IdaLinux"
#define PRG_FACILITY  LOG_USER

//---------------------------------------------------------------------------
/*
 * Expands a path into its directory and file components.
 */
void expandPath(const char *path, char *dir, char *file)
{
    /* the path is in the form /dir1/dir2/file ? */
    const char *tag = strrchr(path, '/');
    size_t  n;

    if(tag) {
      n = ++tag - path;
      memcpy(dir, path, n = tag - path);
    } else {  // only file name
      n = 0;
      tag = path;
    }
    dir[n] = '\0';
    strcpy(file, tag);
}

//---------------------------------------------------------------------------
void fexpand(char *path)
{
    char resolved[PATH_MAX];

    if(realpath(path, resolved) || errno == ENOENT)
        strcpy(path, resolved);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#if STDIN_FILENO != 0
#error
#endif
static struct termios old_tios, my_tios;
static int    tty_mode;

//X11-console (library load dynamically)
static Display  *x11_display;
static Window   x11_window;
static void     *hX11;
typedef Display * (*tXOpenDisplay)(char *);
typedef void      (*tXCloseDisplay)(Display *);
typedef Bool      (*tXQueryPointer)(Display *, Window, Window *, Window *,
                                    int *, int *, int *, int *, unsigned *);
static tXCloseDisplay pXCloseDisplay;
static tXQueryPointer pXQueryPointer;
//GPM (console mouse) support
static void   *hGPM;
typedef int   (*tGpm_Open)(Gpm_Connect *, int);
typedef void  (*tGpm_Close)(void);
typedef int   (*tGpm_GetEvent)(Gpm_Event *);
static  tGpm_Open     pGpm_Open;
static  tGpm_Close    pGpm_Close;
static  tGpm_GetEvent pGpm_GetEvent;
// for terminal with !vt positioning
static void   *hCurses;

static struct pollfd  pfd_data[2];
static int    pfd_count;

/*
 * terminfo strings for current display
 * the first byte is the string length (if string is parametrized, padding counter)
 */
/*
 * FIXME: Currently implemented only ANSI color dispatching
 */
static const char *cur_invis; // set cursor invisible (may be NULL)
static const char *cur_vis;   // set cursor visible (may be NULL)
static const char *cur_move;  // parametrized for tgoto
static const char *bell_line; // (may be NULL)
static const char *clear_scr; // clear screen
static const char *pages_init;// init alt-charset and select p0 (may be NULL)
static const char *page_G1;   // select alt-charset (may be NULL)
static const char *page_G0;   // select std-charset (may be NULL only if !_G1)
static struct {
  uchar doscyr_ninp : 1,  // no koi8->866 on input
        doscyr_nout : 1,  // no 866->koi8 on output
        doscyr_kwin : 1,  // on input 1251->866
        doscyr_owin : 1,  // on output 866->1251
        alt866_font : 1,  // ALT font layout (do not recode pseudographics)
        ansi_none   : 1,  // without colors (dumb terminals)
        ansi_force  : 1,  // make ansi color for terminal without descriptors
        meta_8bit   : 1;  // display set 8 bit at 'Alt' key
}mode;
#define SM_DOSCYR_DEFAULT *(uchar *)&mode |= 3  // _ninp=_nout=1
#define SM_DOSCYR_ENABLE  *(uchar *)&mode &= ~3 // _ninp=_nout=0
#define SM_DOSCYR_CHANGED (*(uchar *)&mode & 0xF) != 3

static struct {
  uchar pc_console : 1,   // working on the console
        have_xmice : 1,   // xterm or other '\e[M' reported display
        xmice_auto : 1,   // last generated xmice event must autocomplete
        reset_attr : 1,   // color and attributes must be fully set
        last_kbd   : 1,   // last generated event kbd (else - mouse)
        doRepaint  : 1,   // should redraw the screen ?
        doResize   : 1,   // resize screen ?
        tty_owned  : 1;   // ttin/ttout handler mode is set
}work;
static uchar    last_page;      // 0-G0, 1-G1
static uchar    last_reverse;   // 0/1 only (speed)

//---------
#pragma pack(1)
static uchar  cursor_hide;  /* is cursor visible? */
static uchar  lastMods;     /* keyboard modifiers generated by esc-seq */
static uchar  last_color;   /* in OEM format */
static struct {
  uchar tioc;
  ulong from; //   struct { ushort x, y; };
  ulong to;   //     struct { ushort x, y; };
  union {
    uchar   cmd; // on3_off4;
    ushort  aligns;
  };
}con_mou;
#pragma pack()
static ushort overlapXmice_code;
static uchar  overlapXmice_mods;
//---
static char xmou_cmd[] = "\E[?1002\0";
#define XMOU_CMD_BYTE   xmou_cmd[sizeof(xmou_cmd)-2]
#define MOUSE_SHOW      XMOU_CMD_BYTE == 'h'
#define MOUSE_HIDE      XMOU_CMD_BYTE != 'h'
#define XMOU_TYPE_BYTE  xmou_cmd[sizeof(xmou_cmd)-3]
#define XMOU_SET_TRACK  XMOU_TYPE_BYTE = '0'
#define XMOU_MUST_TRACK XMOU_TYPE_BYTE == '0'

//---------------------------------------------------------------------------
static const char *logname;
void LOG(const char *format, ...)
{
    FILE *f;
    va_list va;

    va_start(va, format);
    if(!logname) vsyslog(LOG_WARNING, format, va);
    else if((f = fopen(logname, "a")) != NULL) {
      vfprintf(f, format, va);
      fprintf(f, "\n");
      fclose(f);
    }
    va_end(va);
}

//---------------------------------------------------------------------------
static void error(const char *format, ...)
    __attribute__((noreturn, __format__(__printf__, 1, 2)));
static void error(const char *format, ...)
{
    va_list va;

    fputs("\n\nTVision error: ", stderr);
    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);
    fputc('\n', stderr);
    abort();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static char *qstrdup(const char *p)
{
    if((p = strdup(p)) == NULL) abort();
    return((char *)p);
}

//---------------------------------------------------------------------------
static char *qmalloc(size_t len)
{
    char  *p;

    if((p = (char *)malloc(len)) == NULL) abort();
    return(p);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static int qgetch(void)
{
    union {
      int _dummy; //alignment for valgrind ONLY :(
      uchar c;
    };
    (void)_dummy;

    for( ; ; )
      switch(read(0, &c, 1)) {
        case 1:
          if(c) { // skip fillers
#ifdef KBD_DEBUG
            LOG("  getch: %02X(%c)", c, c);
#endif
            return(c);
          }
          continue;
        case -1:
          switch(errno) {
            case EINTR: // for SIGWINCH
            case EAGAIN:
              return(EOF);
            case EPIPE:
              abort();  // mc and other subshell stopped
            default:
              break;
          }
          // PASS THRU
        default:
          error("TTY-read failed");
      }
}

//---------------------------------------------------------------------------
static void term_out(const char *buf, size_t len)
{
    size_t  sz;
    int     svm = -1;

    for( ; ; )
      switch(sz = write(0, buf, len)) {
        default:
          if(sz > len) goto bad;
          if((len -= sz) != 0) {
            buf += sz;
            continue;
          }
          if(!svm && fcntl(0, F_SETFL, tty_mode | O_NONBLOCK) == -1) goto bad;
          if(MOUSE_SHOW && con_mou.cmd == 4) --con_mou.cmd; // redraw
          return;

        case -1:
          switch(errno) {
#if EWOULDBLOCK != EAGAIN
            case EWOULDBLOCK:
#endif
            case EAGAIN:
              if(++svm || fcntl(0, F_SETFL, tty_mode & ~O_NONBLOCK) == -1) {
            default:
                break;
              }
              //PASS THRU
            case EINTR:
              continue;
            case EPIPE:
              abort();  // mc or another subshell stopped
          }
          //PASS THRU
        case 0:
bad:
         error("TTY-output failed");
      }
}
//---------------------------------------------------------------------------
static inline void xmouse_send(void)
{
    term_out(xmou_cmd, sizeof(xmou_cmd)-1);
}

//---------------------------------------------------------------------------
static inline void tistr_out(const char *p)
{
    term_out(p+1, (uchar)*p);
}

//---------------------------------------------------------------------------
static inline char *tistr_copy(char *out, const char *ptr)
{
    size_t  len = (uchar)*ptr++;

    memcpy(out, ptr, len);
    return(out + len);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef char *(*Tgoto)(char *, int, int);

static Tgoto  curs_moveto;
static Tgoto  Pgoto;

//---------------------------------------------------------------------------
static char *_moveto_vt(char *ptr, int x, int y)
{
    return(ptr + sprintf(ptr, "\e[%d;%dH", y + 1, x + 1));
}

//---------------------------------------------------------------------------
static char *_moveto_ncurses(char *ptr, int x, int y)
{
    return(stpcpy(ptr, Pgoto((char *)cur_move, x, y)));
}

//---------------------------------------------------------------------------
static void immediate_curs_moveto(int x, int y)
{
    char    buff[512];
    size_t  sz = curs_moveto(buff, x, y) - buff;

    term_out(buff, sz);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/* FIXME: implemented only ANSI color/attr */
// TEMP: (color can be switched to terminfo, but not attr's...) DBX
#define ANSI_DEFAULT  "\e[;37;40m"  // sgr0 + color = white on black

static char *set_color_page(uchar color, char *buf, uchar reverse, uchar page)
{
    static const uchar map[] = {0, 4, 2, 6, 1, 5, 3, 7};


    if((page ^= last_page) != 0) last_page ^= 1;  // calculate here (see below)

    register char *ptr = buf;
    if(mode.ansi_none) goto skip_color;

    uchar diff;

    *ptr++ = '\e';
    *ptr++ = '[';
    diff = color ^ last_color;
    last_color = color;
    if(work.reset_attr) {
      work.reset_attr = 0;
      goto set_full;
    }
    if(last_reverse != reverse) goto set_attr;
    if(!diff) goto skip_color_dec2; // not changed

    if(diff & 0x88) {
set_attr:
      if(mode.ansi_force) {
set_full:
        diff = (uchar)-1;
        if(page_G1) page = diff; // must set (for 'ansi' format of sgr0
//        *ptr++ = '0'; // DBX
        *ptr++ = ';';
        if(color & 8) {
          *ptr++ = '1'; // bold
          *ptr++ = ';';
        }
        if(color & 0x80) {
          *ptr++ = '5'; // blink
          *ptr++ = ';';
        }
        if(reverse) goto set_reverse;
      } else { // normal ansi
        if(diff & 8) {
          if(!(color & 8)) {
            *ptr++ = '2';
            *ptr++ = '2';
          } else *ptr++ = '1';
          *ptr++ = ';';
        }
        if(diff & 0x80) {
          if(!(color & 0x80)) *ptr++ = '2';
          *ptr++ = '5';
          *ptr++ = ';';
        }
        if(last_reverse != reverse) {
          if(!reverse) *ptr++ = '2';
set_reverse:
          *ptr++ = '7';
          *ptr++ = ';';
        }
      }
      last_reverse = reverse;
    }

    if(diff & 7) {  // foreground
      *ptr++ = '3';
      *ptr++ = (char)('0' + map[color & 7]);
      *ptr++ = ';';
    }
    if(diff & 0x70) { // background
      *ptr++ = '4';
      *ptr++ = (char)('0' + map[(color >> 4) & 7]);
    } else if(ptr[-1] == ';') --ptr;
    *ptr++ = 'm';
    ++ptr;  // unification
    ++ptr;
skip_color_dec2:
    --ptr;
    --ptr;
skip_color:
    if(page) ptr = tistr_copy(ptr, last_page ? page_G1 : page_G0);
    return(ptr);
}

//---------------------------------------------------------------------------
static char *output_colored_char_to_buf(unsigned cc, char *buf)
{
    char  *ptr = buf;
    uchar code;

    code  = (uchar)cc;
    if(code < 0x20 && !mode.alt866_font) {
      static const uchar arrow[] =
            { 0x10, 0x11, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1D, 0x1E, 0x1F };
      static const uchar ar_to[sizeof(arrow)+2] = "><o^v<>-^v\xFE";

      unsigned of = (uchar*)memchr(arrow, code, sizeof(arrow)) - arrow;
      if(of > sizeof(arrow)) of = sizeof(arrow);  // thick dot
      code = ar_to[of];
    }

    uchar needed_page = 0, reverse = 0;
    if(!page_G1) {
      if(code >= 0xDB && code < 0xE0) goto out_mark_space;
      if(code == 0xFE && !mode.alt866_font) code &= 0x7F;  // =>7E(~)
    } else if(code < 0x60) {
      if(code >= 0x40) {
no_change_page:
        needed_page = last_page;
      }
    } else if(code >= 0xB0) {
      static const uchar trans[] =
      {
      /* B0 */ 0x61, 0x61, 0x61, 0x78, 0x75, 0x75, 0x75, 0x6B,
      /* B8 */ 0x6B, 0x75, 0x78, 0x6B, 0x6A, 0x6A, 0x6A, 0x6B,
      /* C0 */ 0x6D, 0x76, 0x77, 0x74, 0x71, 0x6E, 0x74, 0x74,
      /* C8 */ 0x6D, 0x6C, 0x76, 0x77, 0x74, 0x71, 0x6E, 0x76,
      /* D0 */ 0x76, 0x77, 0x77, 0x6D, 0x6D, 0x6C, 0x6C, 0x6E,
      /* D8 */ 0x6E, 0x6A, 0x6C
      };

      if(code == 0xFE) {
        if(!mode.alt866_font) code &= 0x7E;  // =>7E(thick dot)
        goto do_first_page;
      }
      if(code < 0xE0) {
        if(code >= 0xDB) {
out_mark_space:
          ++reverse;
          code = ' ';
          goto no_change_page;
        }
        if(!mode.alt866_font) code = trans[code-0xB0];
do_first_page:
        ++needed_page;
      }
    }

    ptr = set_color_page((uchar)(cc >> 8), ptr, reverse, needed_page);
    if(code >= 0x80 && !mode.doscyr_nout) {
      static const uchar dos2koi1[0xAF-0x80+1] = { // cyr A-p
        0xE1, 0xE2, 0xF7, 0xE7, 0xE4, 0xE5, 0xF6, 0xFA,
        0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0,
        0xF2, 0xF3, 0xF4, 0xF5, 0xE6, 0xE8, 0xE3, 0xFE,
        0xFB, 0xFD, 0xFF, 0xF9, 0xF8, 0xFC, 0xE0, 0xF1,
        0xC1, 0xC2, 0xD7, 0xC7, 0xC4, 0xC5, 0xD6, 0xDA,
        0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0
      };
      static const uchar dos2koi2[0xF1-0xE0+1] = { // cyr r-ja, YO,yo
        0xD2, 0xD3, 0xD4, 0xD5, 0xC6, 0xC8, 0xC3, 0xDE,
        0xDB, 0xDD, 0xDF, 0xD9, 0xD8, 0xDC, 0xC0, 0xD1,
        0xB3, 0xA3
      };

      if(code <= 0xAF) {
        if(!mode.doscyr_owin) code = dos2koi1[code-0x80];
        else code += (0xC0 - 0x80);
      } else if(code >= 0xE0 && code <= 0xF1) {
        if(!mode.doscyr_owin) code = dos2koi2[code-0xE0];
        else if(code < 0xF0) code += (0xF0 - 0xE0);
        else code = ((code & 1) << 4) | 0xA8; // F0->A8, F1->B8
      }
    }

    *ptr++ = (char)code;
    return(ptr);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
ushort  TEventQueue::doubleDelay = 8;
Boolean TEventQueue::mouseReverse;

ushort TScreen::screenMode;
int    TScreen::screenWidth;
int    TScreen::screenHeight;
ushort *TScreen::screenBuffer;

static TEvent *evIn, *evOut;    /* message queue system */
static TEvent evQueue[eventQSize];
static size_t evLength;         /* number of events in the queue */
static int    curX, curY;       /* current cursor coordinates */
static TEvent msev;             /* last mouse event */
static int    msOldButtons;     /* mouse button state (ignoring reverse) */

/*
 * A simple class which implements a timer.
 */

class Timer
{
        ulong limit;
public:
        Timer() { limit = (uint)-1; }
        bool isExpired(ulong curTime) { return(curTime >= limit); }
        bool isRunning() { return(limit != (uint)-1); }
        void set(ulong endTime) { limit = endTime; }
        void stop() { limit = (uint)-1; }
        ulong getLimit() { return(limit); } // for escTimer and poll
        bool inWait(ulong curTime) { return((long)curTime <= (long)limit); }
};

static Timer msAutoTimer;       /* time when generate next cmMouseAuto */
static Timer msDblCTimer;       /* time when generate DoubleClick */
static Timer wakeupTimer;       /* time when generate next cmWakeup */
static Timer gpmReopenTimer;    /* if gpm closing, attemp to reopen it */

//---------------------------------------------------------------------------
/*
 * GENERAL FUNCTIONS
 */

static inline int range(int test, int min, int max)
{
    return(test < min ? min : test > max ? max : test);
}

//---------------------------------------------------------------------------
static ulong _timeBase;
static ulong get_system_time(void)
{
    struct timeval  tv;

    if(gettimeofday(&tv, NULL)) abort();
    return(((tv.tv_sec - _timeBase) * 1000) + (tv.tv_usec / 1000));
}

//---------------------------------------------------------------------------
ulong getTicks(void)
{
    return(get_system_time() / 18);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/*
 * KEYBOARD FUNCTIONS
 */

struct node {
    uchar value;    // key in sequence
    uchar islast;   // is last key
    union {
      struct {
        ushort  tvcode; // tvision code
        uchar   mods;   // shift/ctrl/alt
      };
      ushort next;      // next node in the list
    };
};

static size_t keymap_size, states_size = 1;
static node   keymap[500];    // array of nodes, first slot not used
static ushort *states[200];   // 1,2..., first slot not used
static struct {
    ushort    tvcode;   // tvision code (if 0 => sequence OR undefined)
    union {
      uchar   mods;     // if(tvcode)
      ushort  state;    // if(!tvcode);
    };
}ctrl_map[0x20];

//---------------------------------------------------------------------------
const int kblNormal=0, kblShift=1, kblAltR=2, kblCtrl=4, kblAltL=8;

static inline uchar state2tvstate(uchar state)
{
    uchar   out = 0;
    if(state & kblShift) out |= kbShift;
    if(state & kblAltL)  out |= kbLeftAlt;
    if(state & kblAltR)  out |= kbRightAlt;
    if(state & kblCtrl)  out |= kbCtrlShift;
    return(out);
}

//---------------------------------------------------------------------------
static unsigned findstateidx(unsigned state, uchar key)
{
    const ushort  *nodes = states[state];

    if(nodes)
      for( ; ; ) {
        unsigned  idx = *nodes;
        uchar     chr = keymap[idx].value;  // in zero element == 0 :)
        if(chr == key) return(idx);
        if(chr < key) break;  // not found
        ++nodes;
      }
    return(0);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//***************************************************************************
static int process_escape(int chr, int start_code)
{
    unsigned  idx;
    uchar     mods = 0;

    // esc + escape sequence => alt + escape sequence
    if((uchar)chr == '\e' && chr == start_code) {
      if((int)(idx = qgetch()) == EOF) goto meta_char; // Alt+ESC
      chr = idx;
      mods = kblAltL;
    }

    if((idx = findstateidx(ctrl_map[start_code].state, (uchar)chr)) == 0) {
      if((uchar)start_code == '\e') {
meta_char:
        lastMods = kbLeftAlt;
      }
      return(chr);
    }
    do if(keymap[idx].islast) {
      lastMods = state2tvstate(mods | keymap[idx].mods);
      return(keymap[idx].tvcode);
    }while(   (chr = qgetch()) != EOF
           && (idx = findstateidx(keymap[idx].next, (uchar)chr)) != 0);
    return(0);  // skip nointerpeted sequnce
}

//---------------------------------------------------------------------------
/*
 * Gets information about modifier keys (Alt, Ctrl and Shift).  This can
 * be done only if the program runs on the system console or under X11.
 */
uchar getShiftState(void)
{
    uchar shift = 0;
    int   cmsk;

    if(x11_display) {
      unsigned  gmsk;
      Window    root, child;
      int       root_x, root_y, win_x, win_y;

      if(!pXQueryPointer(x11_display, x11_window, &root, &child, &root_x,
                         &root_y, &win_x, &win_y, &gmsk)) error("X11 error");
      cmsk = gmsk;
#if (ShiftMask != 1) || (ControlMask != 4)
#error
#endif
#ifdef XTERM_DEBUG
      LOG("XMASK=%X", gmsk);
#endif
      goto apply_mask;
    }

    cmsk = 6;    /* TIOCLINUX function #6 */
    if(work.pc_console && ioctl(0, TIOCLINUX, &cmsk) != -1) {
apply_mask:
      shift = state2tvstate(cmsk);
    } else shift = lastMods;

    return(shift);
}

//---------------------------------------------------------------------------
#define LB(v) (v & 0xFF)
#define HB(v) (v >> 8)

#if LB(kbF1) || LB(kbF2) || LB(kbF3) || LB(kbF4) || LB(kbF5) || LB(kbF6) || \
    LB(kbF7) || LB(kbF8) || LB(kbF9) || LB(kbF10)|| LB(kbF11)||LB(kbF12) || \
    LB(kbAltF1) || LB(kbAltF2) || LB(kbAltF3) || LB(kbAltF4) || \
    LB(kbAltF5) || LB(kbAltF6) || LB(kbAltF7) || LB(kbAltF8) || \
    LB(kbAltF9) || LB(kbAltF10) || LB(kbAltF11) || LB(kbAltF12) || \
    LB(kbCtrlF1) || LB(kbCtrlF2) || LB(kbCtrlF3) || LB(kbCtrlF4) || \
    LB(kbCtrlF5) || LB(kbCtrlF6) || LB(kbCtrlF7) || LB(kbCtrlF8) || \
    LB(kbCtrlF9) || LB(kbCtrlF10) || LB(kbCtrlF11) || LB(kbCtrlF12) || \
    LB(kbShiftF1) || LB(kbShiftF2) || LB(kbShiftF3) || LB(kbShiftF4) || \
    LB(kbShiftF5) || LB(kbShiftF6) || LB(kbShiftF7) || LB(kbShiftF8) || \
    LB(kbShiftF9) || LB(kbShiftF10) || LB(kbShiftF11) || LB(kbShiftF12)
#error
#endif

#if LB(kbIns) || LB(kbDel) || LB(kbShiftIns) || LB(kbShiftDel) || \
    LB(kbHome) || LB(kbEnd) || LB(kbPgUp) || LB(kbPgDn) || \
    LB(kbCtrlHome) || LB(kbCtrlEnd) || LB(kbCtrlPgUp) || LB(kbCtrlPgDn)
               //|| !LB(kbTab)
#error
#endif

#if LB(kbAltQ) || LB(kbAltW) || LB(kbAltE) || LB(kbAltR) || LB(kbAltT) || \
    LB(kbAltY) || LB(kbAltU) || LB(kbAltI) || LB(kbAltO) || LB(kbAltP) || \
    LB(kbAltA) || LB(kbAltS) || LB(kbAltD) || LB(kbAltF) || LB(kbAltG) || \
    LB(kbAltH) || LB(kbAltJ) || LB(kbAltK) || LB(kbAltL) || LB(kbAltZ) || \
    LB(kbAltX) || LB(kbAltC) || LB(kbAltV) || LB(kbAltB) || LB(kbAltN) || \
    LB(kbAltM) || LB(kbAlt1) || LB(kbAlt2) || LB(kbAlt3) || LB(kbAlt4) || \
    LB(kbAlt5) || LB(kbAlt6) || LB(kbAlt7) || LB(kbAlt8) || LB(kbAlt9) || \
    LB(kbAlt0) || LB(kbAltMinus) || LB(kbAltEqual) || LB(kbAltSpace) || \
    LB(kbAltOpenBraket) || LB(kbAltCloseBraket) || LB(kbAltSemicolon) || \
    LB(kbAltApostrophe) || LB(kbAltBackApst) || LB(kbAltComma) || LB(kbAltDot)
#error
#endif

#if  LB(kbAltEsc)  || LB(kbAltBack)  || LB(kbCtrlIns) || LB(kbCtrlDel) || \
     LB(kbAltLeft) || LB(kbAltRight) || LB(kbAltUp)   || LB(kbAltDown) || \
     LB(kbAltEnd)  || LB(kbAltHome)  || LB(kbAltPgUp) || LB(kbAltPgDn) || \
     LB(kbIns)  || LB(kbDel) || LB(kbLeft) || LB(kbRight) || LB(kbUp)   || \
     LB(kbDown) || LB(kbEnd) || LB(kbHome) || LB(kbPgUp)  || LB(kbPgDn)
#error
#endif

static int apply_translation(int code, uchar *modifiers)
{
    static const uchar k_f[12] = {
      HB(kbF1), HB(kbF2), HB(kbF3), HB(kbF4), HB(kbF5), HB(kbF6),
      HB(kbF7), HB(kbF8), HB(kbF9), HB(kbF10), HB(kbF11), HB(kbF12)
    };
    static const uchar k_f_a[12] = {
      HB(kbAltF1), HB(kbAltF2), HB(kbAltF3), HB(kbAltF4), HB(kbAltF5),
      HB(kbAltF6), HB(kbAltF7), HB(kbAltF8), HB(kbAltF9), HB(kbAltF10),
      HB(kbAltF11), HB(kbAltF12)
    };
    static const uchar k_f_c[12] = {
      HB(kbCtrlF1), HB(kbCtrlF2), HB(kbCtrlF3), HB(kbCtrlF4), HB(kbCtrlF5),
      HB(kbCtrlF6), HB(kbCtrlF7), HB(kbCtrlF8), HB(kbCtrlF9), HB(kbCtrlF10),
      HB(kbCtrlF11), HB(kbCtrlF12)
    };
    static const uchar k_f_s[12] = {
      HB(kbShiftF1), HB(kbShiftF2), HB(kbShiftF3), HB(kbShiftF4), HB(kbShiftF5),
      HB(kbShiftF6), HB(kbShiftF7), HB(kbShiftF8), HB(kbShiftF9),
      HB(kbShiftF10), HB(kbShiftF11), HB(kbShiftF12)
    };


    static const uchar k_s[] = {
      HB(kbIns), HB(kbDel),
      HB(kbHome), HB(kbEnd), HB(kbPgUp), HB(kbPgDn) // remap to Ctrl
    };
    static const uchar m_s[sizeof(k_s)] = {
      HB(kbShiftIns), HB(kbShiftDel),
      HB(kbCtrlHome), HB(kbCtrlEnd), HB(kbCtrlPgUp), HB(kbCtrlPgDn),
    };


    static const uchar m_a[] = {
      HB(kbIns), HB(kbDel),  HB(kbLeft), HB(kbRight), HB(kbUp), HB(kbDown),
      HB(kbEnd), HB(kbHome), HB(kbPgUp), HB(kbPgDn)
    };
    static const uchar alts[sizeof(m_a) + 2] = {
      HB(kbAltEsc),  HB(kbAltBack),
      HB(kbCtrlIns), HB(kbCtrlDel),   // remap from Alt
      HB(kbAltLeft), HB(kbAltRight), HB(kbAltUp),   HB(kbAltDown),
      HB(kbAltEnd),  HB(kbAltHome),  HB(kbAltPgUp), HB(kbAltPgDn)
    };

    const uchar *p;
    unsigned    of;
    uchar       uc, state = *modifiers;

    if((unsigned)code <= 0xFF) {
      static const uchar  smk[4] = { 0x1B,  0x7F,   0x09,  0x0A    };
      static const ushort smv[4] = { kbEsc, kbBack, kbTab, kbEnter };
      static const ushort smc[3] = { kbCtrlBack, kbCtrlTab, kbCtrlEnter };

      if((p = (const uchar *)memchr(smk, code, sizeof(smk))) != NULL) {
        code = smv[of = p - smk];
        if(state) {
          if(state & kbAltShift) {
            if(of < 2) {
              p = alts;
              goto chg_code;
            }
            *modifiers = kbCtrlShift; // remap Alt+(Tab,Enter) => Ctrl (pty)
do_ctrl:
            return(smc[of-1]);
          } else if(state & kbCtrlShift) {
            if(of) goto do_ctrl;  // for console/X11
          } else if(code == kbTab) return(kbShiftTab);
        } // if(state)
        return(code);
      }

      if(code < 0x20) {
        if(!state && ctrl_map[code].tvcode) {
          *modifiers = state2tvstate(ctrl_map[code].mods);
          return(ctrl_map[code].tvcode);
        }
      } else if(code < 0x80) {
        static const char c_k[] =
            "QWERTYUIOPASDFGHJKLZXCVBNM1234567890-= []:'`,.\\/";

        static const uchar c_a[sizeof(c_k)-1] = {
          HB(kbAltQ), HB(kbAltW), HB(kbAltE), HB(kbAltR), HB(kbAltT),
          HB(kbAltY), HB(kbAltU), HB(kbAltI), HB(kbAltO), HB(kbAltP),
          HB(kbAltA), HB(kbAltS), HB(kbAltD), HB(kbAltF), HB(kbAltG),
          HB(kbAltH), HB(kbAltJ), HB(kbAltK), HB(kbAltL), HB(kbAltZ),
          HB(kbAltX), HB(kbAltC), HB(kbAltV), HB(kbAltB), HB(kbAltN),
          HB(kbAltM), HB(kbAlt1), HB(kbAlt2), HB(kbAlt3), HB(kbAlt4),
          HB(kbAlt5), HB(kbAlt6), HB(kbAlt7), HB(kbAlt8), HB(kbAlt9),
          HB(kbAlt0), HB(kbAltMinus), HB(kbAltEqual), HB(kbAltSpace),
          HB(kbAltOpenBraket), HB(kbAltCloseBraket), HB(kbAltSemicolon),
          HB(kbAltApostrophe), HB(kbAltBackApst), HB(kbAltComma),
          HB(kbAltDot), HB(kbAltBackslash), HB(kbAltSlash)
        };

        if(!(state & kbAltShift)) return(code);
        of = (char*)memchr(c_k, toupper(code), sizeof(c_k)-1) - c_k;
        if(of >= sizeof(c_a)) return(code);
        p = c_a;
        goto chg_code;
      }

      if((state & kbCtrlShift) || code < 0xA3 || mode.doscyr_ninp)
          return(code);

      if(code >= 0xC0) {
        static const uchar koi2dos[0xFF-0xC0+1] = { // (B3->F0, A3->F1)
          0xEE, 0xA0, 0xA1, 0xE6, 0xA4, 0xA5, 0xE4, 0xA3,
          0xE5, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE,
          0xAF, 0xEF, 0xE0, 0xE1, 0xE2, 0xE3, 0xA6, 0xA2,
          0x9C, 0xEB, 0xA7, 0xE8, 0xED, 0xE9, 0xE7, 0xEA,
          0x9E, 0x80, 0x81, 0x96, 0x84, 0x85, 0x94, 0x83,
          0x95, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E,
          0x8F, 0x9F, 0x90, 0x91, 0x92, 0x93, 0x86, 0x82,
          0x9C, 0x9B, 0x87, 0x98, 0x9D, 0x99, 0x97, 0x9A
        };
        if(!mode.doscyr_kwin) return(koi2dos[code-0xC0]);
        if(code < 0xF0) return(code - (0xC0 - 0x80));
        return(code - (0xF0 - 0xE0));
      }
      if(!mode.doscyr_kwin) {
        if(code == 0xA3) return(0xF1);
        else if(code == 0xB3) return(0xF0);
      } else {
        if(code == 0xA8) return(0xF0);
        else if(code == 0xB8) return(0xF1);
      }
      return(code);
    }

    if(!state || LB(code)) return(code);

    uc = (uchar)HB(code);
    if((of = (uchar*)memchr(k_f, uc, sizeof(k_f)) - k_f) < sizeof(k_f)) {
      if(state & kbAltShift)    p = k_f_a;
      else if(state & kbShift)  p = k_f_s;
      else                      p = k_f_c; // control
      goto chg_code;
    }

    if(state & kbAltShift) {
      if((of = (uchar*)memchr(m_a,uc,sizeof(m_a)) - m_a) < sizeof(m_a)) {
        if(of < 2) *modifiers = kbCtrlShift; // remap Ins/Del (Alt->Ctrl)
        p = alts + 2;
        goto chg_code;
      }
    } else if(!(state & kbCtrlShift)) { // (kbShift | kbCtrlShift) == Shift
      if((of = (uchar*)memchr(k_s,uc,sizeof(k_s)) - k_s) < sizeof(k_s)) {
        p = m_s;
        if(of >= 2) *modifiers = kbCtrlShift; // remap shift2ctrl
chg_code:
        code = (unsigned)p[of] << 8;
      }
    }
    return(code);
}
#undef LB
#undef HB

//---------------------------------------------------------------------------
static inline int char2scancode(uchar code)
{
    static const uchar k2s[] =
      "\x08\x09\x0D\x1B !\"#$%&'()*+,-./0123456789:;<=>?@[\\]^_`abcdefghij"
      "klmnopqrstuvwxyz{|}~";

    static const uchar scv[sizeof(k2s)-1] =
    {
      0x0E, 0x0F, 0x1C, 0x01, 0x39, 0x02, 0x28, 0x04, 0x05, 0x06, 0x08, 0x28,
      0x0A, 0x0B, 0x09, 0x0D, 0x33, 0x0C, 0x34, 0x35, 0x0B, 0x02, 0x03, 0x04,
      0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x27, 0x27, 0x33, 0x0D, 0x34, 0x35,
      0x03, 0x1A, 0x2B, 0x1B, 0x07, 0x0C, 0x29, 0x1E, 0x30, 0x2E, 0x20, 0x12,
      0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18, 0x19, 0x10,
      0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x1A, 0x2B, 0x1B,
      0x29
    };

    if(code >= 0x80 && !mode.doscyr_ninp) {
      static const char dos2eng1[0xAF-0x80+1+1] = // cyr A-p
        "f,dult;pbrkvyjghcnea[wxio]sm'.z"
        "f,dult;pbrkvyjg";
      static const char dos2eng2[0xF1-0xE0+1+1-2] = // cyr r-ja (YO,yo NOT implemented)
        "hcnea[wxio]sm'.z";

      if(code <= 0xAF) code = dos2eng1[code-0x80];
      else if(code >= 0xE0 && code <= 0xF1-2) code = dos2eng2[code-0xE0];
      else return(0);
    } else code = (uchar)tolower(code);

    unsigned id = (uchar *)memchr(k2s, code, sizeof(scv)) - k2s;
    if(id >= sizeof(scv)) return(0);

    return(((unsigned)scv[id]) << 8);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void msSetEvent(TEvent &event)
{
    event.mouse.buttons = msOldButtons;
    if(TEventQueue::mouseReverse) /* swap buttons ? */
      switch(event.mouse.buttons & (mbLeftButton | mbRightButton)) {
        case mbLeftButton:
        case mbRightButton:
          event.mouse.buttons ^= (mbLeftButton | mbRightButton);
        default:
          break;
      }
    msev = event;
#ifdef MOUSE_DEBUG
    LOG("MOUSE: what=%d, x=%d, y=%d, btn=%X, flags=%ld, mod=%lX",
        event.what, event.mouse.where.x, event.mouse.where.y,
        event.mouse.buttons, event.mouse.eventFlags,
        event.mouse.controlKeyState);
#endif
}

//---------------------------------------------------------------------------
static uchar create_xmouse_event(TEvent &ev, int event)
{
#define MouseB1Down 0x20
#define MouseB2Down 0x21
#define MouseB3Down 0x22
#define MouseB4Down 0x60
#define MouseB5Down 0x61
#define MouseUp     0x23
#define MouseMove   0x40

    TPoint  me;
    int     whC, whF;

    if((whC = qgetch()) == EOF || (whF = qgetch()) == EOF) return(0);

    if(event >= 0x60 || event < 0x20) return(0);  // B4 and B5 (or illegal code)
    me.x = whC - 0x21;
    me.y = whF - 0x21;

    whF = whC = 0;
    if(msev.mouse.where != me) whF |= meMouseMoved;
    ulong curTime = get_system_time();
    switch(event & ~0x1C) { // modifiers check later
      case MouseB2Down: // middle button
        msDblCTimer.stop();
      //PASS THRU
      default:  // PARANOYA
        return(0);

      case MouseMove:
move_event:
        msDblCTimer.stop();
        whC += evMouseAuto;
        if(whF) { // moved
          whC += (evMouseMove - evMouseAuto);
          if(msOldButtons) msAutoTimer.set(curTime + DELAY_AUTOCLICK_NEXT);
        }
        break;

      case MouseB1Down:
        if(msOldButtons & mbLeftButton) goto move_event;
        msOldButtons |= mbLeftButton;
        if(!whF && msDblCTimer.inWait(curTime)) whF |= meDoubleClick;
down_event:
        msDblCTimer.stop();
        msAutoTimer.set(curTime + DELAY_AUTOCLICK_FIRST);
        whC += evMouseDown;
        break;

      case MouseB3Down:
        if(msOldButtons & mbRightButton) goto move_event;
        msOldButtons |= mbRightButton;
        goto down_event;

      case MouseUp:
        msAutoTimer.stop();
        if(!msOldButtons) goto move_event;  // for middle key
        whC += evMouseUp;
        msDblCTimer.stop();
        if(msOldButtons & mbLeftButton)
          msDblCTimer.set(curTime + TEventQueue::doubleDelay * 18);
        if(whF && XMOU_MUST_TRACK) {
          work.xmice_auto = 1;
          whC += (evMouseMove - evMouseUp);
          break;
        }
        msOldButtons = 0;
        break;
    }

    ev.what = whC;
    ev.mouse.eventFlags = whF;
    ev.mouse.where.x = range(me.x, 0, TScreen::screenWidth - 1);
    ev.mouse.where.y = range(me.y, 0, TScreen::screenHeight - 1);
    if(x11_display || work.pc_console) whF = getShiftState();
    else {
      whF = 0;
      if(event &    4) whF |= kbShift;
      if(event &    8) whF |= kbAltShift;
      if(event & 0x10) whF |= kbCtrlShift;
    }
    ev.mouse.controlKeyState = whF;
    msSetEvent(ev);
    return(1);
}

//---------------------------------------------------------------------------
static void draw_pointer(void)
{
    if(ioctl(0, TIOCLINUX, &con_mou.tioc))
      LOG("GPM drawing error %d", errno);
}

//---------------------------------------------------------------------------
static void restart_gpm(int close_needed, ulong curTime)
{
    Gpm_Connect   gc;
    char          *p, save = 0; // = for gcc bug

    if(close_needed) pGpm_Close();
    pfd_count   = 1;
    con_mou.cmd = 4; // no redraw

    if((p = getenv("TERM")) != NULL) {
      save = *p;
      *p = '-'; // !xterm on linux console :)
    }
    gc.minMod       = 0;
    gc.maxMod       = ~0; // else cutting :(
    gc.defaultMask  = 0;
    gc.eventMask    = ~0;
    if((pfd_data[1].fd = pGpm_Open(&gc, 0)) >= 0) {
      LOG("GPM started");
      ++pfd_count;
      draw_pointer();     // hide pointer
      gpmReopenTimer.stop();
    } else {
#ifdef MOUSE_DEBUG
      LOG("Can't connect to GPM!");
#endif
      if(!curTime) curTime = get_system_time();
      gpmReopenTimer.set(curTime + DELAY_GPM_START_CHECK);
      pGpm_Close(); // remove gpm_tried flag :(
    }
    if(p) *p = save;
}

//---------------------------------------------------------------------------
static uchar create_gpm_event(TEvent &ev)
{
    Gpm_Event ge;
    int       res, cnt;

    cnt = MAX_GPM_REREAD_ATTEMPT;
reread:
    switch(res = pGpm_GetEvent(&ge)) {
      case 1:
        break;
      case -1:  // error (may be because of a signal)
        if(--cnt) goto reread;
      case 0:   // preclosed
        LOG("GPM %s", res ? "check error" : "closed externally");
        restart_gpm(1, 0);
        return(0);
      default:
        error("Unknown Gpm_GetEvent answer code %d", res);
    }

    con_mou.from = con_mou.to = *(ulong *)&ge.x;
    if(MOUSE_HIDE) return(0);

    if(ge.type & GPM_MFLAG) {
      con_mou.cmd = 4; // off (also - redraw is not needed)
      draw_pointer();
    }

    if(ge.type & (GPM_MOVE | GPM_DRAG | GPM_MFLAG))
      con_mou.cmd = 3; // on (redraw is needed)
    ev.mouse.eventFlags = 0;
    if((ge.type & (GPM_DOUBLE | GPM_UP)) == GPM_DOUBLE)
      ev.mouse.eventFlags |= meDoubleClick;
    if(ge.type & (GPM_MOVE | GPM_MFLAG))
      ev.mouse.eventFlags |= meMouseMoved;
    else if((ge.type & GPM_TRIPLE) || (ge.buttons & GPM_B_MIDDLE)) return(0);
    res = 0;
    if(ge.buttons & GPM_B_LEFT)   res |= mbLeftButton;
    if(ge.buttons & GPM_B_RIGHT)  res |= mbRightButton;
    if(ge.type & GPM_DOWN) {
      ev.what = evMouseDown;
      msOldButtons |= res;
    } else if(ge.type & GPM_UP) {
      ev.what = evMouseUp;
      msOldButtons &= ~res;
    } else if(ev.mouse.eventFlags & meMouseMoved) {
      ev.what = evMouseMove;
      msOldButtons = res;
    } else return(0); // res = evMouseAuto;
    --ge.x;
    --ge.y;
    ev.mouse.where.x = range(ge.x, 0, TScreen::screenWidth - 1);
    ev.mouse.where.y = range(ge.y, 0, TScreen::screenHeight - 1);
    ev.mouse.controlKeyState = state2tvstate(ge.modifiers);
    msSetEvent(ev);
    return(1);
}

//---------------------------------------------------------------------------
static void xmouse_up(TEvent &event)
{
      work.xmice_auto = 0;
      msev.what = evMouseUp;
      msOldButtons = msev.mouse.buttons = 0;
      event = msev;
#ifdef MOUSE_DEBUG
      LOG("xmouse autoUp");
#endif
}

//---------------------------------------------------------------------------
/* Reads a key from the keyboard or mouse */
static void get_key_mouse_event(TEvent &event, ulong curTime)
{
    static int    start_code;
    static Timer  escTimer;

    if(work.xmice_auto) return(xmouse_up(event));

    event.what = evNothing;

    int   code;
    long  wait;

    /*
    * suspend until there some data in file descriptors.
    * event_delay is maximum suspend time in milliseconds.
    * if event_delay != 0 and esc-sequence recived we wait for the full sequence.
    */

    ulong top_time = curTime + TProgram::event_delay;
rewait:
    if((wait = escTimer.getLimit()) != -1) { // -1 not started
      if((wait -= curTime) < 0) goto esc_drop;
    }
    if((ulong)wait > (ulong)TProgram::event_delay) wait = TProgram::event_delay;
    /* if working on the console and gpm is not present at start...*/
    if(gpmReopenTimer.isExpired(curTime)) restart_gpm(0, curTime);

    /* redraw mouse poiner after last in/out*/
    if(con_mou.cmd == 3) {
      draw_pointer();
      ++con_mou.cmd;  // = 4 (off)
    }

    lastMods = 0;
    /* wait or check */
    switch(poll(pfd_data, pfd_count, wait)) {
      default:
        if(pfd_count > 1 && pfd_data[1].revents) {
          work.last_kbd = 0;
          if(!create_gpm_event(event)) goto do_esc_check;
          return;
        }
        if(pfd_data[0].revents & POLLHUP)
          abort();
        if(pfd_data[0].revents & POLLIN) {
          if(MOUSE_SHOW && con_mou.cmd == 4) --con_mou.cmd; // redraw
          code = qgetch();
          if(code > 0) break; // -1 if EOF, 0 if cbrk/busy
        }
        // PASS THRU
      case 0:   // timeout
do_esc_check:
        if(!escTimer.isExpired(curTime = get_system_time())) goto no_symbol;
esc_drop:
        escTimer.stop();
        code = start_code;
        goto apply_symbol;
      case -1:  // error
        if(errno == EINTR) return;  // only SIGWINCH :)
        abort();  // system error
    }

    /* see if there is data available */
    if(escTimer.isRunning()) {
      escTimer.stop();
apply_escape:
      switch(code = process_escape(code, start_code)) {
        case kbMouse:
          if((code = qgetch()) != EOF) {
            if(!create_xmouse_event(event, code)) goto no_symbol_ts;
            work.last_kbd = 0;
            return;
          }
          if((code = overlapXmice_code) == 0) goto no_symbol_ts;
          lastMods = overlapXmice_mods;
        default:
          break;
        case 0:
no_symbol_ts:
          if(!wait || work.doResize) return;
          if((curTime = get_system_time()) < top_time) goto rewait;
          return;
      }
    } else if(   (uchar)code < 0x20
              && ctrl_map[code].state
              && !ctrl_map[code].tvcode) {
      start_code = code;
      code = qgetch();
      if(code != EOF) goto apply_escape;
      escTimer.set((curTime = get_system_time()) + DELAY_ESC);
no_symbol:
      if(wait && !work.doResize && curTime < top_time) goto rewait;
      return;
    }

apply_symbol:
    uchar modifiers = getShiftState();
#ifdef KBD_DEBUG
    LOG("KBDIN: code=%x mod=%x (last=%x)", code, modifiers, lastMods);
#endif
    if(code > 0x7F && mode.meta_8bit && (modifiers & kbLeftAlt)) {
      code &= 0x7F;
      modifiers &= ~kbLeftAlt;
    }
    work.last_kbd = 1;
    code = apply_translation(code, &modifiers);
    if((unsigned)code <= 0xFF) code |= char2scancode((uchar)code);
#ifdef KBD_DEBUG
    LOG("KBDOUT: code=%x mod=%x (last=%x)", code, modifiers, lastMods);
#endif
    event.what = evKeyDown;
    event.keyDown.keyCode = code;
    event.keyDown.controlKeyState = modifiers;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void set_mouse_attach(char get);
//---------------------------------------------------------------------------
// try many times if the  system call is interrupted
// it happens on X11
static void set_tty_mode(struct termios *tm, int mode)
{
    for(int i = 0; i < 10; i++) {
      if(!tcsetattr(0, TCSAFLUSH, tm)) break;
      if(errno != EINTR) goto bad;
      usleep(100);  // 0.1ms
    }
    if(fcntl(0, F_SETFL, mode) == -1) {
bad:
      abort();
    }
}

//---------------------------------------------------------------------------
static void reinit_screen(uchar crs_hide)
{
    char  buf[512], *p;

    cursor_hide = crs_hide;
    last_page   = 0;
    curX = curY = 0;
    work.reset_attr = 1; // full set color/mode

    p = buf;
    if(!mode.ansi_none) p = stpcpy(p, ANSI_DEFAULT);  // norm-attr/color
    if(pages_init)  p = tistr_copy(p, pages_init);    // enable alt charset
    if(page_G0)     p = tistr_copy(p, page_G0);       // set G0
    if(cur_invis)   p = tistr_copy(p, crs_hide ? cur_invis : cur_vis);
    p = tistr_copy(p, clear_scr); // blank screen
    p = curs_moveto(p, 0, TScreen::screenHeight-1);
    term_out(buf, p - buf);
    if(!mode.ansi_none) p = stpcpy(p, ANSI_DEFAULT);  // norm-attr/color
    if(page_G0) p = tistr_copy(p, page_G0); // set G0
    if(cur_vis) p = tistr_copy(p, cur_vis);
    p = tistr_copy(p, clear_scr); // blank screen
    p = curs_moveto(p, 0, TScreen::screenHeight-1);
    term_out(buf, p - buf);
}

//---------------------------------------------------------------------------
static void attach_tty(void)
{
    if(work.tty_owned) return;
    work.tty_owned = 1;

    if(!logname) openlog(PRG_LOG_NAME, 0, LOG_USER);

    set_tty_mode(&my_tios, tty_mode | O_NONBLOCK);
    reinit_screen(1);       //... and hide cursor
    set_mouse_attach('h');  // attach mouse
}

//---------------------------------------------------------------------------
static void detach_tty()
{
    if(!work.tty_owned) return;
    work.tty_owned = 0;

    set_mouse_attach(0);    // detach mouse
    reinit_screen(0);       // ... and show cursor
    set_tty_mode(&old_tios, tty_mode);

    if(!logname) closelog();  // for next shell
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/* General signal handler. */
static void sigHandler(int signo)
{
    if(signo != SIGWINCH) { // SIGTERM, SIGHUP, SIGUSR(1,2)
      detach_tty();
      error("Stopped at signal %d", signo);
    }

    work.doResize = 1;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/*
 * CLASS FUNCTIONS
 */
//---------------------------------------------------------------------------
void TScreen::resume()
{
    attach_tty();
    work.doRepaint = 1;
}

//---------------------------------------------------------------------------
void TScreen::suspend()
{
    detach_tty();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static bool find_event_in_queue(bool keyboard, TEvent *ev)
{
    TEvent *e;
    size_t i;

    if(evLength) for(i = 0, e = evOut; i < evLength; i++, e++) {
      if(e >= &evQueue[eventQSize]) e = evQueue;
      if(keyboard) {
        if(e->what == evKeyDown) goto found;
      } else {
        if(e->what & evMouse) goto found;
      }
    }
    return(false);

found:
    --evLength;
    for( ; ; ) {
      *ev = *e;
      if(++i < evLength) break;
      ev = e;
      if(++e >= &evQueue[eventQSize]) e = evQueue;
    }
    return(true);
}

//---------------------------------------------------------------------------
void TEvent::_getKeyEvent(void)
{
  if(!find_event_in_queue(true, this)) {
    get_key_mouse_event(*this, get_system_time());

    if(what & evMouse) {
      TScreen::putEvent(*this);
      what = evNothing;
    }
  }
}

//---------------------------------------------------------------------------
void TEventQueue::getMouseEvent(TEvent &event)
{
  if(!find_event_in_queue(false, &event)) {
    if(work.xmice_auto) goto auto_up;
    if(work.last_kbd && msOldButtons) { // from non-liquidated keys (ida BUG)
      msAutoTimer.stop();
      msDblCTimer.stop();
#ifdef MOUSE_DEBUG
      LOG("Request mouse event with buttons %X!", msOldButtons);
#endif
auto_up:
      return(xmouse_up(event));
    }
    get_key_mouse_event(event, get_system_time());

    if(event.what == evKeyDown) { // else evNothing OR evMouse
      TScreen::putEvent(event);
      event.what = evNothing;
    }
  }
}

//---------------------------------------------------------------------------
/*
 * Gets an event from the queue.
 */
void TScreen::getEvent(TEvent &event)
{
    event.what = evNothing;

    if(work.doResize) {
      work.doResize = 0;
      reinit_screen(cursor_hide);   // also blank the screen

      winsize win;
      if(   ioctl(0, TIOCGWINSZ, &win) != -1
         && win.ws_col > 0 && win.ws_row > 0) {

        screenWidth = range(win.ws_col, 4, maxViewWidth);
        screenHeight = range(win.ws_row, 4, 100); // replace with a symbolic constant!
        delete[] screenBuffer;
        screenBuffer = new ushort[screenWidth * screenHeight];
#ifdef WINCHG_DEBUG
        LOG("screen resized to %dx%d", screenWidth, screenHeight);
#endif
        drawCursor(0); /* hide the cursor */
      }
      signal(SIGWINCH, sigHandler);
      event.message.command = cmSysResize;
      event.what = evCommand;
      return;
    }

    if(evLength) { /* handles pending events */
      --evLength;
      event = *evOut;
      if(++evOut >= &evQueue[eventQSize]) evOut = evQueue;
      return;
    }

    ulong curTime = get_system_time();
    if(msAutoTimer.isExpired(curTime)) {
      msAutoTimer.set(curTime + DELAY_AUTOCLICK_NEXT);
      event = msev;
      event.what = evMouseAuto;
#ifdef MOUSE_DEBUG
      LOG("xmouse autorepeat");
#endif
      return;
    }

    if(wakeupTimer.isExpired(curTime)) {
      wakeupTimer.set(curTime + DELAY_WAKEUP);
      event.message.command = cmSysWakeup;
      event.what = evCommand;
      return;
    }

    get_key_mouse_event(event, curTime);
}

//---------------------------------------------------------------------------
/*
 * Generates a beep.
 */
void TScreen::makeBeep(void)
{
    if(bell_line) tistr_out(bell_line);
}

//---------------------------------------------------------------------------
/*
 * Puts an event in the queue.  If the queue is full the event will be
 * discarded.
 */
void TScreen::putEvent(TEvent &event)
{
    if(evLength < eventQSize) {
      ++evLength;
      *evIn = event;
      if(++evIn >= &evQueue[eventQSize]) evIn = evQueue;
    }
}

//---------------------------------------------------------------------------
/*
 * Hides or shows the cursor.
 */
void TScreen::drawCursor(int show)
{
    if(!show != cursor_hide) {
      cursor_hide ^= 1;
      if(show) immediate_curs_moveto(curX, curY);
      if(cur_invis) tistr_out(show ? cur_vis : cur_invis);
    }
}

//---------------------------------------------------------------------------
/*
 * Moves the cursor to another place.
 */
void TScreen::moveCursor(int x, int y)
{
    immediate_curs_moveto(x, y);
    curX = x;
    curY = y;
}

//---------------------------------------------------------------------------
/*
 * Draws a line of text on the screen.
 */
void TScreen::writeRow(int dst, ushort *src, int len)
{
    static char outbuf[4096 + 2 + 200];

    register char *ptr, *top;

    ptr = outbuf;
    top = ptr + (sizeof(outbuf)-2 - 200);

    ptr = curs_moveto(ptr, dst % TScreen::screenWidth,  // x
                           dst / TScreen::screenWidth); // y

    if(len)
      do if((ptr = output_colored_char_to_buf(*src++, ptr)) >= top) {
        term_out(ptr, ptr - outbuf);
        ptr = outbuf;
      }while(--len);

    ptr = curs_moveto(ptr, curX, curY);
    if(last_page) {
      --last_page; // = 0
      ptr = tistr_copy(ptr, page_G0); // select G0
    }
    if((len = ptr - outbuf) != 0) term_out(outbuf, len);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void TEventQueue::mouseInt(void) { /* no mouse... */ }

//---------------------------------------------------------------------------
/* Show/Hide mouse */
static void set_mouse_show(uchar show)
{
// really we show/unshow only GPM mouse. But when mouse is OFF,
// we disable mouse reporting to minimize the number of events
    if(show == XMOU_CMD_BYTE || !XMOU_CMD_BYTE) return;
    XMOU_CMD_BYTE = show;

#ifdef MOUSE_DEBUG
    LOG("mouse %s", show == 'h' ? "show" : "hide");
#endif
    if(show != 'h') msOldButtons = 0; // for external wait's :(
    if(con_mou.cmd) {
      con_mou.cmd = 3;  // show
      if(show != 'h') ++con_mou.cmd;  // hide
      draw_pointer();
    } else if(work.have_xmice) {
      xmouse_send();
      if(show != 'h') {
        msAutoTimer.stop();
        msDblCTimer.stop();
        work.xmice_auto = 0;
      }
    }
}

//---------------------------------------------------------------------------
/* Get/Free mouse for current job. */
static void set_mouse_attach(char get)
{
    if(!XMOU_CMD_BYTE == !get) return;

#ifdef MOUSE_DEBUG
    LOG("mouse %stach", get ? "at" : "de");
#endif

    if(hGPM) {
      if(!get) {
        gpmReopenTimer.stop();  // PARANOYA
        if(pfd_count > 1) {
          --pfd_count;
          pGpm_Close();
        }
      } else if(hGPM) restart_gpm(0, 0);
    } else if(work.have_xmice) {
      if(get) {
        XMOU_CMD_BYTE = 's';
        xmouse_send();  // save mode
        set_mouse_show('h');
      } else {
        set_mouse_show('l');
        XMOU_CMD_BYTE = 'r';
        xmouse_send();  // restore mode
      }
    }
    XMOU_CMD_BYTE = get;
}

//---------------------------------------------------------------------------
void THWMouse::resume(void)
{
    buttonCount = 2;
}

//---------------------------------------------------------------------------
void THWMouse::suspend(void)
{
    buttonCount = 0;
}

//---------------------------------------------------------------------------
void TV_CDECL THWMouse::show(void)
{
    set_mouse_show('h');  // on
}

//---------------------------------------------------------------------------
void TV_CDECL THWMouse::hide(void)
{
    set_mouse_show('l');  // off
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// terminfo load and other initialization initialization
typedef char * (*Tgetstr)(char *, char **);
static Tgetstr  Pgetstr;

//---------------------------------------------------------------------------
static char *get_pstr(const char *id, size_t *len)
{
    register char *p1, *p2;
    size_t        sz;

    if((p1 = Pgetstr((char *)id, NULL)) == NULL) return(NULL);
    p2 = strchr(p1, '\0');
    if((sz = p2 - p1) == 0) return(NULL);
    if(*--p2 == '>' && p2 > p1)
      do if(!isdigit((uchar)*--p2)) { // remove 'wait time' description
        if(*p2 == '<' && p2 > p1 && *--p2 == '$') {
          if(p2 == p1) return(NULL);
          if(atoi(p2+2) <= 0) return(NULL);
          sz = p2 - p1;
        }
        break;
      }while(p2 > p1);
    if(sz >= 256) return(NULL);
    *len = sz;
    return(p1);
}

//-----------------------------------------------------
static char *load_pstr(const char *id)
{
    char    *p, *out;
    size_t  sz;

    if((p = get_pstr(id, &sz)) == NULL) return(NULL);
    out = qmalloc(sz + 1);
    *out = (char)sz;
    memcpy(out+1, p, sz);
    return(out);
}

//-----------------------------------------------------
static uchar termout_init(signed char noansi, unsigned char std_term)
{ // 1 linux, 2 xterm, 3 scoansi, 4 xterm-sco...
    char    *p;
    size_t  sz;

    if((clear_scr = load_pstr("cl")) == NULL) return(0);  // clear
    if((p = load_pstr("cm")) == NULL) return(0); // cup
    sz = (uchar)*p;
    memmove(p, p+1, sz); //not memcpy for valgrng :)
    p[sz] = '\0';
    if(strcmp(p, "\e[%i%p1%d;%p2%dH") && strcmp(p, "\e[%i%d;%dH")) {
      if(!Pgoto(p, 0, 0)) return(0); // validate paramstring
      cur_move = p;
      curs_moveto = _moveto_ncurses;
    }

    if(!get_pstr("AF", &sz) || !get_pstr("AB", &sz)) { // setaf / setab
      if(!noansi)
        error("Terminal descriptor does not have the ansi-color command.\n"
              "\tYou may want to append 'mono' or 'ansi' to TVOPT");
      if(noansi > 0) mode.ansi_force = 1;
      else           mode.ansi_none  = 1; // monochrome
    }

    bell_line = load_pstr("bl");  // bel (needed?)

    // show/hide cursor
    if((cur_vis = load_pstr("ve")) != NULL) // cnorm
        cur_invis = load_pstr("vi");        // civis  (also as FLAG)

    if(   (page_G0 = load_pstr("ae")) != NULL     // rmacs
       && (page_G1 = load_pstr("as")) != NULL     // smacs
       && (pages_init = load_pstr("eA")) == NULL  // enacs (enable alt-pages)
       && std_term == 1   // BUG (terminfo/vt emulator in linux)
       && *page_G0 == sizeof("\e[10m")-1
       && !memcmp(page_G0 + 1, "\e[10m", sizeof("\e[10m")-1)) {

      free((void *)page_G0);
      free((void *)page_G1);
      page_G0 = "\x01\x0F";
      page_G1 = "\x01\x0E";
    }

    return(1);
}

//-----------------------------------------------------
static void termout_internal(uchar std_term)  // 1 - linux, 2 xterm
{ // 1 linux, 2 xterm, 3 scoansi, 4 xterm-sco...
    bell_line   = "\x01\a";
    clear_scr   = "\x07\e[H\e[2J";
    cur_vis     = "\x06\e[?25h";
    cur_invis   = "\x06\e[?25l";
    page_G0     = "\x01\x0F";
    page_G1     = "\x01\x0E";
    switch(std_term) {
      case 1: // linux
        clear_scr = "\x06\e[H\e[J";
        cur_vis   = "\x0B\e[?25h\e[?0c";
        cur_invis = "\x0B\e[?25l\e[?1c";
        break;
      case 3: // scoansi
        page_G0   = "\x05\e[10m";
        page_G1   = "\x05\e[12m";
        break;
      default: // xterm, xterm-sco...
        pages_init  = "\x07\e(B\e)0";
        break;
    }
}

//---------------------------------------------------------------------------
static void addstateidx(unsigned state, ushort idx)
{
    ushort *p = states[state];
    size_t len = 0;

    if(p) for(ushort *q = p; *q; q++) ++len;
    p = (ushort *)realloc(p, sizeof(ushort)*(len+2));
    if(!p) abort();
    // find the right place
    ushort *q = p;
    size_t i;
    uchar chr = keymap[idx].value;
    for(i = 0; i < len && keymap[*q].value >= chr; i++) ++q;
    // make room
    int rest = len - i;
    if(rest > 0) memmove(q+1, q, rest*sizeof(ushort));
    *q = idx;
    p[len+1] = 0;
    states[state] = p;
}

//---------------------------------------------------------------------------
static unsigned get_free_state(void)
{
    unsigned state;

    if((state = ++states_size) >= qnumber(states)) error("too many states");
    return(state);
}

//---------------------------------------------------------------------------
static const char *addkey(const char *seq, ushort code, uchar mods,
                          unsigned from)
#define addesc(a,b,c)  addkey(a,b,c,1)

{
    unsigned  idx, state = from;
    uchar     c = *seq++;

    if(!keymap_size) goto force_first;

    for( ; ; ) {
      if((idx = findstateidx(state, c)) == 0) { // this char is not present
force_first:
        if((idx = ++keymap_size) >= qnumber(keymap)) error("too many keys");
        keymap[idx].value = c;
        addstateidx(state, idx);
      }
      if((c = *seq++) == 0) {
        if(!keymap[idx].islast) {
          if(keymap[idx].next) return("short definition");
          keymap[idx].islast = 1;
          keymap[idx].tvcode = code;
          keymap[idx].mods   = mods;
        } else {
          if(keymap[idx].tvcode != code || keymap[idx].mods != mods)
              return("duplicate definition");
        }
        break;
      }
      if(keymap[idx].islast) return("long definition");  // islast
      if((state = keymap[idx].next) == 0)
        keymap[idx].next = (ushort)(state = get_free_state());
    }
    return(NULL);
}

//---------------------------------------------------------------------------
static int findkey(const char *seq)
{
  ushort  state = 1;
  char    c = *seq++;

  for( ; ; ) {
    unsigned idx = findstateidx(state, c);

    if(!idx) return(0);   // not found
    if((c = *seq++) == 0) {  // overlap or found
      if(keymap[idx].islast) return(idx); // found
      break;  // overlap
    }
    if(keymap[idx].islast) break; // partial overlap //islast
    if((state = keymap[idx].next) == 0) return(0); // not found
  }
  return(-1); // overlap
}

//---------------------------------------------------------------------------
#ifndef DUMP_TREE_ONLY
#define print_tree()
#else
static void print_tree(void) __attribute__((noreturn));
static void print_tree(void)
{
    FILE *f = fopen(DUMP_TREE_ONLY, "w");

    if(!f) f = stderr;
    for(size_t i = 0; i < qnumber(ctrl_map); i++)
      if(ctrl_map[i].tvcode)
        fprintf(f, "CHAR: ^%c      CODE: %04X/%02X\n", i + 0x40,
                ctrl_map[i].tvcode, ctrl_map[i].mods);
      else if(i != '\e' && ctrl_map[i].state)
        fprintf(f, "CHAR: ^%c      NODE: %d\n", i + 0x40,
                ctrl_map[i].state);

    fputs("-------\n", f);

    for(size_t i = 1; i <= states_size; i++) {
      fprintf(f, "STATE %d\n", i);
      for(ushort *pidx = states[i], idx; (idx = *pidx) != 0; pidx++) {
        node *p = &keymap[idx];

        fprintf(f, "\t%3d: CHAR: %c ", idx, p->value);
        if(p->islast)
          fprintf(f, " CODE: %04X/%02X\n", p->tvcode, p->mods);
        else
          fprintf(f, "              NEXT: %d\n", p->next);
      }
    }
    fputs("TREE COMPLETE\n", f);

    if(overlapXmice_code)
      fprintf(stderr,
              "\nXMouse sequence has been resolved with overlapping %04X/%02X\n",
              overlapXmice_code, overlapXmice_mods);

    if(f != stderr) {
      fclose(f);
      error("Keyboard tree dumped to" DUMP_TREE_ONLY);
    }
    abort();
}
#endif  // DUMP_TREE_ONLY

//---------------------------------------------------------------------------
static const char *addctrl(const char *p, ushort code, uchar mods)
{
    unsigned uc;

    switch(uc = (uchar)p[-1]) {
      case '\n':
      case '\t':
      case '\b':  // backspace
        return(NULL);
      default:
        break;
    }

    if(ctrl_map[uc].tvcode) {
      if(*p) return("long definition");
      if(ctrl_map[uc].tvcode != code || ctrl_map[uc].mods != mods)
        return("duplicate definition");
    } else {
      unsigned state = ctrl_map[uc].state;
      if(*p) {
        if(!state) ctrl_map[uc].state = (ushort)(state = get_free_state());
        return(addkey(p, code, mods, state));
      }
      if(state) return("long definition");
      ctrl_map[uc].tvcode = code;
      ctrl_map[uc].mods   = mods;
    }
    return(NULL);
}

//---------------------------------------------------------------------------
static const char PUTTY_STRING[] = "PuTTY";
static inline int isPuTTY(void)
{
    int   r;

    while(qgetch() != EOF) /* EMPTY */;
    *(uchar *)&r = 5; // ENQ
    term_out((char *)&r, 1);
    sleep(1);
    for(size_t i = 0; i < sizeof(PUTTY_STRING)-1; i++) {
      if((r = qgetch()) == EOF) return(0);
      if((uchar)r != PUTTY_STRING[i]) break;
    }
    while(qgetch() != EOF) /* EMPTY */;
    return(r == sizeof(PUTTY_STRING)-1);
}

//---------------------------------------------------------------------------
static void termkbd_init(signed char std_term)
{ // 1 linux, 2 xterm, 3 scoansi, 4 xterm-sco...
#define S kblShift
#define C kblCtrl
#define A kblAltL

    struct tinfo {
      ushort  code;
      uchar   mods;
      char    id[3];
      char    _xterm[7];
      char    _linux[5];
      char    _sco[3];
    };

#define _EMP_ ""
    static const tinfo key_defs[] = {
      {  kbF1,     0, "k1", "OP",     "[[A",  "[M"  }, // kf1
      {  kbF2,     0, "k2", "OQ",     "[[B",  "[N"  }, // kf2
      {  kbF3,     0, "k3", "OR",     "[[C",  "[O"  }, // kf3
      {  kbF4,     0, "k4", "OS",     "[[D",  "[P"  }, // kf4
      {  kbF5,     0, "k5", "[15~",   "[[E",  "[Q"  }, // kf5
      {  kbF6,     0, "k6", "[17~",   "[17~", "[R"  }, // kf6
      {  kbF7,     0, "k7", "[18~",   "[18~", "[S"  }, // kf7
      {  kbF8,     0, "k8", "[19~",   "[19~", "[T"  }, // kf8
      {  kbF9,     0, "k9", "[20~",   "[20~", "[U"  }, // kf9
      { kbF10,     0, "k;", "[21~",   "[21~", "[V"  }, // kf10
      { kbF11,     0, "F1", "[23~",   "[23~", "[W"  }, // kf11
      { kbF12,     0, "F2", "[24~",   "[24~", "[X"  }, // kf12
      {  kbF1,   S,   "F3", "O2P",    "[25~", "[Y"  }, // kf13
      {  kbF2,   S,   "F4", "O2Q",    "[26~", "[-"  }, // kf14
      {  kbF3,   S,   "F5", "O2R",    "[28~", "[a"  }, // kf15
      {  kbF4,   S,   "F6", "O2S",    "[29~", "[b"  }, // kf16
      {  kbF5,   S,   "F7", "[15;2~", "[31~", "[c"  }, // kf17
      {  kbF6,   S,   "F8", "[17;2~", "[32~", "[d"  }, // kf18
      {  kbF7,   S,   "F9", "[18;2~", "[33~", "[e"  }, // kf19
      {  kbF8,   S,   "FA", "[19;2~", "[34~", "[f"  }, // kf20
      {  kbF9,   S,   "FB", "[20;2~", _EMP_,  "[g"  }, // kf21
      { kbF10,   S,   "FC", "[21;2~", _EMP_,  "[h"  }, // kf22
      { kbF11,   S,   "FD", "[23;2~", _EMP_,  "[i"  }, // kf23
      { kbF12,   S,   "FE", "[24;2~", _EMP_,  "[j"  }, // kf24
      {  kbF1,     C, "FF", "O5P",    _EMP_,  "[k"  }, // kf25
      {  kbF2,     C, "FG", "O5Q",    _EMP_,  "[l"  }, // kf26
      {  kbF3,     C, "FH", "O5R",    _EMP_,  "[m"  }, // kf27
      {  kbF4,     C, "FI", "O5S",    _EMP_,  "[n"  }, // kf28
      {  kbF5,     C, "FJ", "[15;5~", _EMP_,  "[o"  }, // kf29
      {  kbF6,     C, "FK", "[17;5~", _EMP_,  "[p"  }, // kf30
      {  kbF7,     C, "FL", "[18;5~", _EMP_,  "[q"  }, // kf31
      {  kbF8,     C, "FM", "[19;5~", _EMP_,  "[r"  }, // kf32
      {  kbF9,     C, "FN", "[20;5~", _EMP_,  "[s"  }, // kf33
      { kbF10,     C, "FO", "[21;5~", _EMP_,  "[t"  }, // kf34
      { kbF11,     C, "FP", "[23;5~", _EMP_,  "[u"  }, // kf35
      { kbF12,     C, "FQ", "[24;5~", _EMP_,  "[v"  }, // kf36
      {  kbF1,   S+C, "FR", "O6P",    _EMP_,  "[w"  }, // kf37
      {  kbF2,   S+C, "FS", "O6Q",    _EMP_,  "[x"  }, // kf38
      {  kbF3,   S+C, "FT", "O6R",    _EMP_,  "[y"  }, // kf39
      {  kbF4,   S+C, "FU", "O6S",    _EMP_,  "[z"  }, // kf40
      {  kbF5,   S+C, "FV", "[15;6~", _EMP_,  "[@"  }, // kf41
      {  kbF6,   S+C, "FW", "[17;6~", _EMP_,  "[["  }, // kf42
      {  kbF7,   S+C, "FX", "[18;6~", _EMP_,  "[\\" }, // kf43
      {  kbF8,   S+C, "FY", "[19;6~", _EMP_,  "[]"  }, // kf44
      {  kbF9,   S+C, "FZ", "[20;6~", _EMP_,  "[^"  }, // kf45
      { kbF10,   S+C, "Fa", "[21;6~", _EMP_,  "[_"  }, // kf46
      { kbF11,   S+C, "Fb", "[23;6~", _EMP_,  "[`"  }, // kf47
      { kbF12,   S+C, "Fc", "[24;6~", _EMP_,  "[{"  }, // kf48
    // maximum possible kf63 (alplabetize)
      { kbHome,    0, "kh", "OH",     "[1~",  "[H"  }, // khome/
      { kbHome,  S  , "#2", "[1;2H",  _EMP_,  _EMP_ }, // kHOM
      { kbIns,     0, "kI", "[2~",    "[2~",  "[L"  }, // kich1
      { kbIns,   S  , "#3", "[2;2~",  _EMP_,  "[("  }, // kIC (last - scokeys)
      { kbDel,     0, "kD", "[3~",    "[3~",  _EMP_ }, // kdch1 (sco-177 :)
      { kbDel,   S  , "*4", "[3;2~",  _EMP_,  "[)"  }, // kDC (last - scokeys)
      { kbEnd,     0, "@7", "OF",     "[4~",  "[F"  }, // kend
      { kbEnd,   S  , "*7", "[1;2F",  _EMP_,  _EMP_ }, // kEND
      { kbPgUp,    0, "kP", "[5~",    "[5~",  "[I"  }, // kpp
      { kbPgUp,    0, "%8", _EMP_,    _EMP_,  _EMP_ }, // kprv
      { kbPgUp,  S  , "%e", "[5;2~",  _EMP_,  _EMP_ }, // kPRV
      { kbPgDn,    0, "kN", "[6~",    "[6~",  "[G"  }, // knp
      { kbPgDn,    0, "%5", _EMP_,    _EMP_,  _EMP_ }, // knxt
      { kbPgDn,  S  , "%c", "[6;2~",  _EMP_,  _EMP_ }, // kNXT
    //
      { kbUp,      0, "ku", "OA",     "[A",   "[A"  }, // kcuu1
      { kbDown,    0, "kd", "OB",     "[B",   "[B"  }, // kcud1
      { kbRight,   0, "kr", "OC",     "[C",   "[C"  }, // kcuf1
      { kbRight,   S, "%i", "[1;2C",  _EMP_,  _EMP_ }, // kRIT
      { kbLeft,    0, "kl", "OD",     "[D",   "[D"  }, // kcub1
      { kbLeft,    S, "#4", "[1;2D",  _EMP_,  _EMP_ }, // kLFT
    //
//      { kbBack,    0,  }, // kbs // always ^H or 127 :)
      { kbTab,     S, "kB", "[Z",     _EMP_,  "[Z"  }, // kcbt
      { kbEnter,   0, "@8", "EOM",    _EMP_,  _EMP_ }, // kent (enter/send)
    // keypad
      { '5',       0, "K2", "EOE",    "[G",   _EMP_ }, // kb2 (center of keypad)
    // MUST be last!
      { kbMouse,   0, "Km", "[M",     "[M",   _EMP_ }  // kmous (sco - alias)
    };
#undef _EMP_


    size_t      i, off;
    const char  *p;

    off = 0;
    if(std_term < 0) {
      std_term = -std_term;
      off += (offsetof(tinfo, _linux) - offsetof(tinfo, id));
      if(std_term != 1) {
        off += (offsetof(tinfo, _xterm) - offsetof(tinfo, _linux));
        if(std_term != 2)  // sconasi OR xterm-sco
          off += (offsetof(tinfo, _sco) - offsetof(tinfo, _xterm));
      }
    }

    ctrl_map['\e'].state = 1; // unification
    for(i = 0; i < qnumber(key_defs); i++) {
      uchar uc;

      p = &key_defs[i].id[off];
      if(!off) {  // use terminfo
        if((p = Pgetstr((char *)p, NULL)) == NULL) continue;
        if((uc = *p++) >= ' ') continue; // illegal => ignored
        if(uc != '\e') {
          if((p = addctrl(p, key_defs[i].code, key_defs[i].mods)) != NULL)
              goto print_err_code;
          continue;
        }
      }
      if(!*p) continue; // illegal => ignored
      if((p = addesc(p, key_defs[i].code, key_defs[i].mods)) != NULL) {
print_err_code:
          error("TERMINFO: %s for '%s'", p, key_defs[i].id);
      }
      if(i == qnumber(key_defs) - 1) { // kmous
        work.have_xmice = 1;
        goto xmice_done;
      }
    }

/* standart definitions (overlap bad clients/records) */
    if((i = findkey("[M")) == 0) { // can add
      if(work.have_xmice) goto set_mice_key;
      set_tty_mode(&my_tios, tty_mode | O_NONBLOCK);
      i = isPuTTY();  // putty always send reports :(
      set_tty_mode(&old_tios, tty_mode);
      if(i) {
        work.have_xmice = 1;
set_mice_key:
        addesc("[M", kbMouse, 0);
      }
    } else if((int)i > 0 && std_term) {
      overlapXmice_mods = state2tvstate(keymap[i].mods);
      keymap[i].mods    = 0;
      overlapXmice_code = keymap[i].tvcode;
      keymap[i].tvcode  = kbMouse;
      if(!hGPM) work.have_xmice = 1;
    }

xmice_done:
    if(std_term == 2) { // xterm
      static const struct { // xterm cursor keys in 'Application mode'
        ushort  id;
        char    str[3];
      }xterm_noapp_arrow[4] = {
        { kbUp,    "[A" },
        { kbDown,  "[B" },
        { kbRight, "[C" },
        { kbLeft,  "[D" }
      };

      for(i = 0; i < qnumber(xterm_noapp_arrow); i++)
        if(findkey(xterm_noapp_arrow[i].str)) goto no_append_xt1;
      for(i = 0; i < qnumber(xterm_noapp_arrow); i++)
        addesc(xterm_noapp_arrow[i].str, xterm_noapp_arrow[i].id, 0);
no_append_xt1:

      static const struct { // xterm-app home/end
        ushort  id;
        char    str[4];
      }xterm_he[2] = {
        { kbHome,   "[1~" },
        { kbEnd,    "[4~" }
      };

      for(i = 0; i < qnumber(xterm_he); i++)
        if(findkey(xterm_he[i].str)) goto no_append_xt2;
      for(i = 0; i < qnumber(xterm_he); i++)
        addesc(xterm_he[i].str, xterm_he[i].id, 0);
no_append_xt2:

      static const struct { // xterm-old
        ushort  id;
        char    str[6];
      }xterm_alias[4] = {
        { kbHome,   "O2H" },
        { kbEnd,    "O2F" },
        { kbRight,  "O2C" },
        { kbLeft,   "O2D" }
      };

      for(i = 0; i < qnumber(xterm_alias); i++)
        if(findkey(xterm_alias[i].str)) goto no_append_xt3;
      for(i = 0; i < qnumber(xterm_alias); i++)
        addesc(xterm_alias[i].str, xterm_alias[i].id, kblShift);
no_append_xt3:

      static const struct { // xterm-old (putty)
        ushort  id;
        uchar   mods;
        char    str[5];
      }xterm_putty[8] = {
        { kbF1,  0, "[11~" },
        { kbF2,  0, "[12~" },
        { kbF3,  0, "[13~" },
        { kbF4,  0, "[14~" },
        { kbF1,  S, "[25~" },
        { kbF2,  S, "[26~" },
        { kbF3,  S, "[28~" },
        { kbF4,  S, "[29~" }
      };

      for(i = 0; i < qnumber(xterm_putty); i++)
        if(findkey(xterm_putty[i].str)) goto no_append_xt;
      for(i = 0; i < qnumber(xterm_putty); i++)
        addesc(xterm_putty[i].str, xterm_putty[i].id, xterm_putty[i].mods);
    }

no_append_xt:
    if(std_term & 6) { // xterm-sco... (and old xterm? :)
      static const struct {
        ushort  id;
        char    str[4];
      }xtc_keys[4] = {
        { kbUp,     "O2A" },
        { kbDown,   "O2B" },
        { kbRight,  "O2C" },
        { kbLeft,   "O2D" }
      };
      for(i = 0; i < qnumber(xtc_keys); i++)
        if(findkey(xtc_keys[i].str)) goto no_append_xtc;
      for(i = 0; i < qnumber(xtc_keys); i++)
        addesc(xtc_keys[i].str, xtc_keys[i].id, kblShift);
    }

no_append_xtc:
    if(std_term != 1) { // attemt to add rxvt special keys (for telnet users)
      static const struct {
        ushort  id;
        uchar   mods;
        char    str[4];
      }rxvt_adds[6] = {
        { kbPgUp, C,  "[5^" },
        { kbPgDn, C,  "[6^" },
        { kbHome, C,  "[7^" },
        { kbEnd,  C,  "[8^" },
        { kbHome, S,  "[7$" },
        { kbEnd,  S,  "[8$" }
      };
#undef S
#undef C
#undef A

      for(i = 0; i < qnumber(rxvt_adds); i++)
        if(findkey(rxvt_adds[i].str)) goto no_append_rxvt;
      for(i = 0; i < qnumber(rxvt_adds); i++)
        addesc(rxvt_adds[i].str, rxvt_adds[i].id, rxvt_adds[i].mods);
    }
no_append_rxvt:
    print_tree();
}

//---------------------------------------------------------------------------
static void set_all_signals(sighandler_t handler)
{
    signal(SIGWINCH, handler);
    signal(SIGTERM, handler);
    signal(SIGSTOP, handler);
    signal(SIGHUP,  handler);
    signal(SIGUSR1, handler);
    signal(SIGUSR2, handler);
}

//---------------------------------------------------------------------------
// TScreen is a singleton :)
TScreen::~TScreen()
{
  // implement for dynamic unloading only
    delete[] TScreen::screenBuffer;

    detach_tty();

    set_all_signals(SIG_DFL);

//X11-console
    if(x11_display) {
      pXCloseDisplay(x11_display);
      dlclose(hX11);
    }
    if(hGPM)    dlclose(hGPM);
    if(hCurses) dlclose(hCurses);
}

//---------------------------------------------------------------------------
TScreen::TScreen(void)
{
    static  uchar loaded;

    char  *p, *ps;
    signed char no_x11, noansi, no_gpm, ign_meta8;

    if(loaded) error("Multiple TV instances");
    ++loaded;

    if(   !isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)
       || (tty_mode = fcntl(STDIN_FILENO, F_GETFL, 0)) == -1
       || (tty_mode & O_ACCMODE) != O_RDWR
       || tcgetattr(STDIN_FILENO, &old_tios))
                        error("Can not work with redirected stdin/stdout");

    my_tios = old_tios;
    my_tios.c_iflag |= (IGNBRK | BRKINT); // Ignore breaks
    my_tios.c_iflag &= ~(IXOFF | IXON);   // Disable Xon/off
    my_tios.c_lflag &= ~(ICANON | ECHO | ISIG); // Character oriented, no echo, no signals
    my_tios.c_oflag |= OPOST;  // apply special output sequence

    if((p = getenv("TVLOG")) != NULL && *p) {
      FILE *f;
      char resolved[PATH_MAX];

      if(   (!realpath(p, resolved) && errno != ENOENT)
         || (f = fopen(resolved, "w")) == NULL)
          error("Invalid/inaccessibble TVLOG file name in the environment");
      fclose(f);
      logname = strdup(resolved);
    } else openlog(PRG_LOG_NAME, 0, LOG_USER);

    no_x11 = noansi = no_gpm = ign_meta8 = 0;
    SM_DOSCYR_DEFAULT;  // mode.(doscyr_ninp=doscyr_nout=1)
    if((ps = getenv("TVOPT")) != NULL) {
      ps = qstrdup(ps);
      for(p = strtok(ps, ","); p; p = strtok(NULL, ","))
        if(!strcasecmp(p, "noX11")) ++no_x11;
        else if(!strcasecmp(p, "noGPM")) ++no_gpm;
        else if(!strcasecmp(p, "ign8")) ++ign_meta8;
        else if(!strcasecmp(p, "alt866")) mode.alt866_font = 1;
        else if(!strcasecmp(p, "xtrack")) XMOU_SET_TRACK;
        else if(!strcasecmp(p, "mono")) {
          if(noansi > 0) goto ansi_uncompat;
          noansi = -1;
        } else if(!strcasecmp(p, "ansi")) {
          if(noansi < 0) {
ansi_uncompat:
            error("TVOPT: 'ansi' and 'mono' are uncompatible");
          }
          noansi = 1;
        } else if(!strncasecmp(p, "cyrcvt=", sizeof("cyrcvt=")-1)) {
          if(SM_DOSCYR_CHANGED) error("Duplicate cyrcvt in TVOPT");
          p += sizeof("cyrcvt=")-1;
          if(!strcasecmp(p, "windows")) {
            mode.doscyr_owin = 1;
            goto cyrcvt_kwin;
          }
          if(!strcasecmp(p, "kwin")) {
cyrcvt_kwin:
            mode.doscyr_kwin = 1;
            goto cyrcvt_all;
          }
          if(!strcasecmp(p, "linux")) {
cyrcvt_all:
            SM_DOSCYR_ENABLE;  // mode.(doscyr_ninp=doscyr_nout)=0
          } else error("Possible values for cyrcvt are: linux, windows, kwin");
        } else error("Possible fields in TVOPT are:\n\t"
                  "noX11, noGPM, ign8, ansi, mono, alt866, xtrack, cyrcvt=");
      free(ps);
    }

//---
    ++pfd_count;  // 1
    pfd_data[0].fd = STDIN_FILENO;
    pfd_data[0].events = pfd_data[1].events = POLLIN;
    con_mou.tioc = 2;
    {
      int tmp = 7;  // get mouse reporting (check tty==con :)
      if(ioctl(STDIN_FILENO, TIOCLINUX, &tmp) != -1) work.pc_console = 1;
    }
    if(work.pc_console && (hGPM = dlopen("libgpm.so", RTLD_NOW)) != NULL) {
      pGpm_GetEvent = (tGpm_GetEvent)dlsym(hGPM, "Gpm_GetEvent");
      if(dlerror()) goto drop_gpm;
      pGpm_Close = (tGpm_Close)dlsym(hGPM, "Gpm_Close");
      if(dlerror()) goto drop_gpm;
      pGpm_Open = (tGpm_Open)dlsym(hGPM, "Gpm_Open");
      if(dlerror()) {
drop_gpm:
        if(!no_gpm)
          error("Invalid libgpm.so\n"
                "\tTemporary you may want to append 'noGPM' to TVOPT");

        dlclose(hGPM);
        hGPM = NULL;
      }
    }

//---
    curs_moveto = _moveto_vt; // preinit (standard)

    p = getenv("TERM");
    if(p && !*p) p = NULL;  // paranoidally speed

    no_gpm = 0; // used as std_term_type
    if(work.pc_console) goto set_term_linux;

//X11-console (speed - no check X11 if tty is console :)
    if((hX11 = dlopen("libX11.so", RTLD_NOW)) != NULL) {
      tXOpenDisplay pXOpenDisplay;

      pXCloseDisplay = (tXCloseDisplay)dlsym(hX11, "XCloseDisplay");
      if(dlerror()) goto drop_x11;
      pXOpenDisplay  = (tXOpenDisplay)dlsym(hX11, "XOpenDisplay");
      if(dlerror()) goto drop_x11;
      pXQueryPointer = (tXQueryPointer)dlsym(hX11, "XQueryPointer");
      if(dlerror()) {
drop_x11:
        if(no_x11) goto ignore_x11;
        error("Invalid libX11.so\n"
                "\tTemporary you may want to append 'noX11' to TVOPT");
      }
      if((x11_display = pXOpenDisplay(NULL)) != NULL) {
        x11_window = DefaultRootWindow(x11_display);
        goto set_term_xterm;
      }
ignore_x11:
      dlclose(hX11);
    }

    if(!p) error("Undefined TERM environment variable");
    if(!strncmp(p, "xterm", sizeof("xterm")-1)) {
      if(!strncmp(p, "xterm-sco", sizeof("xterm-sco")-1)) { // alias kbd
        ++no_gpm; // std_term_type = xterm-sco(4)
set_term_sco:
        ++no_gpm; // std_term_type = scoansi(3)
      }
set_term_xterm:
      work.have_xmice = 1;
      ++no_gpm;   // std_term_type = xterm(2)
      ++no_gpm;
    } else if(!strncmp(p, "linux", sizeof("linux")-1)) {
set_term_linux:
      ++no_gpm; // std_term_type = linux(1)
    } else if(!strncmp(p, "scoansi", sizeof("scoansi")-1)) goto set_term_sco;

//    if(no_gpm) goto without_libcurses;
    if((hCurses = dlopen("libcurses.so", RTLD_NOW)) != NULL) {
      typedef int (*Tgetent)(char *, const char *);
      typedef int (*Tgetflag)(char *);

      Tgetent   Pgetent;
      Tgetflag  Pgetflag;

      Pgoto = (Tgoto)dlsym(hCurses, "tgoto");
      if(dlerror()) goto drop_curses;
      Pgetstr  = (Tgetstr)dlsym(hCurses, "tgetstr");
      if(dlerror()) goto drop_curses;
      Pgetent = (Tgetent)dlsym(hCurses, "tgetent");
      if(dlerror()) goto drop_curses;
      Pgetflag = (Tgetflag)dlsym(hCurses, "tgetflag");
      if(dlerror()) {
drop_curses:
        if(!no_gpm)
          error("Invalid libcurses.so\n"
                "\tTemporary you can work without it but only if the terminal is:\n"
                "\txterm, linux or scoansi.");
        dlclose(hCurses);
        hCurses = NULL;
        goto without_ncurses;
      }
      if(Pgetent(NULL, p) != 1 || !termout_init(noansi, no_gpm))
        error("Dumb terminal %s is not supported\n", p);

      if(Pgetflag("km")) mode.meta_8bit = 1;
    } else { // no loaded (attempt to work without ncurses)
      if(!no_gpm) error("Can not load libcurses.so\n\t"
                        "Without libcurses can work only with xterm/linux");
without_ncurses:
      if(!(no_gpm & 1)) mode.meta_8bit = 1; // xterm[-xxx]
      termout_internal(no_gpm);
      no_gpm = -no_gpm;
    }
    if(ign_meta8) mode.meta_8bit = 0;
//no_gpm = 1-linux, 2-xterm, 3-sco; if < 0 - use it, else have terminfo
    termkbd_init(no_gpm);
    if(hCurses && !cur_move) {  // memory always needed :)
      dlclose(hCurses);
      hCurses = NULL;
    }

// singleton initialization

    /* acquire screen size */
    {
      winsize win;
      if(ioctl(0, TIOCGWINSZ, &win) != -1) {
        if(win.ws_col <= 0 || win.ws_row <= 0) abort();
        screenWidth = range(win.ws_col, 4, maxViewWidth);
        screenHeight = range(win.ws_row, 4, 80);
#ifdef WINCHG_DEBUG
        LOG("screen size is %dx%d", screenWidth, screenHeight);
#endif
      } else {
        LOG("unable to detect the screen size, using 80x25");
        screenWidth = 80;
        screenHeight = 25;
      }
    }
    screenBuffer = new ushort[screenWidth * screenHeight];

    /* catch useful signals */
    set_all_signals(sigHandler);

    /* internal stuff */
    evIn = evOut = evQueue;
    _timeBase = time(NULL);
    wakeupTimer.set(get_system_time() + DELAY_WAKEUP);

    if(!logname) closelog(); // for standard handling
}

//---------------------------------------------------------------------------

#endif // __LINUX__
