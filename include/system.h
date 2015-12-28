/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   SYSTEM.H                                                              */
/*                                                                         */
/*   Copyright (c) Borland International 1991                              */
/*   All Rights Reserved.                                                  */
/*                                                                         */
/*   defines the classes THWMouse, TMouse, TEventQueue, TDisplay,          */
/*   TScreen, and TSystemError                                             */
/*                                                                         */
/* ------------------------------------------------------------------------*/

#if !defined( __EVENT_CODES )
#define __EVENT_CODES

/* Event codes */

const int evMouseDown = 0x0001;
const int evMouseUp   = 0x0002;
const int evMouseMove = 0x0004;
const int evMouseAuto = 0x0008;
const int evKeyDown   = 0x0010;
const int evCommand   = 0x0100;
const int evBroadcast = 0x0200;

/* Event masks */

const int evNothing   = 0x0000;
const int evMouse     = 0x000f;
const int evKeyboard  = 0x0010;
const int evMessage   = 0xFF00;

/* Mouse button state masks */

const int mbLeftButton  = 0x01;
const int mbRightButton = 0x02;

/* Mouse event flags */

const int meMouseMoved = 0x01;
const int meDoubleClick = 0x02;

#endif  // __EVENT_CODES

#if defined( Uses_TEvent ) && !defined( __TEvent )
#define __TEvent

/**
 * Stores mouse events.
 *
 * This structure holds the data that characterizes a mouse event: button
 * number, whether double-clicked, and the coordinates of the point where the
 * click was detected.
 * @see TEvent
 * @see TEventQueue
 * @short Information about mouse events
 */
struct MouseEventType
{
    /**
     * This is the position where the event happened.
     */
    TPoint where;
    /**
     * Helps to specify the event.
     *
     * This bitmap variable is set to meDoubleClick if a double-click event
     * happened. If the mouse is simply moved its value is meMouseMoved.
     * Otherwise its value is 0.
     *
     * <pre>
     * Flag          Value Meaning
     *
     * meMouseMoved  0x01  Set if mouse is moved
     * meDoubleClick 0x02  Set if a button was double clicked
     * </pre>
     */
    ulong eventFlags;           // Replacement for doubleClick.
    bool doubleClick(void) const { return (eventFlags & meDoubleClick) != 0; }
    /**
     * This bitmap variable stores the status of the control keys when the
     * event happened. The following values define keyboard states, and can be
     * used when examining the keyboard shift state:
     *
     * <pre>
     * Flag          Value  Meaning
     *
     * kbRightShift  0x0001 Set if the Right Shift key is currently down
     * kbLeftShift   0x0002 Set if the Left Shift key is currently down
     * kbCtrlShift   0x0004 Set if the Ctrl key is currently down
     * kbAltShift    0x0008 Set if the Alt key is currently down
     * kbScrollState 0x0010 Set if the keyboard is in the Scroll Lock state
     * kbNumState    0x0020 Set if the keyboard is in the Num Lock state
     * kbCapsState   0x0040 Set if the keyboard is in the Caps Lock state
     * kbInsState    0x0080 Set if the keyboard is in the Ins Lock state
     * </pre>
     *
     * Its value is 0 if none of these keys was pressed. Warning: this
     * information is not reliable. Its value depends on your operating system
     * and libraries (gpm, ncurses). Usually only a subset of these flags are
     * detected. See file `system.cc' for details.
     */
    ulong controlKeyState;
    /**
     * This variable reports the status of the mouse buttons when the event
     * happened. It's a combination of the following constants:
     *
     * <pre>
     * Flag          Value Meaning
     *
     * mbLeftButton  0x01  Set if left button was pressed
     * mbRightButton 0x02  Set if right button was pressed
     * </pre>
     *
     * These constants are useful when examining the buttons data member. For
     * example:
     *
     * <pre>
     * if ((event.what == @ref evMouseDown) && (event.buttons == mbLeftButton))
     *     doLeftButtonDownAction();
     * </pre>
     *
     * Note: you can swap left and right buttons by setting variable
     * @ref TEventQueue::mouseReverse to True. See the `demo' program for more
     * information.
     */
    uchar buttons;
};

class THWMouse
{

protected:

