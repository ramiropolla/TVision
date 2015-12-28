/*------------------------------------------------------------*/
/* filename -       tscreen.cpp                               */
/*                                                            */
/* function(s)                                                */
/*                  TScreen member functions                  */
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
/*                                                            */
/*    Turbo Vision -  Version 1.0                             */
/*                                                            */
/*                                                            */
/*    Copyright (c) 1991 by Borland International             */
/*    All Rights Reserved.                                    */
/*                                                            */
/*------------------------------------------------------------*/

#define Uses_TScreen
#define Uses_TEvent
#define Uses_TThreaded
#include <tv.h>

#include <stdio.h>
#include <assert.h>

#ifndef __UNIX__
#ifdef __MSDOS__
#include <dos.h>
ushort *TDisplay::equipment = (ushort *)MK_FP( 0, 0x410 );
uchar *TDisplay::crtInfo = (uchar *)MK_FP( 0, 0x487 );
uchar *TDisplay::crtRows = (uchar *)MK_FP( 0, 0x484 );
#endif

ushort TScreen::startupMode = 0xFFFF;
ushort TScreen::startupCursor = 0;
#ifdef __MSDOS__
ushort TScreen::screenMode = 0;
#else
ushort TScreen::screenMode = (80 << 8) | 25;
#endif
int TScreen::screenWidth = 0;
int TScreen::screenHeight = 0;
Boolean TScreen::hiResScreen = False;
Boolean TScreen::checkSnow = False;
ushort *TScreen::screenBuffer = 0;
ushort TScreen::cursorLines = 0;
#endif // !__UNIX__
//--------------------------------------------------------------------------
#ifdef __MSDOS__
#define maxViewHeight 100
#else
#define maxViewHeight 300
#endif

#ifndef __UNIX__
static void checksize(int height, int width)
{
  if ( height > maxViewHeight )
  {
    fprintf(stderr,"\n\n\nFatal error: the window is too high (max %d rows)!\n", maxViewHeight);
    _exit(0);
  }
  if ( (size_t)width > maxViewWidth )
  {
    fprintf(stderr,"\n\n\nFatal error: the window is too wide (max %d columns)!\n", maxViewWidth);
    _exit(0);
  }
}
#endif // !__UNIX__

//--------------------------------------------------------------------------

#ifdef __MSDOS__
void TScreen::resume()
{
    startupMode = getCrtMode();
    startupCursor = getCursorType();
    if (screenMode != startupMode)
       setCrtMode( screenMode );
    setCrtData();
}

TScreen::~TScreen()
{
    suspend();
}
#endif // __MSDOS__

//--------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef __DOS32__

ushort TDisplay::getCursorType()
{
  REGS r;
  r.h.ah = 3;
  r.h.bh = 0;
  int386(0x10,&r,&r);
  return r.w.cx;
}

void TDisplay::setCursorType( ushort ct )
{
  REGS r;
  r.h.ah = 1;
  r.w.cx = ct;
  int386(0x10,&r,&r);
}

void TDisplay::clearScreen( int w, int h )
{
  REGS r;
  r.w.ax = 0x0600;
  r.h.bh = 0x07;
  r.w.cx = 0;
  r.h.dl = uchar(w);
  r.h.dh = uchar(h - 1);
  int386(0x10,&r,&r);
  r.h.ah = 0x10;
  r.h.al = 0x03;
  r.h.bl = 0x00;        // don't blink, just make intense
  int386(0x10,&r,&r);
}

ushort TDisplay::getRows()
{
  REGS r;
  r.w.ax = 0x1130;
  r.h.bh = 0x0;
  r.h.dl = 0x0;
  int386(0x10,&r,&r);
  if( r.h.dl == 0 ) r.h.dl = 24;
  return r.h.dl + 1;
}

ushort TDisplay::getCols()
{
  REGS r;
  r.h.ah = 0x0F;
  int386(0x10,&r,&r);
  return r.h.ah;
}

