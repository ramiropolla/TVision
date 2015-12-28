// os2handl.cpp
//
// - Functions that are not supplied by icc or emx but are used by TVision.
// - Threads and Semaphores
//
// There are a lot of #ifs in this file. It took a long time to get the thread stuff
// working, I kept all the unsuccessful attempts.
//
// Copyright 1993 by J”rn Sierwald

#define Uses_TEventQueue
#define Uses_TScreen
#define Uses_TThreaded
#define Uses_TKeys
#define Uses_TProgram
#include <tv.h>
#ifdef __IDA__
#include <prodir.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <errno.h>

#ifdef __OS2__

TiledStruct *TThreads::tiled;        // tiled will be allocated with DosAllocMem. The TiledStruct will
                                     // not cross a 64K boundary, therefore the members can be used by
                                     // 16Bit functions.
TID TThreads::mouseThreadID;
TID TThreads::keyboardThreadID;
SEMRECORD TThreads::evCombined[2];      // Two Event Semaphores, one for the Mouse and one for Keyboard.
                                        // The main thread will do a MuxWait for both.
HEV &TThreads::hevMouse1   = (HEV&) TThreads::evCombined[0].hsemCur;
HEV &TThreads::hevKeyboard1= (HEV&) TThreads::evCombined[1].hsemCur;
                                        // Just an alias.
HEV TThreads::hevMouse2;                // _beginthread can't create a suspended thread, so
                                        // we keep it waiting with this one
HEV TThreads::hevKeyboard2;             // Signals that the keypress is received.
HMTX TThreads::hmtxMouse1;              // Access to the internal mouse queue
HMUX TThreads::hmuxMaster;              // MuxWait for evCombined.
volatile int TThreads::shutDownFlag=0;

void mouseThread( void * arg )    { TEventQueue::mouseThread(arg); }
void keyboardThread( void * arg ) { TEventQueue::keyboardThread(arg); }
                                        // ICC doesn't want to call member functions as threads
void TThreads::deadEnd() {
  // Kill The current thread
  _endthread();
}

TThreads::TThreads()  {}
TThreads::~TThreads() {}

void TThreads::resume() {
//  cerr << "TThreads::resume\n";
  shutDownFlag=0;
  assert(! DosAllocMem((void**)&tiled,sizeof(TiledStruct),fALLOC | OBJ_TILE)    );
  tiled->modeInfo.cb = (unsigned short) sizeof(VIOMODEINFO);

  if ( MouOpen ((PSZ) 0, &tiled->mouseHandle) != 0 ) tiled->mouseHandle = 0xFFFF;

  assert(! DosCreateEventSem( NULL, &hevMouse1, 0, 0 )      );
  assert(! DosCreateEventSem( NULL, &hevMouse2, 0, 0 )      );
  assert(! DosCreateEventSem( NULL, &hevKeyboard1, 0, 0 )   );
  assert(! DosCreateEventSem( NULL, &hevKeyboard2, 0, 0 )   );
  assert(! DosCreateMutexSem( NULL, &hmtxMouse1, 0, 0 )     );

  evCombined[0].ulUser=0;
  evCombined[1].ulUser=1;

  assert(!DosCreateMuxWaitSem(NULL,&hmuxMaster,2,evCombined,DCMW_WAIT_ANY));

  mouseThreadID = 0xFFFF;
  if ( tiled->mouseHandle != 0xFFFF ) {
#ifdef __BORLANDC__
    mouseThreadID = _beginthread(mouseThread,16384,NULL);
#else
    mouseThreadID = _beginthread(mouseThread,NULL,16384,NULL);
#endif
  }
  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,31,mouseThreadID);
  DosPostEventSem(TThreads::hevMouse2);
#ifdef __BORLANDC__
  keyboardThreadID = _beginthread(keyboardThread,16384,NULL);
#else
  keyboardThreadID = _beginthread(keyboardThread,NULL,16384,NULL);