    THWMouse();
    THWMouse( const THWMouse& ) {};
    ~THWMouse();

    static void TV_CDECL show();
    static void TV_CDECL hide();
#ifdef __OS2__
    static void hide( const TRect& rect );
#endif

    static void setRange( ushort, ushort );
    static void getEvent( MouseEventType& );
    static void registerHandler( unsigned, void (*)() );
    static Boolean present();

    static void suspend();
    static void resume();
    static void inhibit();

protected:

    static uchar buttonCount;

private:

    static Boolean handlerInstalled;
    static Boolean noMouse;

};

inline Boolean THWMouse::present()
{
    return Boolean( buttonCount != 0 );
}

inline void THWMouse::inhibit()
{
    noMouse = True;
}

class TMouse : public THWMouse
{

public:

    TMouse();
    ~TMouse();

    static void show();
    static void hide();
#ifdef __OS2__
    static void hide( const TRect& rect ) { THWMouse::hide( rect ); }
#endif

    static void setRange( ushort, ushort );

    static void getEvent( MouseEventType& );
    static void registerHandler( unsigned, void (*)() );
    static Boolean present();

    static void suspend() { THWMouse::suspend(); }
    static void resume() { THWMouse::resume(); show(); }

};

inline void TMouse::show()
{
    THWMouse::show();
}

inline void TMouse::hide()
{
    THWMouse::hide();
}

inline void TMouse::setRange( ushort rx, ushort ry )
{
    THWMouse::setRange( rx, ry );
}

inline void TMouse::getEvent( MouseEventType& me )
{
    THWMouse::getEvent( me );
}

inline void TMouse::registerHandler( unsigned mask, void (*func)() )
{
    THWMouse::registerHandler( mask, func );
}

inline Boolean TMouse::present()
{
    return THWMouse::present();
}

#pragma pack(push, 1)
struct CharScanType
{
#ifdef __ppc__
    uchar scanCode;
    uchar charCode;
#else
    uchar charCode;
    uchar scanCode;
#endif
};
#pragma pack(pop)

struct KeyDownEvent
{
    union
        {
        ushort keyCode;
        CharScanType charScan;
        };
    /**
     * Stores the status of the control keys when the event happened. The
     * following values define keyboard states, and can be used when
     * examining the keyboard shift state:
     *
     * <pre>
     * Constant      Value  Meaning
     *
     * kbRightShift  0x0001 Set if the Right Shift key is currently down
     * kbLeftShift   0x0002 Set if the Left Shift key is currently down
     * kbCtrlShift   0x0004 Set if the Ctrl key is currently down
     * kbAltShift    0x0008 Set if the Alt key is currently down
     * kbScrollState 0x0010 Set if the keyboard is in the Scroll Lock state
     * kbNumState    0x0020 Set if the keyboard is in the Num Lock state
     * kbCapsState   0x0040 Set if the keyboard is in the Caps Lock state
     * kbInsState    0x0080 Set if the keyboard is in the Ins Lock state
     * </pre>
     *
     * Its value is 0 if none of these keys was pressed. Warning: this
     * information is not reliable. Its value depends on your operating system
     * and libraries (gpm, ncurses). Usually only a subset of these flags are
     * detected. See file `system.cc' for details.
     */
    ulong controlKeyState;
};

struct MessageEvent
{
    ushort command;
    union
        {
        void *infoPtr;
        long infoLong;
        ushort infoWord;
        short infoInt;
        uchar infoByte;
        char infoChar;
        };
};

class TEvent
{
public:
    ushort what;
    union
    {
        MouseEventType mouse;
        KeyDownEvent keyDown;
        MessageEvent message;
    };
    void getMouseEvent();
    void getKeyEvent();                 // with macro handling
    void _getKeyEvent();                // without macro handling
};

#endif  // Uses_TEvent

#if defined( Uses_TEventQueue ) && !defined( __TEventQueue )
#define __TEventQueue

class TEventQueue
{
public:
    TEventQueue();
    ~TEventQueue();

    static void getMouseEvent( TEvent& );
    static void suspend();
    static void resume();

    friend class TView;
    friend void genRefs();
#ifndef __UNIX__
    friend unsigned long getTicks(void);
#endif
    friend class TProgram;
    static ushort doubleDelay;
    static Boolean mouseReverse;

private:

