/* $Id: //depot/ida/tvision/source/linuxx.cpp#5 $ */
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
/* Modified by Jeremy Cooper, <jeremy@baymoo.org> */

#if defined(__LINUX__) || defined(__MAC__)
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
#define Uses_TThreaded
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

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>

#define ADVANCED_CLIPBOARD 1

//
// Description of an X11 color string update block as determined by the
// screen update engine.
//
typedef struct {
  enum { UPD_NONE, UPD_UPDATE, UPD_REDUNDANT };
  char *characters;
  uint16 len;
  uint16 x;
  uint8 state;
  uint8 color;
} UpdateBlock;


static void writeRow(int dst, ushort *src, int len, bool isExpose);
static Bool findSelectionNotifyEvent(Display *d, XEvent *e, XPointer arg);

static Display *x_disp;
static Window x_win;
static GC *characterGC;
static GC cursorGC;
static int x_disp_fd;
//
// The last X Event received.
//
static XEvent xEvent;

#ifdef ADVANCED_CLIPBOARD
//
// The atom we use for receiving clipboard contents.
//
static Atom clipboardAtom;
static Atom XA_CLIPBOARD;
static char *clipboardBuffer;
static int clipboardSize;
#endif

static bool inited = false;

static int lastMods;
static Time last_release_time;
static int last_release_button;
static int keyboardModifierState;
static Region expose_region;

#define	APP_CLASS	"TVision"
#define	APP_NAME	"TVision"

#define TICKS_TO_MS(x)	((x) * 1000/18)
#define MS_TO_TICKS(x)  ((x) * 18/1000)

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
 * Default text display font.
 */
#define DEFAULT_FONT		"vga"

/*
 * X Window border width (in pixels).
 */
#define	BORDER_WIDTH	1

/*
 * Default background and foreground colors.
 */
#define DEFAULT_BGCOLOR_0	"black"
#define DEFAULT_BGCOLOR_1	"blue4"
#define DEFAULT_BGCOLOR_2	"green4"
#define DEFAULT_BGCOLOR_3	"cyan4"
#define DEFAULT_BGCOLOR_4	"red4"
#define DEFAULT_BGCOLOR_5	"magenta4"
#define DEFAULT_BGCOLOR_6	"brown"
#define DEFAULT_BGCOLOR_7	"grey80"
#define DEFAULT_BGCOLOR_8	"grey30"
#define DEFAULT_BGCOLOR_9	"blue"
#define DEFAULT_BGCOLOR_10	"green"
#define DEFAULT_BGCOLOR_11	"cyan"
#define DEFAULT_BGCOLOR_12	"red"
#define DEFAULT_BGCOLOR_13	"magenta"
#define DEFAULT_BGCOLOR_14	"yellow"
#define DEFAULT_BGCOLOR_15	"white"
/* Foreground colors */
#define DEFAULT_FGCOLOR_0	"black"
#define DEFAULT_FGCOLOR_1	"blue4"
#define DEFAULT_FGCOLOR_2	"green4"
#define DEFAULT_FGCOLOR_3	"cyan4"
#define DEFAULT_FGCOLOR_4	"red4"
#define DEFAULT_FGCOLOR_5	"magenta4"
#define DEFAULT_FGCOLOR_6	"brown"
#define DEFAULT_FGCOLOR_7	"grey80"
#define DEFAULT_FGCOLOR_8	"grey30"
#define DEFAULT_FGCOLOR_9	"blue"
#define DEFAULT_FGCOLOR_10	"green"
#define DEFAULT_FGCOLOR_11	"cyan"
#define DEFAULT_FGCOLOR_12	"red"
#define DEFAULT_FGCOLOR_13	"magenta"
#define DEFAULT_FGCOLOR_14	"yellow"
#define DEFAULT_FGCOLOR_15	"white"

/*
 * Default window geometry.
 */
#define	DEFAULT_GEOMETRY_WIDTH	80
#define	DEFAULT_GEOMETRY_HEIGHT	25

/* Background text colors */
static char *bgColor[] = {
  DEFAULT_BGCOLOR_0,
  DEFAULT_BGCOLOR_1,
  DEFAULT_BGCOLOR_2,
  DEFAULT_BGCOLOR_3,
  DEFAULT_BGCOLOR_4,
  DEFAULT_BGCOLOR_5,
  DEFAULT_BGCOLOR_6,
  DEFAULT_BGCOLOR_7,
  DEFAULT_BGCOLOR_8,
  DEFAULT_BGCOLOR_9,
  DEFAULT_BGCOLOR_10,
  DEFAULT_BGCOLOR_11,
  DEFAULT_BGCOLOR_12,
  DEFAULT_BGCOLOR_13,
  DEFAULT_BGCOLOR_14,
  DEFAULT_BGCOLOR_15
};

/* Foreground text colors */
static char *fgColor[] = {
  DEFAULT_FGCOLOR_0,
  DEFAULT_FGCOLOR_1,
  DEFAULT_FGCOLOR_2,
  DEFAULT_FGCOLOR_3,
  DEFAULT_FGCOLOR_4,
  DEFAULT_FGCOLOR_5,
  DEFAULT_FGCOLOR_6,
  DEFAULT_FGCOLOR_7,
  DEFAULT_FGCOLOR_8,
  DEFAULT_FGCOLOR_9,
  DEFAULT_FGCOLOR_10,
  DEFAULT_FGCOLOR_11,
  DEFAULT_FGCOLOR_12,
  DEFAULT_FGCOLOR_13,
  DEFAULT_FGCOLOR_14,
  DEFAULT_FGCOLOR_15
};

static char *fontName = DEFAULT_FONT,
            *geometry = NULL;

static int fontHeight, fontAscent, fontDescent, fontWidth;

struct XResource {
  const char *name;
  const char *cls;
  char **value;
};
typedef struct XResource XResource;

static XResource TvisionResources[] = {
  { "tvision.text.bgcolor0",  "Tvision.Text.Bgcolor0",  &bgColor[0] },
  { "tvision.text.bgcolor1",  "Tvision.Text.Bgcolor1",  &bgColor[1] },
  { "tvision.text.bgcolor2",  "Tvision.Text.Bgcolor2",  &bgColor[2] },
  { "tvision.text.bgcolor3",  "Tvision.Text.Bgcolor3",  &bgColor[3] },
  { "tvision.text.bgcolor4",  "Tvision.Text.Bgcolor4",  &bgColor[4] },
  { "tvision.text.bgcolor5",  "Tvision.Text.Bgcolor5",  &bgColor[5] },
  { "tvision.text.bgcolor6",  "Tvision.Text.Bgcolor6",  &bgColor[6] },
  { "tvision.text.bgcolor7",  "Tvision.Text.Bgcolor7",  &bgColor[7] },
  { "tvision.text.bgcolor8",  "Tvision.Text.Bgcolor8",  &bgColor[8] },
  { "tvision.text.bgcolor9",  "Tvision.Text.Bgcolor9",  &bgColor[9] },
  { "tvision.text.bgcolor10", "Tvision.Text.Bgcolor10", &bgColor[10] },
  { "tvision.text.bgcolor11", "Tvision.Text.Bgcolor11", &bgColor[11] },
  { "tvision.text.bgcolor12", "Tvision.Text.Bgcolor12", &bgColor[12] },
  { "tvision.text.bgcolor13", "Tvision.Text.Bgcolor13", &bgColor[13] },
  { "tvision.text.bgcolor14", "Tvision.Text.Bgcolor14", &bgColor[14] },
  { "tvision.text.bgcolor15", "Tvision.Text.Bgcolor15", &bgColor[15] },
  { "tvision.text.fgcolor0",  "Tvision.Text.Fgcolor0",  &fgColor[0] },
  { "tvision.text.fgcolor1",  "Tvision.Text.Fgcolor1",  &fgColor[1] },
  { "tvision.text.fgcolor2",  "Tvision.Text.Fgcolor2",  &fgColor[2] },
  { "tvision.text.fgcolor3",  "Tvision.Text.Fgcolor3",  &fgColor[3] },
  { "tvision.text.fgcolor4",  "Tvision.Text.Fgcolor4",  &fgColor[4] },
  { "tvision.text.fgcolor5",  "Tvision.Text.Fgcolor5",  &fgColor[5] },
  { "tvision.text.fgcolor6",  "Tvision.Text.Fgcolor6",  &fgColor[6] },
  { "tvision.text.fgcolor7",  "Tvision.Text.Fgcolor7",  &fgColor[7] },
  { "tvision.text.fgcolor8",  "Tvision.Text.Fgcolor8",  &fgColor[8] },
  { "tvision.text.fgcolor9",  "Tvision.Text.Fgcolor9",  &fgColor[9] },
  { "tvision.text.fgcolor10", "Tvision.Text.Fgcolor10", &fgColor[10] },
  { "tvision.text.fgcolor11", "Tvision.Text.Fgcolor11", &fgColor[11] },
  { "tvision.text.fgcolor12", "Tvision.Text.Fgcolor12", &fgColor[12] },
  { "tvision.text.fgcolor13", "Tvision.Text.Fgcolor13", &fgColor[13] },
  { "tvision.text.fgcolor14", "Tvision.Text.Fgcolor14", &fgColor[14] },
  { "tvision.text.fgcolor15", "Tvision.Text.Fgcolor15", &fgColor[15] },
  { "tvision.text.font",      "Tvision.Text.Font",      &fontName },
  { "tvision.geometry",       "Tvision.Geometry",       &geometry },
};
static const int TvisionResources_count = qnumber(TvisionResources);

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
  exit(-1);
}