ushort TDisplay::getCrtMode()
{
    REGS r;
    r.h.ah = 0x0F;
    int386(0x10,&r,&r);
    ushort mode = r.h.al;
#ifndef __NOROW__
    if( getRows() > 25 )
        mode |= smFont8x8;
#endif
    return mode;
}


void TDisplay::setCrtMode( ushort mode )
{
    *equipment &= 0xFFCF;
    *equipment |= (mode == smMono) ? 0x30 : 0x20;
    *crtInfo &= 0x00FE;

  REGS r;
  r.h.ah = 0x0;
  r.h.al = mode;
  int386(0x10,&r,&r);

#ifndef __NOROW__
    if( (mode & smFont8x8) != 0 )
        {
        r.w.ax = 0x1112;
        r.h.bl = 0;
        int386(0x10,&r,&r);

        if( getRows() > 25 )
            {
            *crtInfo |= 1;

            r.h.ah = 1;
            r.w.cx = 0x0607;
            int386(0x10,&r,&r);

            r.h.ah = 0x12;
            r.h.bl = 0x20;
            int386(0x10,&r,&r);
            }
        }
#endif // __NOROW__
}

ushort TScreen::fixCrtMode( ushort mode )
{
#ifdef __NOROW__
    return mode;
#else
    char m = mode;
    if( m != smMono && m != smCO80 && m != smBW80 )
        m = smCO80;
    return (mode & ~0xFF) + m;
#endif // __NOROW__
}

void TScreen::setCrtData()
{
    screenMode = getCrtMode();
    screenWidth = getCols();
    screenHeight = getRows();
    checksize(screenHeight, screenWidth);
    hiResScreen = Boolean(screenHeight > 25);

    if( screenMode == smMono )
        {
        screenBuffer = (ushort *)MK_FP( 0, 0xB0000 );
        checkSnow = False;
        }
    else
        {
        screenBuffer = (ushort *)MK_FP( 0, 0xB8000 );
        if( hiResScreen )
            checkSnow = False;
        }

    cursorLines = getCursorType();
    setCursorType( 0x2000 );

}

#endif // __DOS32__
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef __OS2__

ushort TDisplay::getCursorType()
{
   VIOCURSORINFO cinfo;
   VioGetCurType(&cinfo,NULL);
   return (cinfo.yStart << 8) + cinfo.cEnd;
}

void TDisplay::setCursorType( ushort ct )
{
   VIOCURSORINFO cinfo;
   VioGetCurType(&cinfo,NULL);
   if ( (ct>>8) == 0x20 ) {
     cinfo.attr   = 0xFFFF;          // hide
   } else {
     cinfo.attr   = 0;               // show
     cinfo.yStart = uchar(ct>>8);
     cinfo.cEnd   = uchar(ct);
     cinfo.cx     = 0;
   }
   VioSetCurType(&cinfo,NULL);
}

void TDisplay::clearScreen( int w, int h )
{
   char cell = ' ';
   VioScrollUp (0,0,h,w,h,&cell,NULL);
}

ushort TDisplay::getRows()
{
    VIOMODEINFO info;
    info.cb=sizeof(VIOMODEINFO);
    VioGetMode(&info,0);
    return info.row;
}

ushort TDisplay::getCols()
{
    VIOMODEINFO info;
    info.cb=sizeof(VIOMODEINFO);
    VioGetMode(&info,0);
    return info.col;
}

ushort TDisplay::getCrtMode()
{
  return getRows() + (getCols() << 8);
}

void TDisplay::setCrtMode( ushort mode )
{
    VIOMODEINFO info;
    info.cb = sizeof(VIOMODEINFO);
    VioGetMode(&info,0);
    info.cb = 8;
    info.col = uchar(mode >> 8);
    info.row = uchar(mode);
    checksize(info.row, info.col);
    VioSetMode(&info,0);
    clearScreen(info.col,info.row);
}

void TScreen::resume()
{
    if (screenMode != startupMode)
       setCrtMode( screenMode );
    setCrtData();
}

TScreen::~TScreen()
{
}

ushort TScreen::fixCrtMode( ushort mode )
{
  if ( char(mode) == 0 || char(mode>>8) == 0 ) return 0;
  return mode;
}