#endif
  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,31,keyboardThreadID);
}

void TThreads::suspend() {
  shutDownFlag=1;
  TID localTID=mouseThreadID;
    if ( localTID != 0xFFFF ) DosWaitThread(&localTID,DCWW_WAIT);
//  cerr << " MouseThread has ended\n";
  ULONG count;
  assert(! DosQueryEventSem(TThreads::hevKeyboard2,&count) );
  if ( !count )
    assert(! DosPostEventSem(TThreads::hevKeyboard2) ); // Make sure the thread is running.
  localTID=keyboardThreadID;
    DosWaitThread(&localTID,DCWW_WAIT);
//  cerr << " KeyboardThread has ended\n";

  assert(!DosCloseMutexSem(hmtxMouse1)  );
  assert(!DosCloseEventSem(hevKeyboard2) );
  assert(!DosCloseEventSem(hevKeyboard1) );
  assert(!DosCloseEventSem(hevMouse2) );
  assert(!DosCloseEventSem(hevMouse1) );
  assert(!DosCloseMuxWaitSem(hmuxMaster) );

  if ( tiled->mouseHandle != 0xFFFF ) MouClose(tiled->mouseHandle); // This doesn't work, the mouseThread uses the handle.
  assert(! DosFreeMem(tiled) ); // Better not, dito
}

uint32 getTicks() {
// return a value that can be used as a substitute for the DOS Ticker at [0040:006C]
  uint32 m;
  DosQuerySysInfo( 14, 14, &m, sizeof(m));
  return m/52;
}

unsigned char getShiftState() {
// returns a value that can be used as a substitute for the shift state at [0040:0017]
  TThreads::tiled->keyboardShiftInfo.cb = 10;
  KbdGetStatus(&TThreads::tiled->keyboardShiftInfo, 0 );
  return TThreads::tiled->keyboardShiftInfo.fsState & 0xFF;
}

#endif  // __OS2__

//---------------------------------------------------------------------------

#ifdef __NT__

HANDLE TThreads::chandle[2];
DWORD TThreads::consoleMode;
CONSOLE_CURSOR_INFO TThreads::crInfo;
CONSOLE_SCREEN_BUFFER_INFO TThreads::sbInfo;
INPUT_RECORD TThreads::ir;
DWORD TThreads::evpending;
int TThreads::macro_playing = 0;
static int inited = 0;

TThreads tvision_for_win32_starter;     // just to call resume()

void TThreads::resume() {
  if ( !inited )
  {
    SetFileApisToOEM();
    chandle[cnInput]  = GetStdHandle( STD_INPUT_HANDLE );
    chandle[cnOutput] = GetStdHandle( STD_OUTPUT_HANDLE );
    GetConsoleCursorInfo( chandle[cnOutput], &crInfo );
    GetConsoleScreenBufferInfo( chandle[cnOutput], &sbInfo );
    GetConsoleMode(chandle[cnInput],&consoleMode);
    consoleMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    consoleMode |= ENABLE_WINDOW_INPUT;
    SetConsoleMode(chandle[cnInput],consoleMode);
    evpending = 0;
    inited = 1;
  }
}

void TThreads::suspend()
{
  inited = 0;
//  SetConsoleMode(chandle[cnOutput],
//               ENABLE_PROCESSED_OUTPUT|ENABLE_WRAP_AT_EOL_OUTPUT);
}

uint32 getTicks() {
// return a value that can be used as a substitute for the DOS Ticker at [0040:006C]
// To change units from ms to clock ticks.
//   X ms * 1s/1000ms * 18.2ticks/s = X/55 ticks, roughly.
  return GetTickCount() / 55;
}