static void warning(const char *format, ...)
{
  va_list va;
  va_start(va, format);
  vfprintf(stderr, format, va);
  va_end(va);
}

ushort TEventQueue::doubleDelay = 8;
Boolean TEventQueue::mouseReverse = (enum Boolean) False;

ushort TScreen::screenMode;
int TScreen::screenWidth;
int TScreen::screenHeight;
ushort *TScreen::screenBuffer;
static ushort *currentScreenBuffer;
static char *currentChunkBuf;
static UpdateBlock *currentUpdateBlocks;

static TEvent *evIn;            /* message queue system */
static TEvent *evOut;
static TEvent evQueue[eventQSize];
static char env[PATH_MAX];      /* value of the TVOPT environment variable */
static int curX, curY;          /* current cursor coordinates */
static bool cursor_displayed;   /* is cursor visible? */
static int currentTime;         /* current timer value */
static int doRepaint;           /* should redraw the screen ? */
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

ulong getTicks(void)
{
  return currentTime * 18 / 1000;
}

uchar getShiftState(void)
{
  return keyboardModifierState;
}

/*
 * X DRAWING FUNCTIONS
 */
static void
handleXExpose(XEvent *event)
{
  XRectangle r;

  r.x = event->xexpose.x;
  r.y = event->xexpose.y;
  r.width = event->xexpose.width;
  r.height = event->xexpose.height;

  XUnionRectWithRegion(&r, expose_region, expose_region);

  if (event->xexpose.count == 0) {
    int row, res;

    for (row = 0; row < TScreen::screenHeight; row++) {
      /*
       * First, calculate if anything in this entire row is exposed.
       */
      r.x = 0;
      r.y = row * fontHeight;
      r.width = TScreen::screenWidth * fontWidth;
      r.height = fontHeight;

      res = XRectInRegion(expose_region, 0, row * fontHeight,
        fontWidth * TScreen::screenWidth, fontHeight);

      if (res != RectangleOut) {
        /*
         * Something in this row is exposed, just draw the whole row.
         */
        writeRow(row * TScreen::screenWidth,
                 &TScreen::screenBuffer[row * TScreen::screenWidth],
                 TScreen::screenWidth, true);
      }
    }

    XDestroyRegion(expose_region);
    expose_region = XCreateRegion();
  }
}

static void
drawTextCursor()
{
  XFillRectangle(x_disp, x_win, cursorGC, curX * fontWidth, curY * fontHeight,
                 fontWidth, fontHeight);
}

static void
eraseTextCursor()
{
  XFillRectangle(x_disp, x_win, cursorGC, curX * fontWidth, curY * fontHeight,
                 fontWidth, fontHeight);
}

/*
 * X KEYBOARD AND MOUSE TRANSLATION FUNCTIONS
 */

static inline int
translateXModifierState(int x_state)
{
  int state = 0;

  if (x_state & ShiftMask)
    state |= kbLeftShift;
  if (x_state & ControlMask)
    state |= kbLeftCtrl;
  if (x_state & LockMask)
    state |= kbCapsState;
  if (x_state & Mod1Mask)
    state |= kbLeftAlt;

  return state;
}

/*
 * Translate an X button number into a TVision button number.
 */
static inline int
translateXButton(int x_button)
{
  switch (x_button) {
  case Button1:
    return TEventQueue::mouseReverse ? mbRightButton : mbLeftButton;
  case Button3:
    return TEventQueue::mouseReverse ? mbLeftButton : mbRightButton;
  default:
    return 0;
  }
}



/*
 * Keyboard translation tables.
 *
 * These tables direct the translation of X keyboard events into
 * their TVision counterparts.  The tables definied here are "seed" tables;
 * they aren't used directly by the running code, but rather, they are used
 * as initialization values when creating the faster hash-based lookup table.
 */

struct KeyboardXlat {
  int in_code;
  int out_code;
};
typedef struct KeyboardXlat KeyboardXlat;

//
// Translation table for unmodified key presses.
//
static const KeyboardXlat plainXlatSeed[] = {
  { XK_Escape,          kbEsc       },
  { XK_BackSpace,       kbBack      },
  { XK_Tab,             kbTab       },
  { XK_KP_Enter,        kbEnter     },
  { XK_Return,          kbEnter     },
  { XK_F1,              kbF1        },
  { XK_F2,              kbF2        },
  { XK_F3,              kbF3        },
  { XK_F4,              kbF4        },
  { XK_F5,              kbF5        },
  { XK_F6,              kbF6        },
  { XK_F7,              kbF7        },
  { XK_F8,              kbF8        },
  { XK_F9,              kbF9        },
  { XK_F10,             kbF10       },
  { XK_F11,             kbF11       },
  { XK_F12,             kbF12       },
  { XK_Home,            kbHome      },
  { XK_Up,              kbUp        },
  { XK_Page_Up,         kbPgUp      },
  { XK_Prior,           kbPgUp      },
  { XK_KP_Subtract,     kbGrayMinus },
  { XK_Left,            kbLeft      },
  { XK_Right,           kbRight     },
  { XK_KP_Add,          kbGrayPlus  },
  { XK_End,             kbEnd       },
  { XK_Down,            kbDown      },
  { XK_Page_Down,       kbPgDn      },
  { XK_Next,            kbPgDn      },
  { XK_Insert,          kbIns       },
  { XK_Delete,          kbDel       },
// XXX ASCII dependent
  { XK_KP_Multiply,     '*'         },
  { XK_KP_0,            '0'         },
  { XK_KP_1,            '1'         },
  { XK_KP_2,            '2'         },
  { XK_KP_3,            '3'         },
  { XK_KP_4,            '4'         },
  { XK_KP_5,            '5'         },
  { XK_KP_6,            '6'         },
  { XK_KP_7,            '7'         },
  { XK_KP_8,            '8'         },
  { XK_KP_9,            '9'         },
//
// These key down events must be ignored because they are not
// emitted by the IBM PC BIOS.
// (Their states will be obeyed, however).
//
  { XK_Shift_L,         kbNoKey },
  { XK_Shift_R,         kbNoKey },
  { XK_Control_L,       kbNoKey },
  { XK_Control_R,       kbNoKey },
  { XK_Caps_Lock,       kbNoKey },
  { XK_Shift_Lock,      kbNoKey },
  { XK_Meta_L,          kbNoKey },
  { XK_Alt_L,           kbNoKey },
  { XK_Alt_R,           kbNoKey },
  { XK_Super_L,         kbNoKey },
  { XK_Super_R,         kbNoKey },
  { XK_Hyper_L,         kbNoKey },
  { XK_Hyper_R,         kbNoKey }
};