void TScreen::setCrtData()
{
    screenMode = getCrtMode();
    screenWidth = getCols();
    screenHeight = getRows();
    checksize(screenHeight, screenWidth);
    hiResScreen = Boolean(screenHeight > 25);

    ushort _far16 *s;
    ushort l;
    VioGetBuf( (PULONG) &s, &l, 0 ); // !!! OS/2 specific
    screenBuffer = s; // Automatic conversion

    checkSnow = False;
    cursorLines = getCursorType();
    setCursorType( 0x2000 );

}

#endif  // __OS2__

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef __NT__

//void doom(const char *format, ...)
//{
//  static FILE *fp = NULL;
//  if ( fp == NULL ) fp = fopen("doom", "w");
//  if ( fp != NULL )
//  {
//    va_list va;
//    va_start(va, format);
//    vfprintf(fp, format, va);
//    va_end(va);
//    fflush(fp);
//  }
//}


ushort TDisplay::getCursorType()
{
   GetConsoleCursorInfo(TThreads::chandle[cnOutput], &TThreads::crInfo);
   int ct = TThreads::crInfo.bVisible
                ? TThreads::crInfo.dwSize*31/100
                : 0x2000;
   return ushort(ct);
}

void TDisplay::setCursorType( ushort ct )
{
  if ( (ct>>8) == 0x20 ) {
    TThreads::crInfo.bVisible = False;
    TThreads::crInfo.dwSize = 1;
  } else {
    TThreads::crInfo.bVisible = True;
    TThreads::crInfo.dwSize = (uchar(ct)-uchar(ct>>8))*100/31;
  }
  SetConsoleCursorInfo(TThreads::chandle[cnOutput], &TThreads::crInfo);
}

void TDisplay::clearScreen( int w, int h )
{

// Another Win95/98 bug workaround:
//   FillConsole... functions fail if they are called immediately
//   after changing the screen buffer size.
//   I think this happens because Windows thinks that the window is already
//   blank and doesn't redraw it.

//   We just wait a little bit to give Windows the change to realize
//   that the window size has changed.

  DWORD written;
  COORD from  = { 0, 0 };
#if 0
  COORD bsize = { 1, h };
  SMALL_RECT to = { w-1, 0, w-1, h-1 };
  CHAR_INFO cbuf[maxViewWidth];
  for ( int i=0; i < h; i++ )
  {
    cbuf[i].Char.AsciiChar = '.';
    cbuf[i].Attributes     = 0x07;
  }
  WriteConsoleOutput(TThreads::chandle[cnOutput],cbuf,bsize,from,&to);
#endif

  Sleep(100);

//  printf("after grb (%d,%d)\n", w, h), gets((char*)cbuf);

//doom("writechar %d %d\n", code, GetLastError());

  FillConsoleOutputAttribute( TThreads::chandle[cnOutput], 0x07, w*h, from, &written );
//doom("fillattr w=%d h=%d m=%d wr=%d ret=%d %d\n", w, h, w*h, written, code, GetLastError());
  FillConsoleOutputCharacterA( TThreads::chandle[cnOutput], ' ', w*h, from, &written );
//doom("fillchar w=%d h=%d m=%d wr=%d ret=%d %d\n", w, h, w*h, written, code, GetLastError());

//  printf("after fill (%d,%d) %d\n", w, h, code), gets((char*)cbuf);
}

ushort TDisplay::getRows()
{
  GetConsoleScreenBufferInfo( TThreads::chandle[cnOutput], &TThreads::sbInfo );
  return TThreads::sbInfo.dwSize.Y;
}

ushort TDisplay::getCols()
{
  GetConsoleScreenBufferInfo( TThreads::chandle[cnOutput], &TThreads::sbInfo );
  return TThreads::sbInfo.dwSize.X;
}

ushort TDisplay::getCrtMode()
{
  int rows = getRows();
  if ( rows >= 0xFF ) rows = 0xFF;
  return ushort(rows + (getCols() << 8));
}