// hack to allow disabling detection of AltGr key as Right Alt key,
// especially useful for the @ key using AltGr on German keyboards.
// (see http://www.cygwin.com/ml/cygwin/2001-06/msg01007.html
// for a full description of the different 9X/NT problems)
static uchar get_alt_pressed_mask(void)
{
  static uchar alt_pressed_mask = 0;
  if (alt_pressed_mask == 0) // initialize if not already done
  {
    alt_pressed_mask = LEFT_ALT_PRESSED;
    if (!getenv("TV_IGNORE_RIGHT_ALT_PRESSED"))
      alt_pressed_mask |= RIGHT_ALT_PRESSED;
  }
  return alt_pressed_mask;
}

unsigned char getShiftState() {
// returns a value that can be used as a substitute for the shift state at [0040:0017]
  uchar state = (uchar)TThreads::ir.Event.KeyEvent.dwControlKeyState;
  uchar tvstate = 0;
  if ( state & get_alt_pressed_mask() ) tvstate |= kbAltShift;
  if ( state & (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED) ) tvstate |= kbCtrlShift;
  if ( state & SHIFT_PRESSED ) tvstate |= kbLeftShift;
  if ( state & NUMLOCK_ON    ) tvstate |= kbNumState;
  if ( state & SCROLLLOCK_ON ) tvstate |= kbScrollState;
  if ( state & CAPSLOCK_ON   ) tvstate |= kbCapsState;
  return tvstate;
}

#define IO_RAW_EVENT    0
#define IO_CHR_EVENT    1
#define IO_IGN_EVENT    2
static int event_type(INPUT_RECORD &ir) {
  if ( ir.EventType == MOUSE_EVENT ) return IO_RAW_EVENT;
  if ( ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown ) {
    //  Skip shifts and caps,num,scroll locks
    static const char ignore[] = { 0x1D,0x2A,0x38,0x36,0x3A,0x45,0x46,0 };
    if ( strchr(ignore,ir.Event.KeyEvent.wVirtualScanCode) != NULL )
      return IO_IGN_EVENT;
    uchar chr = ir.Event.KeyEvent.uChar.AsciiChar;
    if ( chr == 0 ) return IO_RAW_EVENT;
// ++++++++++++ DEBUGGING CODE
#if 0
  printf("vcode=%x vscan=%x ascii=%c\n",
                ir.Event.KeyEvent.wVirtualKeyCode,
                ir.Event.KeyEvent.wVirtualScanCode,
                ir.Event.KeyEvent.uChar.AsciiChar);
#endif
// ++++++++++++ END OF DEBUGGING CODE

    // the keypad keys are different from normal keys:
    switch ( ir.Event.KeyEvent.wVirtualKeyCode )
    {
      case VK_ADD:
      case VK_SUBTRACT:
        return IO_RAW_EVENT;
    }

    int state = ir.Event.KeyEvent.dwControlKeyState;
//    if ( state & (RIGHT_ALT_PRESSED|LEFT_CTRL_PRESSED) ) return IO_CHR_EVENT;
    if ( state & (get_alt_pressed_mask()
                 |RIGHT_CTRL_PRESSED
                 |LEFT_CTRL_PRESSED) ) return IO_RAW_EVENT;
    static const char chars[] = "`1234567890-=   ~!@#$%^&*()_+"
                                "qwertyuiop[]\\  QWERTYUIOP{}|"
                                "asdfghjkl;'     ASDFGHJKL:\""
                                "zxcvbnm,./      ZXCVBNM<>?";
    if ( strchr(chars,chr) != NULL ) return IO_CHR_EVENT;
    return IO_RAW_EVENT;
  }
  return IO_IGN_EVENT;
}

int changed_nt_console_size = 0;