//
// Translation table for shift-key modified keypresses.
//
static const KeyboardXlat shiftXlatSeed[] = {
  { kbIns,   kbShiftIns },
  { kbDel,   kbShiftDel },
  { kbTab,   kbShiftTab },
  { kbF1,    kbShiftF1  },
  { kbF2,    kbShiftF2  },
  { kbF3,    kbShiftF3  },
  { kbF4,    kbShiftF4  },
  { kbF5,    kbShiftF5  },
  { kbF6,    kbShiftF6  },
  { kbF7,    kbShiftF7  },
  { kbF8,    kbShiftF8  },
  { kbF9,    kbShiftF9  },
  { kbF10,   kbShiftF10 },
  { kbF11,   kbShiftF11 },
  { kbF12,   kbShiftF12 }
};

//
// Translation table for control-key modified keypresses.
//
static const KeyboardXlat ctrlXlatSeed[] = {
// XXX - These entries will be incorrect on non-ASCII systems
  { 'a',      kbCtrlA },
  { 'b',      kbCtrlB },
  { 'c',      kbCtrlC },
  { 'd',      kbCtrlD },
  { 'e',      kbCtrlE },
  { 'f',      kbCtrlF },
  { 'g',      kbCtrlG },
  { 'h',      kbCtrlH },
  { 'i',      kbCtrlI },
  { 'j',      kbCtrlJ },
  { 'k',      kbCtrlK },
  { 'l',      kbCtrlL },
  { 'm',      kbCtrlM },
  { 'n',      kbCtrlN },
  { 'o',      kbCtrlO },
  { 'p',      kbCtrlP },
  { 'q',      kbCtrlQ },
  { 'r',      kbCtrlR },
  { 's',      kbCtrlS },
  { 't',      kbCtrlT },
  { 'u',      kbCtrlU },
  { 'v',      kbCtrlV },
  { 'w',      kbCtrlW },
  { 'x',      kbCtrlX },
  { 'y',      kbCtrlY },
  { 'z',      kbCtrlZ },
//
  { kbIns,    kbCtrlIns     },
  { kbDel,    kbCtrlDel     },
  { kbBack,   kbCtrlBack    },
  { kbEnter,  kbCtrlEnter   },
  { kbF1,     kbCtrlF1      },
  { kbF2,     kbCtrlF2      },
  { kbF3,     kbCtrlF3      },
  { kbF4,     kbCtrlF4      },
  { kbF5,     kbCtrlF5      },
  { kbF6,     kbCtrlF6      },
  { kbF7,     kbCtrlF7      },
  { kbF8,     kbCtrlF8      },
  { kbF9,     kbCtrlF9      },
  { kbF10,    kbCtrlF10     },
  { kbF11,    kbCtrlF11     },
  { kbF12,    kbCtrlF12     },
  { kbLeft,   kbCtrlLeft    },
  { kbRight,  kbCtrlRight   },
  { kbUp,     kbCtrlUp      },
  { kbDown,   kbCtrlDown    },
  { kbEnd,    kbCtrlEnd     },
  { kbHome,   kbCtrlHome    },
  { kbPgUp,   kbCtrlPgUp    },
  { kbPgDn,   kbCtrlPgDn    },
  { kbTab,    kbCtrlTab     },
};

//
// Translation table for alt-key modified keypresses
//
static const KeyboardXlat altXlatSeed[] = {
// XXX - These entries will be incorrect on non-ASCII systems
  { 'a',      kbAltA           },
  { 'b',      kbAltB           },
  { 'c',      kbAltC           },
  { 'd',      kbAltD           },
  { 'e',      kbAltE           },
  { 'f',      kbAltF           },
  { 'g',      kbAltG           },
  { 'h',      kbAltH           },
  { 'i',      kbAltI           },
  { 'j',      kbAltJ           },
  { 'k',      kbAltK           },
  { 'l',      kbAltL           },
  { 'm',      kbAltM           },
  { 'n',      kbAltN           },
  { 'o',      kbAltO           },
  { 'p',      kbAltP           },
  { 'q',      kbAltQ           },
  { 'r',      kbAltR           },
  { 's',      kbAltS           },
  { 't',      kbAltT           },
  { 'u',      kbAltU           },
  { 'v',      kbAltV           },
  { 'w',      kbAltW           },
  { 'x',      kbAltX           },
  { 'y',      kbAltY           },
  { 'z',      kbAltZ           },
  { ' ',      kbAltSpace       },
  { '0',      kbAlt0           },
  { '1',      kbAlt1           },
  { '2',      kbAlt2           },
  { '3',      kbAlt3           },
  { '4',      kbAlt4           },
  { '5',      kbAlt5           },
  { '6',      kbAlt6           },
  { '7',      kbAlt7           },
  { '8',      kbAlt8           },
  { '9',      kbAlt9           },
  { '-',      kbAltMinus       },
  { '=',      kbAltEqual       },
  { '[',      kbAltOpenBraket  },
  { ']',      kbAltCloseBraket },
  { ';',      kbAltSemicolon   },
  { '\'',     kbAltApostrophe  },
  { '`',      kbAltBackApst    },
  { '\\',     kbAltBackslash   },
  { ',',      kbAltComma       },
  { '.',      kbAltDot         },
  { '/',      kbAltSlash       },
//
  { kbF1,     kbAltF1    },
  { kbF2,     kbAltF2    },
  { kbF3,     kbAltF3    },
  { kbF4,     kbAltF4    },
  { kbF5,     kbAltF5    },
  { kbF6,     kbAltF6    },
  { kbF7,     kbAltF7    },
  { kbF8,     kbAltF8    },
  { kbF9,     kbAltF9    },
  { kbF10,    kbAltF10   },
  { kbF11,    kbAltF11   },
  { kbF12,    kbAltF12   },
  { kbBack,   kbAltBksp  },
  { kbEnter,  kbAltEnter },
  { kbEsc,    kbAltEsc   },
  { kbTab,    kbAltTab   },
  { kbLeft,   kbAltLeft  },
  { kbRight,  kbAltRight },
  { kbUp,     kbAltUp    },
  { kbDown,   kbAltDown  },
  { kbDel,    kbAltDel   },
  { kbEnd,    kbAltEnd   },
  { kbHome,   kbAltHome  },
  { kbIns,    kbAltIns   },
  { kbPgUp,   kbAltPgUp  },
  { kbPgDn,   kbAltPgDn  }
};

/*
 * Keyboard translation lookup table routines.
 *
 * These structures and functions implement a simple lookup table
 * using a hashing scheme.
 */

struct KeyBucket {
  int num_entries;
  KeyboardXlat *entries;
};
typedef struct KeyBucket KeyBucket;
  
struct KeyboardXlatHash {
  int num_buckets;
  int hash_key;
  KeyBucket *buckets;
};
typedef struct KeyboardXlatHash KeyboardXlatHash;

//
// Create a new hash-based keyboard key lookup table.
// 
static KeyboardXlatHash *
KeyboardXlatHash_new(int buckets, int hash_key)
{
  int i;
  KeyboardXlatHash *h;

  if (buckets <= 0)
    error("Dumb bucket count.");

  h = (KeyboardXlatHash *) malloc(sizeof(KeyboardXlatHash));

  h->num_buckets = buckets;
  h->hash_key = hash_key;

  h->buckets = (KeyBucket *) malloc(sizeof(KeyBucket) * h->num_buckets);

  for (i = 0; i < h->num_buckets; i++) {
    h->buckets[i].num_entries = 0;
    h->buckets[i].entries = NULL;
  }

  return h;
}