void TDisplay::setCrtMode( ushort mode )
{
  int oldr = getRows();
  int oldc = getCols();
  int cols = uchar(mode >> 8);
  int rows = uchar(mode);      if ( rows == 0xFF ) rows = maxViewHeight;
  if ( cols == 0 ) cols = oldc;
  if ( rows == 0 ) rows = oldr;
//  if ( oldr == rows && oldc == cols ) return;   // nothing to do
  checksize(rows, cols);
  COORD newSize = { SHORT(cols), SHORT(rows) };
  SMALL_RECT rect = { 0, 0, SHORT(cols-1), SHORT(rows-1) };

#if 1   // it seems that better to check it (25.05.99)
  COORD maxSize = GetLargestConsoleWindowSize( TThreads::chandle[cnOutput] );
  if ( newSize.X > maxSize.X ) {
    newSize.X = maxSize.X;
    rect.Right = newSize.X-1;
  }
  if ( newSize.Y > maxSize.Y ) {
    newSize.Y = maxSize.Y;
    rect.Bottom = newSize.Y-1;
  }
//  doom("adjusted h=%d x=%d y=%d\n",TThreads::chandle[cnOutput],newSize.X,newSize.Y);
#endif

//
// We MUST turn the cursor on before changing the screen size.
// Otherwise the application will crash under Win95/98 if this is the very
// first console application after reboot.
// I've lost one whole day trying to figure it out (endless reboots...)
//
  TThreads::crInfo.bVisible = True;
  TThreads::crInfo.dwSize = 10;         // small cursor
//  int code =
    SetConsoleCursorInfo(TThreads::chandle[cnOutput],&TThreads::crInfo);
//doom("set_cursor_info %d %d %d\n", TThreads::chandle[cnOutput], code, GetLastError());

  COORD zero = { 0, 0 };
//  code =
    SetConsoleCursorPosition(TThreads::chandle[cnOutput], zero);
//doom("set_cursor_pos %d %d %d\n", TThreads::chandle[cnOutput], code, GetLastError());

  if ( oldr <= rows )
  {
    if ( oldc <= cols )
    {                           // increasing both dimensions
BUFWIN:
//doom("1bufsize %d (%d,%d)\n", TThreads::chandle[cnOutput], newSize.X, newSize.Y);
//      SetLastError(0);
//      code =
      SetConsoleScreenBufferSize( TThreads::chandle[cnOutput], newSize );
//doom("1bufsize %d %d\n", code, GetLastError());
//doom("2windowinfo %d (%d,%d)-(%d,%d)\n", TThreads::chandle[cnOutput], rect.Left, rect.Top, rect.Right, rect.Bottom);
//      SetLastError(0);
//      code =
      SetConsoleWindowInfo( TThreads::chandle[cnOutput], True, &rect );
//doom("2windowinfo %d %d\n", code, GetLastError());
    }
    else
    {                           // cols--, rows+
      SMALL_RECT tmp = { 0, 0, SHORT(cols-1), SHORT(oldr-1) };
//      SetLastError(0);
//      code =
      SetConsoleWindowInfo( TThreads::chandle[cnOutput], True, &tmp );
//doom("3windowinfo %d %d %d (%d,%d)-(%d,%d)\n", code, TThreads::chandle[cnOutput], GetLastError(), tmp.Left, tmp.Top, tmp.Right, tmp.Bottom);
      goto BUFWIN;
    }
  }
  else
  {
    if ( oldc < cols )
    {                           // cols+, rows--
      SMALL_RECT tmp = { 0, 0, SHORT(oldc-1), SHORT(rows-1) };
//      SetLastError(0);
//      code =
      SetConsoleWindowInfo( TThreads::chandle[cnOutput], True, &tmp );
//doom("4windowinfo %d %d %d (%d,%d)-(%d,%d)\n", code, TThreads::chandle[cnOutput], GetLastError(), tmp.Left, tmp.Top, tmp.Right, tmp.Bottom);
      goto BUFWIN;
    }
    else
    {                           // cols--, rows--
//doom("5windowinfo %d (%d,%d)-(%d,%d)\n", TThreads::chandle[cnOutput], rect.Left, rect.Top, rect.Right, rect.Bottom);
//      SetLastError(0);
//      code =
      SetConsoleWindowInfo( TThreads::chandle[cnOutput], True, &rect );
//doom("5windowinfo %d %d\n", code, GetLastError());

//doom("6bufsize %d (%d,%d)\n", TThreads::chandle[cnOutput], newSize.X, newSize.Y);
//      SetLastError(0);
//      code =
      SetConsoleScreenBufferSize( TThreads::chandle[cnOutput], newSize );
//doom("6bufsize %d %d\n", code, GetLastError());
    }
  }

  OSVERSIONINFO osv;
  osv.dwOSVersionInfoSize = sizeof(osv);
  GetVersionEx(&osv);
  if ( osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) // WIN95
  {
    char *delay = getenv("TV_DELAY");
    Sleep(delay ? atoi(delay) : 500);
  }

  int nn = 0;
  do
  {
    GetConsoleScreenBufferInfo( TThreads::chandle[cnOutput], &TThreads::sbInfo);
    nn++;
    if ( nn >= 1000 ) break;
  } while ( TThreads::sbInfo.dwSize.X != cols
         || TThreads::sbInfo.dwSize.Y != rows);

//  TDisplay::clearScreen(TThreads::sbInfo.dwSize.X, TThreads::sbInfo.dwSize.Y);
//  char lll[5];
//  printf("after clearscreen %d %d\n", TThreads::sbInfo.dwSize.X, TThreads::sbInfo.dwSize.Y), gets(lll);
}