INPUT_RECORD *TThreads::get_next_event(void)
{
  if ( evpending ) return &ir;
  PeekConsoleInput(chandle[cnInput],&ir,1,&evpending);
  if ( evpending )
  {
    int code = event_type(ir);
//    printf("evtype = %d\n",code);
    if ( code  == IO_CHR_EVENT )
    {
      char chr;
//    printf("before readconsole\n");
      ReadConsole(chandle[cnInput],&chr,1,&evpending,NULL);
//    printf("key %x %d\n",chr,evpending);
      ir.Event.KeyEvent.uChar.AsciiChar = chr;
    }
    else
    {
      ReadConsoleInput(chandle[cnInput],&ir,1,&evpending);
      if ( evpending && ir.EventType == WINDOW_BUFFER_SIZE_EVENT )
      {
        COORD nsz = ir.Event.WindowBufferSizeEvent.dwSize;
        if(nsz.X < 80) nsz.X = 80;
        if(nsz.Y < 25) nsz.Y = 25;
        changed_nt_console_size = *(LPDWORD)&nsz;
      }
      if ( code != IO_RAW_EVENT ) accept_event();
    }
  }
  return evpending ? &ir : NULL;
}

//---------------------------------------------------------------------------
bool TThreads::my_console = true;

static void switch_screen_size(HANDLE h, COORD from, COORD to)
{
  SMALL_RECT  r;
  uchar       chg;

  if ( *(LPDWORD)&from == *(LPDWORD)&to ) return;

  ((LPDWORD)&r)[0] = 0;
  ((LPDWORD)&r)[1] = *(LPDWORD)&to - 0x10001;
  chg = 0;

  if ( from.X < to.X )
  {
    r.Right = from.X-1;
    ++chg;
  }
  if ( from.Y < to.Y )
  {
    r.Bottom = from.Y-1;
    ++chg;
  }
  SetConsoleWindowInfo( h, TRUE, &r );
  SetConsoleScreenBufferSize( h, to );
  if( chg )
  {
    ((LPDWORD)&r)[0] = 0; // PARANOYA
    ((LPDWORD)&r)[1] = *(LPDWORD)&to - 0x10001;
    SetConsoleWindowInfo( h, TRUE, &r );
  }
}

// 2 - notify TV of end of debugger (preserved)
// 1 - application screen
// 0 - TV screen
// -1 - display the app screen and wait for a keyboard key, then switch back
// -2 - notify TV of start new debugger (invalidate previous user screen)
// and request ONLY (is switching supported on current display type? )
// return: false if switching can't be realized or invalid status.
bool TProgram::switch_screen(int to_user)
{
  static const COORD zero_coord = { 0, 0 };

  static PCHAR_INFO           user_screen_buffer;
  static COORD                user_screen_size;
  static CONSOLE_CURSOR_INFO  user_cinfo;
  static COORD                user_cursorpos;

  switch ( to_user )
  {
    case -2:
      *(LPDWORD)&user_screen_size &= 0; // flag of recreation
      delete[] user_screen_buffer;
      user_screen_buffer = NULL;
      return(true);

    case -1:
      if( user_screen_buffer ) break;
      //PASSTHRU
    default:  // PARANOYA
      return(false);

    case 0:
    case 1:
      if ( to_user == (int)TThreads::my_console ) return(true);
      break;
  }

  HANDLE  hout = TThreads::chandle[cnOutput];
  if( to_user )
  {
    TThreads::my_console = false;
    SetConsoleCursorPosition(hout, zero_coord);
    if ( user_screen_buffer )
    {
      SMALL_RECT wrg = { 0, 0, user_screen_size.X-1, user_screen_size.Y-1 };
      switch_screen_size(hout, TThreads::sbInfo.dwSize, user_screen_size);
      WriteConsoleOutput(hout, user_screen_buffer, user_screen_size,
                         zero_coord, &wrg);
      SetConsoleCursorPosition(hout, user_cursorpos);
    }
    else if ( !*(LPDWORD)&user_screen_size ) // only first call
    {
      *(LPDWORD)&user_screen_size = (25 << 16) | 80;
      switch_screen_size(hout, TThreads::sbInfo.dwSize, user_screen_size);
      TDisplay::clearScreen(user_screen_size.X, user_screen_size.Y);
      user_cursorpos = zero_coord;  // PARANOYA
      user_cinfo.dwSize = 1;
      user_cinfo.bVisible = TRUE;
    }
    SetConsoleCursorInfo(hout, &user_cinfo);
    if ( to_user > 0 )
      goto done;
    {
      HANDLE        hin = TThreads::chandle[cnInput];

      SetConsoleMode(hin, 0);
      Sleep(500); // anti-blink'ing and release hot-key wait
      FlushConsoleInputBuffer(hin);

      INPUT_RECORD  ir;
      DWORD         c;
      do if ( !ReadConsoleInput(hin, &ir, 1, &c) || !c ) break; // PARANOYA
      while(ir.EventType != KEY_EVENT || !ir.Event.KeyEvent.bKeyDown);
      Sleep(100); // skip immediate unpressing and keyboard 'zummers'
      FlushConsoleInputBuffer(hin);
      SetConsoleMode(hin, TThreads::consoleMode);
    }
  }
  {
    CONSOLE_SCREEN_BUFFER_INFO  cbi;

    GetConsoleCursorInfo(hout, &user_cinfo);
    GetConsoleScreenBufferInfo(hout, &cbi);
    user_cursorpos = cbi.dwCursorPosition;
    SetConsoleCursorPosition(hout, zero_coord);
    if ( *(LPDWORD)&cbi.dwSize != *(LPDWORD)&user_screen_size )
    {
       delete[] user_screen_buffer;
       user_screen_buffer = NULL; // unification
    }
    if ( !user_screen_buffer )
    {
       user_screen_buffer = new CHAR_INFO[cbi.dwSize.X * cbi.dwSize.Y];
       user_screen_size = cbi.dwSize;
    }
    SMALL_RECT rgn = { 0, 0, cbi.dwSize.X-1, cbi.dwSize.Y-1 };
    ReadConsoleOutput(hout, user_screen_buffer, cbi.dwSize, zero_coord, &rgn);
    switch_screen_size(hout, cbi.dwSize, TThreads::sbInfo.dwSize);
    SetConsoleCursorInfo(hout, &TThreads::crInfo);
    TThreads::my_console = true;
    application->redraw();
  }
done:
  return(true);
}