//
// Add an entry to a hash-based keyboard key lookup table.
//
static int
KeyboardXlatHash_add(KeyboardXlatHash *h, int key, int result)
{
  int i;

  i = (key * h->hash_key) % h->num_buckets;

  h->buckets[i].num_entries++;
  h->buckets[i].entries = (KeyboardXlat *) realloc(h->buckets[i].entries,
    h->buckets[i].num_entries * sizeof(KeyboardXlat));

  h->buckets[i].entries[h->buckets[i].num_entries-1].in_code = key;
  h->buckets[i].entries[h->buckets[i].num_entries-1].out_code = result;
}

static void
KeyboardXlatHash_free(KeyboardXlatHash *h)
{
  int i, j;

  for (i = 0; i < h->num_buckets; i++)
    free(h->buckets[i].entries);

  free(h->buckets);
  free(h);
}

static int
KeyboardXlatHash_lookup(KeyboardXlatHash *h, int in, int *out)
{
  int i, j;

  i = (in * h->hash_key) % h->num_buckets;

  for (j = 0; j < h->buckets[i].num_entries; j++) {
    if (h->buckets[i].entries[j].in_code == in) {
      *out = h->buckets[i].entries[j].out_code;
      return 1;
    }
  }

  return 0;
}

static inline int
char2scancode(uchar code)
{
    uchar ncode;

    static const uchar k2s[] =
      " !\"#$%&'()*+,-./0123456789:;<=>?@[\\]^_`abcdefghij"
      "klmnopqrstuvwxyz{|}~";

    static const uchar scv[sizeof(k2s)-1] =
    {
      0x39, 0x02, 0x28, 0x04, 0x05, 0x06, 0x08, 0x28,
      0x0A, 0x0B, 0x09, 0x0D, 0x33, 0x0C, 0x34, 0x35, 0x0B, 0x02, 0x03, 0x04,
      0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x27, 0x27, 0x33, 0x0D, 0x34, 0x35,
      0x03, 0x1A, 0x2B, 0x1B, 0x07, 0x0C, 0x29, 0x1E, 0x30, 0x2E, 0x20, 0x12,
      0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18, 0x19, 0x10,
      0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x1A, 0x2B, 0x1B,
      0x29
    };

    ncode = (uchar)tolower(code);

    unsigned id = (uchar *)memchr(k2s, ncode, sizeof(scv)) - k2s;
    if(id >= sizeof(scv)) return(code);

    return(((unsigned)scv[id]) << 8 | code);
}

static KeyboardXlatHash *plainKeyTable;
static KeyboardXlatHash *shiftKeyTable;
static KeyboardXlatHash *ctrlKeyTable;
static KeyboardXlatHash *altKeyTable;

static KeyboardXlatHash *
createKeyboardXlatHash(int hash_key, int num_buckets, const KeyboardXlat *seed,
		       int seed_count)
{
  KeyboardXlatHash *h;
  int i;

  h = KeyboardXlatHash_new(hash_key, num_buckets);

  for (i = 0; i < seed_count; i++)
    KeyboardXlatHash_add(h, seed[i].in_code, seed[i].out_code);

  return h;
}
  
static void
setupKeyTables()
{
  int i;

  plainKeyTable = createKeyboardXlatHash(17, 16, plainXlatSeed,
    qnumber(plainXlatSeed));

  shiftKeyTable = createKeyboardXlatHash(17, 16, shiftXlatSeed,
    qnumber(shiftXlatSeed));

  ctrlKeyTable = createKeyboardXlatHash(17, 64, ctrlXlatSeed,
    qnumber(ctrlXlatSeed));

  altKeyTable = createKeyboardXlatHash(17, 64, altXlatSeed,
    qnumber(altXlatSeed));
}

//////////////////////////////////////////////////////////////////////////

/*
 * Reads a key from the keyboard.
 */