void TScreen::resume()
{
//  fprintf(stderr, "\nscreenMode %x startupMode %x\n", screenMode, startupMode); getchar();
  if (screenMode != startupMode)
     setCrtMode( screenMode );
  setCrtData();
}

TScreen::~TScreen()
{
  if ( screenBuffer != NULL ) delete screenBuffer; // VirtualFree( screenBuffer, 0, MEM_RELEASE );
  screenBuffer = NULL;
}

ushort TScreen::fixCrtMode( ushort mode )
{
  if ( char(mode) == 0 || char(mode>>8) == 0 ) return 0;
  return mode;
}

void TScreen::setCrtData()
{
  screenMode = getCrtMode();
  screenWidth = getCols();
  screenHeight = getRows();
  checksize(screenHeight, screenWidth);
  hiResScreen = Boolean(screenHeight > 25);

  short x = (short)screenWidth;
  short y = (short)screenHeight;
  if( x < maxViewWidth  ) x = maxViewWidth;
  if( y < maxViewHeight ) y = maxViewHeight;   // 512*100*2 = 1024*100 = 102 400

  if ( screenBuffer == NULL )
  {
    screenBuffer = new ushort[x * y];
    if ( screenBuffer == NULL )
    {
      fprintf(stderr, "\nFATAL: Can't allocate a screen buffer!\n");
      _exit(1);
    }
  }
  checkSnow = False;
  cursorLines = getCursorType();
  setCursorType( 0x2000 );
}

#endif  // __NT__

//---------------------------------------------------------------------------
#ifndef __UNIX__
TScreen::TScreen()
{
#ifdef __NT__
  TThreads::resume();   // make sure the console is inited
#endif
  if ( startupMode == 0xFFFF ) {
    startupMode = getCrtMode();
    startupCursor = getCursorType();
    setCrtData();               // alloc buf, set screenMode
  }
}

void TScreen::suspend()
{
    if( startupMode != screenMode )
        setVideoMode( startupMode );
    clearScreen();
    setCursorType( startupCursor );
}

void TScreen::clearScreen()
{
    TDisplay::clearScreen( screenWidth, screenHeight );
}
#endif // !__UNIX__

void TScreen::setVideoMode( ushort mode )
{
#ifndef __UNIX__
    setCrtMode( fixCrtMode( mode ) );
    setCrtData();
#else  // __UNIX__
   (void)mode;
#endif // !__UNIX__
}