void TProgram::at_child_exec(bool /*before*/) {}

bool TThreads::clipboard_put(const char *str, size_t from, size_t to)
{
    bool res = false;
    if ( to > from && OpenClipboard(NULL) )
        {
        size_t  sz = to - from;
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, sz+1);
        if ( hMem != NULL )
            {
            LPVOID  pc = GlobalLock(hMem);
            if ( pc == NULL ) GlobalFree(hMem);
            else
                {
                  memcpy(pc, str + from, sz);
                  ((char *)pc)[sz] = 0;
                  GlobalUnlock(hMem);
                  EmptyClipboard();
                  if ( SetClipboardData(CF_OEMTEXT, hMem) ) res = true;
                }
            }
        CloseClipboard();
        }
    return res;
}

char *TThreads::clipboard_get(size_t &sz, bool line)
{
    char *answer = NULL;
    if ( IsClipboardFormatAvailable(CF_OEMTEXT) && OpenClipboard(NULL) )
        {
        HGLOBAL hMem = GetClipboardData(CF_OEMTEXT);
        if ( hMem != NULL )
            {
            char *clip = (char *)GlobalLock(hMem);
            if ( clip != NULL )
                {
                size_t  clsz = strlen(clip);
                if ( clsz != 0 )
                    {
                    if ( sz < clsz ) clsz = sz;
                    answer = (char *)malloc(clsz + 1);
                    if ( answer != NULL )
                        {
                        char *p = answer;
                        while ( *clip != 0 )
                            {
                            *p = *clip++;
                            if ( *p < ' ' || *p == 127 )
                                {
                                if ( line && p == answer )
                                    continue;
                                if ( line || (*p != '\r' && *p != '\n') )
                                    *p = ' ';
                                }
                            ++p;
                            }
                            if ( line )
                                for ( ; p > answer; --p )
                                  if ( p[-1] > ' ' ) break;
                            if ( p == answer )
                                {
                                free ( answer);
                                answer = NULL;
                                }
                            else
                                {
                                *p = '\0';
                                sz = p - answer;
                                }
                        }
                    }
                }
            GlobalUnlock(hMem);
            }
        CloseClipboard();
        }
    return answer;
}