static void
get_key_mouse_event(TEvent &event)
{
  int num_events;
  int button;

  event.what = evNothing;

  /*
   * suspend until there is a signal or some data in file
   * descriptors
   *
   * if event_delay is zero, then do not suspend
   */
  if ( XEventsQueued(x_disp, QueuedAfterFlush) == 0 &&
       TProgram::event_delay != 0)
  {
    fd_set fdSetRead;

    FD_ZERO(&fdSetRead);
    FD_SET(x_disp_fd, &fdSetRead);

    select(x_disp_fd + 1, &fdSetRead, NULL, NULL, NULL);
  }

  num_events = XEventsQueued(x_disp, QueuedAfterFlush);

  /*
   * Read all of the X events in the X queue until we encounter one we
   * can deliver to the application, or run out of events.
   */
  for (;num_events > 0; num_events--) {
    XNextEvent(x_disp, &xEvent);
    switch (xEvent.type) {
    case Expose:
      /*
       * X Expose event.
       *
       * X sends an Expose event whenever part (or all) of a window needs to be
       * redrawn.
       */
      handleXExpose(&xEvent);
      break;
    case ButtonPress:
      /*
       * X ButtonPress event.
       *
       * X sends a ButtonPress event whenever the user presses a button on the
       * pointing device (mouse).
       */
      keyboardModifierState = translateXModifierState(xEvent.xbutton.state);

      button = translateXButton(xEvent.xbutton.button);

      if (button == 0)
        // Ignore this button, TVision doesn't support it.
        break;

      event.what = evMouseDown;
      event.mouse.buttons = msev.mouse.buttons | button;
      event.mouse.where.x = xEvent.xbutton.x / fontWidth;
      event.mouse.where.y = xEvent.xbutton.y / fontHeight;
      event.mouse.controlKeyState = keyboardModifierState;

      if (button == last_release_button &&
          event.mouse.where == msev.mouse.where &&
          last_release_time != 0 &&
          (xEvent.xbutton.time - last_release_time) <
          TICKS_TO_MS(TEventQueue::doubleDelay)) {
        event.mouse.eventFlags = meDoubleClick;
        last_release_time = 0;
      } else {
        event.mouse.eventFlags = 0;
      }

      // Save the event parameters away.
      msev = event;

      // Start the autoclick timer.
      msAutoTimer.start(DELAY_AUTOCLICK_FIRST);

      return;
    case ButtonRelease:
      keyboardModifierState = translateXModifierState(xEvent.xbutton.state);

      button = translateXButton(xEvent.xbutton.button);
      if (button == 0)
        break;

      event.what = evMouseUp;
      event.mouse.buttons = msev.mouse.buttons & ~button;
      event.mouse.where.x = xEvent.xbutton.x / fontWidth;
      event.mouse.where.y = xEvent.xbutton.y / fontHeight;
      event.mouse.eventFlags = 0;
      event.mouse.controlKeyState = keyboardModifierState;

      last_release_time = xEvent.xbutton.time;
      last_release_button = button;

      msev = event;

      msAutoTimer.stop();

      return;
    case MotionNotify:
      int x, y;

      // X reports mouse movements at every pixel position.  TVision only
      // cares if the mouse changes character positions.  Calculate the
      // character position for the event and see if it has changed from the
      // last event.
      x = xEvent.xbutton.x / fontWidth;
      y = xEvent.xbutton.y / fontHeight;
      keyboardModifierState = translateXModifierState(xEvent.xkey.state);
      if (msev.mouse.where.x != x || msev.mouse.where.y != y) {
        event.what = evMouseMove;
        event.mouse.buttons = msev.mouse.buttons;
        event.mouse.where.x = x;
        event.mouse.where.y = y;
        event.mouse.eventFlags = 0;
        event.mouse.controlKeyState = keyboardModifierState;
        msev = event;
        return;
      }
      break;
    case KeyPress:
      KeySym xcode;
      int code;

      keyboardModifierState = translateXModifierState(xEvent.xkey.state);

      XLookupString(&xEvent.xkey, NULL, 0, &xcode, NULL);
      if (xcode == NoSymbol)
        break;

      if (KeyboardXlatHash_lookup(plainKeyTable, xcode, &code) == 0) {
        //
        // Most key symbols in X are direct ASCII equivalents.  Since we didn't
        // find a plain mapping from X to TVision, just use the X key symbol
        // directly, as it is probably plain ASCII.
        //
        if (xcode < 0x100) {
          //
          // The character is a plain ASCII or extended character.
          // No translation needed.
          //
          code = xcode;
        } else {
          //
          // The character is outside the ASCII or extended set.  It slipped
          // through our translation routines and thus, is probably incorrectly
          // translated.  Sending it up to the TVision routines as is would be
          // an error.
          //
          break;
        }
      } else if (code == kbNoKey)
        //
        // This key is to be ignored.
        //
        break;

      if (keyboardModifierState & kbShift)
        KeyboardXlatHash_lookup(shiftKeyTable, code, &code);
      if (keyboardModifierState & kbCtrlShift)
        KeyboardXlatHash_lookup(ctrlKeyTable, code, &code);
      if (keyboardModifierState & kbAltShift)
        KeyboardXlatHash_lookup(altKeyTable, code, &code);

      //
      // If the character is still plain ASCII after all this translation,
      // then it needs an additional translation to make it a fully formed
      // IBM PC scan code.
      //
      if (code < 0x100)
        code = char2scancode(code);

      event.what = evKeyDown;
      event.keyDown.controlKeyState = keyboardModifierState;
      event.keyDown.keyCode = code;
      return;
    case KeyRelease:
      keyboardModifierState = translateXModifierState(xEvent.xkey.state);
      break;
    case MappingNotify:
      XRefreshKeyboardMapping(&xEvent.xmapping);
      break;
#ifdef ADVANCED_CLIPBOARD
    case SelectionClear:
      //
      // Another application has posted something to the clipboard.
      // We are no longer the clipboard source.
      // There's really nothing to do.
      //
      break;
    case SelectionRequest: {
      //
      // We're in charge of the clipboard and another application has requested
      // its contents.
      //
      XEvent reply;
      
      LOG("Got selection request.");
      
      //
      // We must have something on the internal clipboard and the requestor
      // must ask for the data as a string.
      //
      if (clipboardSize > 0 && xEvent.xselectionrequest.target == XA_STRING) {
        //
        // The requestor has asked for the data as a string.
        // Send it the data by attaching it to the specified property on the
        // specified window.
        //
        XChangeProperty(
          x_disp,
          xEvent.xselectionrequest.requestor,
          xEvent.xselectionrequest.property,
          XA_STRING,
          8, // Format.  8 = 8-bit character array
          PropModeReplace,
          (unsigned char *) clipboardBuffer,
          clipboardSize
        );
        
        //
        // Set up the reply to indicate that the transfer succeeded.
        //
        reply.xselection.property = xEvent.xselectionrequest.property;
      } else {
        //
        // Couldn't satisfy the request.
        // Set up the reply to indicate that the transfer failed.
        //
        reply.xselection.property = None;
      }
      //
      // Notify the requestor that the transfer has completed.
      //
      reply.type = SelectionNotify;
      reply.xselection.display = x_disp;
      reply.xselection.requestor = xEvent.xselectionrequest.requestor;
      reply.xselection.selection = xEvent.xselectionrequest.selection;
      reply.xselection.target = xEvent.xselectionrequest.target;
      reply.xselection.time = xEvent.xselectionrequest.time;
      XSendEvent(x_disp, xEvent.xselectionrequest.requestor, False, 0, &reply);
      }
      break;
#endif // ADVANCED_CLIPBOARD
    case ConfigureNotify:
      int width, height;
      
      width = range(xEvent.xconfigure.width / fontWidth,4, maxViewWidth);
      height = range(xEvent.xconfigure.height / fontHeight, 4, 100);
      
      if (TScreen::screenWidth == width && TScreen::screenHeight == height)
        //
        // Screen size hasn't changed.  Ignore.
        //
        break;
        
      TScreen::screenWidth = width;
      TScreen::screenHeight = height;
      delete[] TScreen::screenBuffer;
      delete[] currentScreenBuffer;
      delete[] currentUpdateBlocks;
      delete[] currentChunkBuf;
      int totalSize = TScreen::screenWidth * TScreen::screenHeight;
      TScreen::screenBuffer = new ushort[totalSize];
      currentScreenBuffer = new ushort[totalSize];
      currentUpdateBlocks = new UpdateBlock[TScreen::screenWidth];
      currentChunkBuf = new char[TScreen::screenWidth];
      memset(currentScreenBuffer, sizeof(short) * totalSize, 0);
      LOG("screen resized to %dx%d", TScreen::screenWidth,
        TScreen::screenHeight);
      TScreen::drawCursor(0); /* hide the cursor */
      event.message.command = cmSysResize;
      event.what = evCommand;
      return;
    }
 }

  /* see if there is data available */
}

static void selectPalette()
{
  TScreen::screenMode = TScreen::smCO80;
}

static inline int
max(int a, int b)
{
	return (a > b) ? a : b;
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
  writeRow(0, b.getBufAddr(), TScreen::screenWidth, false);

	for (;;) {
		char xlat[20];
		int char_count;
		KeySym key;
		XEvent theEvent;

		XNextEvent(x_disp, &theEvent);

		switch (theEvent.type) {
		case KeyPress:
			char_count = XLookupString(&theEvent.xkey, xlat,
				sizeof(xlat), &key, NULL);
			if (char_count > 0) 
				 return toupper(xlat[0]) == 'Y';
			break;
		case Expose:
			handleXExpose(&theEvent);
			break;
		case MappingNotify:
			XRefreshKeyboardMapping(&theEvent.xmapping);
			break;
		default:
			break;
		}
	}

	return 0;
}

static void freeResources()
{
  delete[] TScreen::screenBuffer;
  delete[] currentScreenBuffer;
  delete[] currentUpdateBlocks;
  delete[] currentChunkBuf;
#ifdef ADVANCED_CLIPBOARD
  free(clipboardBuffer);
#endif
  
	KeyboardXlatHash_free(plainKeyTable);
	KeyboardXlatHash_free(shiftKeyTable);
	KeyboardXlatHash_free(ctrlKeyTable);
	KeyboardXlatHash_free(altKeyTable);

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
  }
}

/*
 * INTERNAL
 *
 * Load X resource overrides from the specified file.
 */
static void
loadXResources(const char *filename)
{
  XrmDatabase xdb;
  XrmValue value;
  char *type;
  int res, i;

  XrmInitialize();

  xdb = XrmGetFileDatabase(filename);
  if (xdb == NULL) {
    LOG("Couldn't open Xdefaults file '%s'.", filename);
    return;
  }

  for (i = 0; i < TvisionResources_count; i++) {
    res = XrmGetResource(xdb, TvisionResources[i].name,
                         TvisionResources[i].cls, &type, &value);
    if (res != 0)
      *TvisionResources[i].value = value.addr;
  }
}

static void
setupGCs(Display *disp, Window win, Font font)
{
	int i, j;
	XGCValues gcv;
	XColor dummy, foreground[16], background[16];
	Colormap colormap;

	colormap = DefaultColormap(disp,  DefaultScreen(disp));

	/*
	 * Allocate the sixteen background colors.
	 */	
	for (i = 0; i < 16; i++) {
		if (XAllocNamedColor(x_disp, colormap, bgColor[i],
			&background[i], &dummy) == 0)
			error("Couldn't allocate color %s", bgColor[i]);
	}

	/*
	 * Allocate the sixteen foreground colors.
	 */
	for (i = 0; i < 16; i++) {
		if (XAllocNamedColor(x_disp, colormap, fgColor[i],
			&foreground[i], &dummy) == 0)
			error("Couldn't allocate color %s", fgColor[i]);
	}

	/*
	 * Create a unique graphics context for each combination of
	 * background and foreground colors.
	 */
	characterGC = new GC[256];

	gcv.font = font;

	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			gcv.foreground = foreground[j].pixel;
			gcv.background = background[i].pixel;
			characterGC[i * 16 + j] = XCreateGC(
				disp,
				win,
				GCForeground | GCBackground | GCFont,
				&gcv
			);
		}
	}

	/*
	 * Setup a GC for drawing the text cursor.
	 */
	gcv.function = GXxor;
	gcv.plane_mask = AllPlanes;
	gcv.foreground = WhitePixel(x_disp, DefaultScreen(x_disp));
	cursorGC = XCreateGC(disp, win,
                             GCFunction|GCForeground|GCPlaneMask, &gcv);
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

	p = getenv("DISPLAY");
	if (p == NULL)
		error("No DISPLAY defined.\n");

        /*
	 * Connect to X server.
	 */
	x_disp = XOpenDisplay(p);
	if (x_disp == NULL)
		error("Couldn't open display '%s'.\n", p);