    static TMouse mouse;
    static void getMouseState( TEvent& );
#ifdef __DOS32__
    friend void _loadds _far mouseInt(int flag,int buttons,int x,int y);
#   pragma aux mouseInt parm [EAX] [EBX] [ECX] [EDX];
#else
    static void mouseInt();
#endif

    static void setLast( TEvent& );

    static MouseEventType lastMouse;
    static MouseEventType curMouse;

    static MouseEventType downMouse;
    static ushort downTicks;

    static ushort *Ticks;
    static TEvent eventQueue[ eventQSize ];
    static TEvent *eventQHead;
    static TEvent *eventQTail;
    static Boolean mouseIntFlag;
    static ushort eventCount;

    static Boolean mouseEvents;

    static ushort repeatDelay;
    static ushort autoTicks;
    static ushort autoDelay;

#ifndef __MSDOS__
public:
    static void mouseThread(void* arg);
    static void keyboardThread( void * arg );
    static TEvent keyboardEvent;
    static unsigned char shiftState;
#endif

};

inline void TEvent::getMouseEvent()
{
    TEventQueue::getMouseEvent( *this );
}

#endif  // Uses_TEventQueue

#if defined( Uses_TScreen ) && !defined( __TScreen )
#define __TScreen

#ifdef PROTECT

extern ushort monoSeg;
extern ushort colrSeg;
extern ushort biosSeg;

#endif

class TDisplay
{

public:

    friend class TView;

    enum videoModes
        {
        smBW80      = 0x0002,
        smCO80      = 0x0003,
        smMono      = 0x0007,
        smFont8x8   = 0x0100
        };

    static void clearScreen( int, int );

    static void setCursorType( ushort );
    static ushort getCursorType();

    static ushort getRows();
    static ushort getCols();

    static void setCrtMode( ushort );
    static ushort getCrtMode();

protected:

    TDisplay() { updateIntlChars(); };
    TDisplay( const TDisplay& ) { updateIntlChars(); };
    ~TDisplay() {};

private:

    static void videoInt();
    static void updateIntlChars();

    static ushort *equipment;
    static uchar *crtInfo;
    static uchar *crtRows;

};

class TScreen : public TDisplay
{

public:

    TScreen();
    ~TScreen();
#ifdef __UNIX__
    /**
     * Returns the first available event.
     */
    static void getEvent(TEvent &event);
    /**
     * Emits a beep.
     */
    static void makeBeep();
    /**
     * Puts an event in the event queue.
     *
     * Do not use it, use @ref TProgram::putEvent() if you need.
     */
    static void putEvent(TEvent &event);
#endif
    /**
     * Recovers the execution of the application.
     *
     * Resumes the execution of the process after the user stopped it.
     * Called by @ref TApplication::resume(). You should call the latter
     * method.
     */
    static void resume();
    /**
     * Stops the execution of the application.
     *
     * Suspends execution of the process.
     * Called by @ref TApplication::suspend(). You should call the latter
     * method.
     */
    static void suspend();
#ifdef __UNIX__
    /**
     * Shows or hides the cursor.
     *
     * Flag `show' specifies the operation to perform.
     */
    static void drawCursor(int show);
    /**
     * Moves the cursor to another place.
     *
     * Parameters `x' and `y' are 0-based.
     */
    static void moveCursor(int x, int y);
    /**
     * Writes a row of character & attribute pairs on the screen.
     *
     * `dst' is the destination position, `src' is a pointer to the source
     * buffer and `len' is the size of the buffer expressed as the number
     * of pairs.
     */
    static void writeRow(int dst, ushort *src, int len);
#endif

    static void setVideoMode( ushort mode );
    static void clearScreen();

    static ushort startupMode;
    static ushort startupCursor;
    static ushort screenMode;
    /**
     * Holds the current screen width.
     *
     * It is initialized by the constructor of this class.
     */
    static int screenWidth;
    /**
     * Holds the current screen height.
     *
     * It is initialized by the constructor of this class.
     */
    static int screenHeight;
    static Boolean hiResScreen;
    static Boolean checkSnow;
    /**
     * Holds the current screen buffer address.
     *
     * It is initialized by the constructor of this class.
     */
    static ushort *screenBuffer;
    static ushort cursorLines;