#elif !defined(__UNIX__)
#error "Not implemented for this system!"
#endif //__UNIX__

//---------------------------------------------------------------------------

#ifdef __BORLANDC__
int qgetcurdir(int __drive, char *buffer, size_t bufsize)
{
  char buf2[MAXSTR];
  int code = getcurdir(__drive, buf2);
  if ( code != -1 )
    qstrncpy(buffer, buf2, bufsize);
  return code;
}

#else

#ifdef __FAT__

#ifdef _MSC_VER
#pragma comment(lib,"user32")
inline void _dos_getdrive(unsigned *drive)  {  *drive = _getdrive(); }
inline void _dos_setdrive(int drive, unsigned *) { _chdrive(drive); }
#endif // _MSC_VER

int fnsplit(const char *__path,
            char *__drive,
            char *__dir,
            char *__name,
            char *__ext)
{
  _splitpath(__path,__drive,__dir,__name,__ext);
  int code = 0;
  if ( __drive != NULL && __drive[0] != '\0' ) code |= DRIVE;
  if ( __dir   != NULL && __dir  [0] != '\0' ) code |= DIRECTORY;
  if ( __name  != NULL && __name [0] != '\0' ) code |= FILENAME;
  if ( __ext   != NULL && __ext  [0] != '\0' ) code |= EXTENSION;
  return code;
}

/* makepath -- Copyright (c) 1993 by Eberhard Mattes */

static void my_makepath (char *dst, const char *drive, const char *dir,
                const char *fname, const char *ext)
{
  int n;
  char slash;

  n = 0; slash = '/';
  if (drive != NULL && *drive != 0)
    {
      dst[n++] = *drive;
      dst[n++] = ':';
    }
  if (dir != NULL && *dir != 0)
    {
      while (n < MAXPATH - 1 && *dir != 0)
        {
          if (*dir == DIRCHAR)
            slash = DIRCHAR;
          dst[n++] = *dir++;
        }
      if (dst[n-1] != '\\' && dst[n-1] != '/' && n < MAXPATH - 1)
        dst[n++] = slash;
    }
  if (fname != NULL)
    {
      while (n < MAXPATH - 1 && *fname != 0)
        dst[n++] = *fname++;
    }
  if (ext != NULL && *ext != 0)
    {
      if (*ext != '.' && n < MAXPATH - 1)
        dst[n++] = '.';
      while (n < MAXPATH - 1 && *ext != 0)
        dst[n++] = *ext++;
    }
  dst[n] = 0;
}

void fnmerge(char *__path,
                            const char *__drive,
                            const char *__dir,
                            const char *__name,
                            const char *__ext)
{
  my_makepath(__path,__drive,__dir,__name,__ext);
}

int getdisk(void)
{
  unsigned int mydrive;
  _dos_getdrive(&mydrive);
  return mydrive-1;
}

// getcurdir should fill buffer with the current directory without
// drive char and without a leading backslash.

int qgetcurdir(int __drive, char *buffer, size_t bufsize)
{
#define MAXPATHLEN MAXPATH
  int32 size=MAXPATHLEN;
  char tmp[MAXPATHLEN+1];
  tmp[0]=DIRCHAR;
  unsigned int old, dummy;
  _dos_getdrive(&old);
  _dos_setdrive(__drive,&dummy);
  if (getcwd (tmp, size) == 0) { // getcwd returns a leading backslash
    _dos_setdrive(old,&dummy);
    return 0;
  }
  _dos_setdrive(old,&dummy);
  qstrncpy(buffer, &tmp[3], bufsize);
  return 1;
}
#endif  // ifdef __FAT__

#endif  // !__BORLANDC__ ig