#ifdef ADVANCED_CLIPBOARD
  /*
   * "Intern" an atom that we can use as a target identifier when
   * asking X for the clipboard contents.
   */
  clipboardAtom = XInternAtom(x_disp, "TVISION_CLIPBOARD", False);
  if (clipboardAtom == None)
    error("Couldn't intern clipboard atom.\n");
  XA_CLIPBOARD = XInternAtom(x_disp, "CLIPBOARD", False);
  if (clipboardAtom == None)
    error("Couldn't intern clipboard selection atom.\n");  
#endif

	/*
	 * Read in X resource overrides.
	 */
	p = getenv("HOME");
	if (p != NULL) {
		char rdbFilename[PATH_MAX];
		qsnprintf(rdbFilename, sizeof(rdbFilename), "%s/.Xdefaults", p);
		loadXResources(rdbFilename);
	}
		
	/*
	 * Load the screen font.
	 */
	XFontStruct *theFont = XLoadQueryFont(x_disp, fontName);
	if (theFont == NULL)
		error("Can't load font '%s'.", fontName);

	/*
	 * Log a warning if the font isn't fixed-width.
	 */
	if (theFont->min_bounds.width != theFont->max_bounds.width)
		warning("Font '%s' is not fixed-width.", fontName);

	fontAscent = theFont->ascent;
	fontDescent = theFont->descent;
	fontHeight = fontAscent + fontDescent;
	fontWidth = theFont->max_bounds.width;
	Font fid = theFont->fid;

	/*
	 * We're done with the font info.
	 */
	XFreeFontInfo(NULL, theFont, 0);

        /*
	 * Negotiate window size with user settings and our defaults.
	 */
	XSizeHints *sh;

	if ((sh = XAllocSizeHints()) == NULL)
		error("alloc");

	sh->min_width = 4 * fontWidth;
	sh->max_width = maxViewWidth * fontWidth;
	sh->min_height = 4 * fontHeight;
	sh->max_height = 80 * fontHeight;
	sh->width_inc = fontWidth;
	sh->height_inc = fontHeight;
	sh->flags = (PSize | PMinSize | PResizeInc);

	char default_geometry[80];

	qsnprintf(default_geometry, sizeof(default_geometry),
		"%dx%d+%d+%d", DEFAULT_GEOMETRY_WIDTH,
		DEFAULT_GEOMETRY_HEIGHT, 0, 0);

	int geometryFlags = XGeometry(x_disp, DefaultScreen(x_disp),
		geometry, default_geometry, BORDER_WIDTH, fontWidth,
		fontHeight, 0, 0, &sh->x, &sh->y, &sh->width, &sh->height);

	if (geometryFlags & (XValue | YValue))
		sh->flags |= USPosition;
	if (geometryFlags & (WidthValue | HeightValue))
		sh->flags |= USSize;

	sh->width *= fontWidth;
	sh->height *= fontHeight;

	/*
	 * Create our window.
	 */
	x_win = XCreateSimpleWindow(x_disp, DefaultRootWindow(x_disp),
		sh->x, sh->y, sh->width, sh->height, BORDER_WIDTH,
		BlackPixel(x_disp, DefaultScreen(x_disp)),
		WhitePixel(x_disp, DefaultScreen(x_disp)));

	/*
	 * Setup class hint for the window manager.
	 */
	XClassHint *clh;

	if ((clh = XAllocClassHint()) == NULL)
		error("alloc");

	clh->res_name = APP_NAME;
	clh->res_class = APP_CLASS;

	/*
	 * Setup hints to the window manager.
	 */
	XWMHints *wmh;

	if ((wmh = XAllocWMHints()) == NULL)
		error("alloc");

	wmh->flags = (InputHint | StateHint);
	wmh->input = True;
	wmh->initial_state = NormalState;

	XTextProperty WName, IName;
	char *app_name = APP_NAME;

	if (XStringListToTextProperty(&app_name, 1, &WName) == 0)
		error("Error creating text property");
	if (XStringListToTextProperty(&app_name, 1, &IName) == 0)
		error("Error creating text property");

	/*
	 * Send all of these hints to the X server.
	 */
	XSetWMProperties(x_disp, x_win, &WName, &IName, NULL, 0,
		sh, wmh, clh);

	/*
	 * Set the window's bit gravity so that resize events keep the
	 * upper left corner contents.
	 */
	XSetWindowAttributes wa;

	wa.bit_gravity = NorthWestGravity;
	wa.backing_store = WhenMapped;
	wa.event_mask = ExposureMask|KeyPressMask|ButtonPressMask|
	                ButtonReleaseMask|ButtonMotionMask|
	                KeymapStateMask|ExposureMask|
		        StructureNotifyMask;

	XChangeWindowAttributes(x_disp, x_win,
		CWBitGravity|CWBackingStore|CWEventMask, &wa);

  screenWidth = sh->width / fontWidth;
  screenHeight = sh->height / fontHeight;

	XFree(wmh);
	XFree(clh);
	XFree(sh);

	/*
	 * Setup the 256 graphics contexts we require to draw
	 * color text.
	 */
	setupGCs(x_disp, x_win, fid);

	/*
	 * Cause the window to be displayed.
	 */
	XMapWindow(x_disp, x_win);

  LOG("screen size is %dx%d", screenWidth, screenHeight);
  TScreen::screenBuffer = new ushort[screenWidth * screenHeight];
  currentScreenBuffer = new ushort[screenWidth * screenHeight];
  currentUpdateBlocks = new UpdateBlock[screenWidth];
  currentChunkBuf = new char[screenWidth];
#ifdef ADVANCED_CLIPBOARD
  clipboardBuffer = NULL;
  clipboardSize = 0;
#endif
  
  /* internal stuff */

	expose_region = XCreateRegion();

	/* Setup keyboard translations */

	setupKeyTables();

	/* setup file descriptors */

	x_disp_fd = ConnectionNumber(x_disp);

  curX = curY = 0;
  cursor_displayed = true;
  currentTime = doRepaint = evLength = 0;
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
  doRepaint++;
}

void TScreen::suspend()
{
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
    get_key_mouse_event(*this);
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
    get_key_mouse_event(event);
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
  XBell(x_disp, 0);
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
  if (cursor_displayed == show)
    // Cursor is already in desired state.
    return;

  if (show) {
    // Turn on the cursor.
    cursor_displayed = 1;
    drawTextCursor();
  } else {
    // Turn off the cursor.
    cursor_displayed = 0;
    eraseTextCursor();
  }
}

/*
 * Moves the cursor to another place.
 */

void TScreen::moveCursor(int x, int y)
{
  if (cursor_displayed)
    eraseTextCursor();
  curX = x;
  curY = y;
  if (cursor_displayed)
    drawTextCursor();
}

/*
 * Draws a line of text on the screen.
 */
void TScreen::writeRow(int dst, ushort *src, int len)
{
  ::writeRow(dst, src, len, false);
}

/*
 * Updates the X display with new text.
 */