    static void setCrtData();
    static ushort fixCrtMode( ushort );


};

#endif  // Uses_TScreen

#if defined( Uses_TSystemError ) && !defined( __TSystemError )
#define __TSystemError

class TDrawBuffer;

class TSystemError
{

public:

    TSystemError();
    ~TSystemError();

    static Boolean ctrlBreakHit;

    static void TV_CDECL suspend();
    static void TV_CDECL resume();
    static short ( * TV_CDECL sysErrorFunc )( short, uchar );

private:

    static ushort sysColorAttr;
    static ushort sysMonoAttr;
    static Boolean saveCtrlBreak;
    static Boolean sysErrActive;

    static void TV_CDECL swapStatusLine( TDrawBuffer & );
    static ushort selectKey();
    static short TV_CDECL sysErr( short, uchar );

    static Boolean inIDE;

    static const char *const errorString[24];
    static const char *sRetryOrCancel;

    friend class Int11trap;

};

#ifdef __DOS16__

class Int11trap
{

public:

    Int11trap();
    ~Int11trap();

private:

    static void interrupt handler(...);
    static void interrupt (*oldHandler)(...);

};

#endif  // __DOS16__

#endif  // Uses_TSystemError

#if defined( Uses_TThreaded ) && !defined( __TThreaded)
#define __TThreaded

#ifdef __OS2__
#define INCL_NOPMAPI
#define INCL_KBD
#define INCL_MOU
#define INCL_VIO
#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSMISC
#include <os2.h>

#pragma pack()
#pragma options -a.

// These must not cross a 64K boundary
struct TiledStruct {
  HMOU mouseHandle;
  MOUEVENTINFO mouseInfo;
  NOPTRRECT ptrArea;
  PTRLOC ptrLoc;
  VIOCURSORINFO cursorInfo;
  VIOMODEINFO modeInfo;
  KBDKEYINFO keyboardInfo;
  KBDINFO    keyboardShiftInfo;
};

class TThreads {
public:
  TThreads();
  ~TThreads();
  static void resume();
  static void suspend();
  static void deadEnd();
  static TiledStruct *tiled; // tiled will be allocated with DosAllocMem
  static TID mouseThreadID;
  static TID keyboardThreadID;
  static SEMRECORD evCombined[2];
  static HEV &hevMouse1;
  static HEV &hevKeyboard1;
  static HEV hevKeyboard2;
  static HEV hevMouse2;
  static HMTX hmtxMouse1;
  static HMUX hmuxMaster;
  static volatile int shutDownFlag;
};

#endif // __OS2__

#ifdef __NT__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// BCB5 does not define INVALID_FILE_ATTRIBUTES for some reason
#if defined(__BORLANDC__) && __BORLANDC__ < 0x560 && !defined(INVALID_FILE_ATTRIBUTES)
#define INVALID_FILE_ATTRIBUTES (-1)
#endif

class TThreads {
public:
  TThreads(void)        { resume(); }
  ~TThreads(void)       { suspend(); }
  static void resume();
  static void suspend();
  static HANDLE chandle[2];
#define cnInput  0
#define cnOutput 1
  static DWORD consoleMode;
  static CONSOLE_CURSOR_INFO crInfo;
  static CONSOLE_SCREEN_BUFFER_INFO sbInfo;
  static INPUT_RECORD ir;
  static DWORD evpending;       // is event present in 'ir'?
  static INPUT_RECORD *get_next_event(void);
  static inline int ispending(void) { return evpending; }
  static inline void accept_event(void) { evpending = 0; }
  static int macro_playing;
  static bool my_console;
#define MAX_GET_FROM_CLIP	(200*133)  // maximum size for insert in editdialog's
  static char *clipboard_get(size_t &sz, bool line);
  static bool clipboard_put(const char *str, size_t from, size_t to);
};

#endif // __NT__

#ifdef __X11__
class TThreads {
public:
#define MAX_GET_FROM_CLIP	(200*133)  // maximum size for insert in editdialog's
  static char *clipboard_get(size_t &sz, bool line);
  static bool clipboard_put(const char *str, size_t from, size_t to);
};
#endif

#endif // Uses_TThreaded