static void
writeRow(int dst, ushort *src, int len, bool isExpose)
{
  int x, y, i, j, endblock;
  char *chunk_p;
  ushort *src_p, *cache_p;

  //
  // If this isn't a forced update, it may be possible to skip this update
  // completely if it is redundant.
  //
  if (!isExpose) {
    //
    // This is not a forced update, but rather a signal from further inside
    // the TVision library that something may have changed in this row
    // on the screen.
    //
    // TVision sends many unecessary redraws, so we can take this opportunity
    // to skip the operation if the current screen buffer already contains
    // the characters being drawn.
    //
    if (memcmp(src, &currentScreenBuffer[dst], len * sizeof(ushort)) == 0)
      return;
  }

  //
  // Gather up a list of update blocks for the current row, taking into
  // account what is already on the screen.
  //
  endblock = 0;
  chunk_p = currentChunkBuf;
  cache_p = &currentScreenBuffer[dst];
  src_p = src;
  currentUpdateBlocks[endblock].state = UpdateBlock::UPD_NONE;
  
  for (i = len; i > 0; src_p++, cache_p++, chunk_p++, i--) {
    int color = (*src_p & 0xff00) >> 8; // color code
    int code = *src_p & 0xff;           // character code
    int ccolor = (*cache_p & 0xff00) >> 8; // cached color code
    int ccode = *cache_p & 0xff;           // cached character code

    //
    // Determine if a new update block needs to be created.
    //
    if (color == ccolor && code == ccode && !isExpose) {
      //
      // Current character is already on the screen.
      //
      if (currentUpdateBlocks[endblock].state != UpdateBlock::UPD_REDUNDANT ||
          currentUpdateBlocks[endblock].color != color) {
        //
        // Start a new redundant block.
        //
        if (currentUpdateBlocks[endblock].state != UpdateBlock::UPD_NONE)
          endblock++;
        currentUpdateBlocks[endblock].state = UpdateBlock::UPD_REDUNDANT;
        currentUpdateBlocks[endblock].color = color;
        currentUpdateBlocks[endblock].characters = chunk_p;
        currentUpdateBlocks[endblock].len = 0;
      }
    } else {
      //
      // Current character is not already on the screen.
      //
      if (currentUpdateBlocks[endblock].state != UpdateBlock::UPD_UPDATE ||
          currentUpdateBlocks[endblock].color != color) {
        //
        // Start a new update block.
        //
        if (currentUpdateBlocks[endblock].state != UpdateBlock::UPD_NONE)
          endblock++;
        currentUpdateBlocks[endblock].state = UpdateBlock::UPD_UPDATE;
        currentUpdateBlocks[endblock].color = color;
        currentUpdateBlocks[endblock].characters = chunk_p;
        currentUpdateBlocks[endblock].len = 0;
      }
    }
    
    //
    // Add this character to the current block.
    //
    currentUpdateBlocks[endblock].len++;
    *chunk_p = code;
  }

  if (currentUpdateBlocks[0].state == UpdateBlock::UPD_NONE) {
    //
    // There's nothing to do.
    //
    return;
  }
  
  //
  // Now collapse update blocks into an efficient form.
  //
  // This algorithm takes into account the network cost of an
  // XDrawImageString request (known as an ImageText8 request in the
  // X11 protocol specification).
  //
  // If two draw image requests can be collapsed into a single request
  // of smaller size, it will do so.
  //
  
  // The size of an ImageText8 message header.
  //
  //  ImageText8
  //             drawable: DRAWABLE
  //             gc: GCONTEXT
  //             x, y: INT16
  //             string: STRING8
  //
  static const int IMAGETEXT8_SIZE = 1 + 4 + 4 + 2 + 2 + 4; 

  //
  // Compute the current screen x and y position.
  //
  x = dst % TScreen::screenWidth;
  y = dst / TScreen::screenWidth;

  for (i = 0; i <= endblock; i++) {
    //
    // Skip any leading redundant blocks.
    //
    if (currentUpdateBlocks[i].state == UpdateBlock::UPD_REDUNDANT) {
#ifdef DEBUG_OPTIMIZATION
      LOG("Skipping %d redundant at (%d, %d) color %02x.",
             currentUpdateBlocks[i].len, y, x, currentUpdateBlocks[i].color);
#endif
      x += currentUpdateBlocks[i].len;
      continue;
    }
    
    //
    // We've got an update block.  See if it can be expanded.
    //
    while ((i + 2 <= endblock) &&
        currentUpdateBlocks[i + 1].state == UpdateBlock::UPD_REDUNDANT &&
        currentUpdateBlocks[i + 1].color == currentUpdateBlocks[i].color &&
        currentUpdateBlocks[i + 2].state == UpdateBlock::UPD_UPDATE &&
        currentUpdateBlocks[i + 2].color == currentUpdateBlocks[i].color) {
      //
      // We've found a possible group to collapse.
      // Is it worth it?
      //
      if (currentUpdateBlocks[i + 1].len <= IMAGETEXT8_SIZE) {
        //
        // The collapse is worth it.
        // Do it.
        //
#ifdef DEBUG_OPTIMIZATION
        LOG("Collapsing %d:%d:%d color %02x run.",
          currentUpdateBlocks[i].len,
          currentUpdateBlocks[i + 1].len,
          currentUpdateBlocks[i + 2].len,
          currentUpdateBlocks[i].color);
#endif
        currentUpdateBlocks[i].len += currentUpdateBlocks[i + 1].len;
        currentUpdateBlocks[i].len += currentUpdateBlocks[i + 2].len;
        for (j = i + 1; (j + 2) <= endblock; j++)
          currentUpdateBlocks[j] = currentUpdateBlocks[j + 2];
        endblock -= 2;
        
        //
        // See if we can collapse some more.
        //
        continue;
      } else
        break;
    }

    //
    // Draw the current block.
    //
    XDrawImageString(
      x_disp,
      x_win,
      characterGC[currentUpdateBlocks[i].color],
      x * fontWidth,
      y * fontHeight + fontAscent,
      currentUpdateBlocks[i].characters,
      currentUpdateBlocks[i].len
    );
    
#ifdef DEBUG_OPTIMIZATION
    XFlush(x_disp);

    LOG("Wrote %d characters at (%d, %d) color %02x.",
      currentUpdateBlocks[i].len, y, x, currentUpdateBlocks[i].color);
#endif

    //
    // If the cursor is on and we've just drawn over it, redisplay it.
    //
    if (cursor_displayed &&
        curY == y &&
        curX >= x && curX < (x + currentUpdateBlocks[i].len))
      drawTextCursor();

    x += currentUpdateBlocks[i].len;
  }

  
  if (!isExpose)
    //
    // Update the current screen buffer cache.
    //
    memcpy(&currentScreenBuffer[dst], src, len * sizeof(ushort));
}

/*
 * Expands a path into its directory and file components.
 */

void expandPath(const char *path,
                char *dir, size_t dirsize,
                char *file, size_t filesize)
{
    /* the path is in the form /dir1/dir2/file ? */
    const char *tag = strrchr(path, '/');
    size_t n;

    if(tag) {
      n = ++tag - path;
      if ( ssize_t(dirsize) > 0 )
      {
        if ( n >= dirsize )
          n = dirsize-1;
        memcpy(dir, path, n);
      }
    } else {  // only file name
      n = 0;
      tag = path;
    }
    if ( ssize_t(dirsize) > 0 )
      dir[n] = '\0';
    qstrncpy(file, tag, filesize);
}
void fexpand(char *path, size_t pathsize)
{
	char resolved[PATH_MAX];

	if ( realpath(path, resolved) || errno == ENOENT )
		qstrncpy(path, resolved, pathsize);
}

void TEventQueue::mouseInt() { /* no mouse... */ }

void THWMouse::resume()
{
  LOG("RESUME MOUSE");
}

void THWMouse::suspend()
{
  LOG("SUSPEND MOUSE");
}

void TV_CDECL THWMouse::show()
{
  LOG("SHOW MOUSE");
}

void TV_CDECL THWMouse::hide()
{
  LOG("HIDE MOUSE");
}

bool TProgram::switch_screen(int to_user)
{
  //
  // The X window is separate from any child application's
  // I/O.  Therefore, nothing needs to be done in this implementation.
  //
  return true;
}

void TProgram::at_child_exec(bool before)
{
  //
  // Nothing needs to be done.
  //
}

//
// Get the contents of the system clipboard.
//
char *
TThreads::clipboard_get(size_t &sz, bool line)
{
  char *answer = NULL, *data, *clip;
  bool xfree = false;
  size_t clsz;
#ifdef ADVANCED_CLIPBOARD
  Window currentOwner;
  int r;
  
  //
  // See who is the current selection owner.
  //
  currentOwner = XGetSelectionOwner(x_disp, XA_CLIPBOARD);
  
  if (currentOwner == None) {
    //
    // No one has posted anything to the clipboard.
    //
    goto fail;
  } else if (currentOwner == x_win) {
    //
    // We're the current selection owner.
    // Bypass the normal X11 query protocol and use what is on our
    // internal clipboard.
    //
    clsz = clipboardSize;
    data = clipboardBuffer;
    LOG("Using internal clipboard data, size %d.", clsz);
  } else {
    //
    // Someone else owns the current selection.
    // Ask them to convert the current selection to an XA_STRING.
    //    
    r = XConvertSelection(x_disp, XA_CLIPBOARD, XA_STRING, clipboardAtom,
                          x_win, CurrentTime);
    if (r == BadAtom || r == BadWindow) {
      LOG("ConvertSelection failed (%d).", r);
      goto fail;
    }
      
    
    //
    // Await a response from the selection owner, notifying us that the
    // selection transfer has been completed.
    //
    XEvent responseEvent;
      
    //
    // Pull the next SelectionNotify event out of the event queue, waiting
    // for one to arrive if none are currently available, and leaving all
    // other queued events undisturbed.
    //
    r = XIfEvent(x_disp, &responseEvent, findSelectionNotifyEvent, NULL);
    if (r != Success) {
      LOG("Wait for selectionNotify failed.");
      goto fail;
    }
    
    //
    // The filter function should have ensured that the only event we receive
    // is a SelectionNotify event, but double check just to make sure.
    //
    if (responseEvent.type != SelectionNotify) {
      LOG("Expected selection notify, got %d.", responseEvent.type);
      goto fail;
    }

    //
    // Did the sender indicate a successful transfer?
    //
    if (responseEvent.xselection.property == None) {
      LOG("Responder says transfer failed.");
      goto fail;
    }
    
    //
    // Query our window (which we designated as the receipient of the
    // transfer when we made the selection request) for the property
    // to which we asked the sender to write the results.
    //
    Atom actual_type_atom;
    int actual_format;
    unsigned long bytes_returned, bytes_remaining;
    
    r = XGetWindowProperty(x_disp, x_win, clipboardAtom, 0, 1024 * 1024, False,
                           XA_STRING, &actual_type_atom, &actual_format,
                           &bytes_returned, &bytes_remaining,
                           (unsigned char **)&data);
    if (r != Success) {
      LOG("Couldn't get window property.");
      goto fail;
    }

    if (bytes_remaining > 0)
      //
      // Wow.  There are more bytes in the clipboard than the insane amount
      // that we requested.  The clipboard text will truncated.
      // Log a warning.
      //
      warning("Dropped %d bytes from clipboard transfer.", bytes_remaining);
    
    clsz = bytes_returned;
    xfree = true;
  }
#else 
  int bytes_returned;
  
  data = XFetchBytes(x_disp, &bytes_returned);
  if (data == NULL) {
    warning("XFetchBytes failed.\n");
    goto fail;
  }
  LOG("XFetchBytes ok: %p, %d.", data, bytes_returned);
  xfree = true;
  clsz = bytes_returned;
#endif

  //
  // This code stolen from the Windows NT TVision clipboard code.
  // It appears to filter out unwanted characters.
  //
  if (clsz != 0) {
    //
    // Is there more available than the caller is willing to accept?
    //
    if (sz < clsz)
      //
      // We've got more than the caller wants.  Truncate.
      //
      clsz = sz;
    
    int allocsz = clsz;
    
    //
    // If this is a multi-line retreival, scan the string for newlines not
    // preceeded by a carriage return.  These "UNIX" convention newlines will
    // not be interpreted correctly by TVision unless they are preceeded by
    // carriage returns, so we must insert them ourselves.  We need to know
    // how many we are going to insert so that we can allocate a proper buffer.
    //
    if (line) {
      int newlines, i;
      bool cr;
      
      for (i = newlines = 0, cr = false; i < clsz; i++) {
        if (data[i] == '\r') {
          cr = true;
        } else {
          if (data[i] == '\n')
            if (cr == false)
              newlines++;
          cr = false;
        }
      }
      allocsz += newlines;
    }
    
    //
    // Allocate a buffer to return to the caller.
    //
    answer = (char *)malloc(allocsz);

    //
    // Copy characters into the buffer, trimming unprintables and
    // mapping newline and carriage returns into spaces, if needed.
    //
    if (answer != NULL) {
      char *dst = answer;
      char *src = data;
      bool cr = false;
      
      while (*src != '\0' && clsz > 0) {
        char c = *src++;
        clsz--;
        if (c < ' ' || c == 127) {
          //
          // Character is non-printable.
          //
          if (line && dst == answer)
            //
            // We haven't seen any printables yet, so
            // just skip this character entirely.
            // (This has the effect of eating leading whitespace).
            //
            continue;
          if (line || (c != '\r' && c != '\n'))
            //
            // We've written printable characters already.
            // Convert this weird character to a space.
            //
            c = ' ';
        }
        if (c == '\n' && !line && !cr) {
          //
          // We need to insert a carriage return in front
          // of this newline.
          //
          *dst++ = '\r';
        }
        if (c == '\r')
          cr = true;
        else
          cr = false;
        *dst++ = c;
      }
      if ( line )
        //
        // Trim trailing whitespace.
        //
        for ( ; dst > answer; --dst )
          if ( dst[-1] > ' ' )
            break;
      
      //
      // Is the entire result empty?
      //
      if (dst == answer) {
        free (answer);
        answer = NULL;
      } else {
        *dst = '\0';
        sz = dst - answer;
      }
    }
  }

  if (xfree)
    XFree(data);

  return answer;
  
fail:
  sz = 0;
  return NULL;
}

//
// Put data on the system clipboard.
//
bool
TThreads::clipboard_put(const char *str, size_t from, size_t to)
{
#ifdef ADVANCED_CLIPBOARD
  Time theTime;
  
  //
  // We need to figure out what XEvent caused this program to want to
  // paste something on the clipboard so that we can tell the X server
  // exactly _when_ we want it on the clipboard.
  //
  // We do so by assuming that the last received X event must have caused
  // this behavior and using that event's timestamp.
  //
  switch (xEvent.type) {
  case ButtonPress:
  case ButtonRelease:
    theTime = xEvent.xbutton.time;
    break;
  case KeyPress:
  case KeyRelease:
    theTime = xEvent.xkey.time;
    break;
  default:
    LOG("Can't paste to clipboard because I can't correlate an X event.");
    return false;
  }
  
  //
  // Notify the X server that we want to own the clipboard.
  //
  XSetSelectionOwner(x_disp, XA_CLIPBOARD, x_win, theTime);
  
  //
  // See if we managed to gain ownership.
  //
  if (XGetSelectionOwner(x_disp, XA_CLIPBOARD) != x_win)
    //
    // We failed to acquire the selection.
    //
    return false;
  
  //
  // We've got ownership.
  // Remember the characters that the application wants to send so that we
  // can send them later, when the server asks for them.
  //
  free(clipboardBuffer);
  clipboardSize = to - from;
  clipboardBuffer = (char *) malloc(clipboardSize);
  memcpy(clipboardBuffer, str + from, clipboardSize);
  LOG("Stored %d bytes on internal clipboard.", clipboardSize);
  
  return true;
#else
  int r;
  
  r = XStoreBytes(x_disp, str + from, to - from);
  if (r == Success)
    return true;
  warning("XStoreBytes failed (%d).\n", r);
  return false;
#endif
}

#ifdef ADVANCED_CLIPBOARD
static Bool
findSelectionNotifyEvent(Display *d, XEvent *e, XPointer arg)
{
  if (e->type == SelectionNotify)
    return True;
  return False;
}
#endif // ADVANCED_CLIPBOARD

#endif // __UNIX__
