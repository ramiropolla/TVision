/*------------------------------------------------------------*/
/* filename -       tmouse.cpp                                */
/*                                                            */
/* function(s)                                                */
/*                  TMouse and THWMouse member functions      */
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

#define Uses_TEvent
#define Uses_TScreen
#define Uses_TThreaded
#include <tv.h>

#include <assert.h>

uchar THWMouse::buttonCount = 0;
Boolean THWMouse::handlerInstalled = False;
Boolean THWMouse::noMouse = False;

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
#ifdef __DOS32__

THWMouse::THWMouse()
{
    resume();
}

void THWMouse::resume()
{
    if ( noMouse ) return;

    union REGS r;

    r.w.ax = 0;
    int386(0x33,&r,&r);
    if( r.w.ax == 0 ) return;

    buttonCount = r.h.dl;

    r.w.ax = 4;
    r.w.cx = 0;
    r.w.dx = 0;
    int386(0x33,&r,&r);         // move pointer to (0,0)
}

THWMouse::~THWMouse()
{
    suspend();
}

void THWMouse::suspend()
{
    if( present() == False )
        return;
    hide();
    if( handlerInstalled == True )
        {
        registerHandler( 0, 0 );
        handlerInstalled = False;
        }
    buttonCount = 0;
}

void TV_CDECL THWMouse::show() {
  if ( present() ) {
    union REGS r;
    r.w.ax = 1;
    int386(0x33,&r,&r);
  }
}

void TV_CDECL THWMouse::hide() {
  if ( buttonCount != 0 ) {
    union REGS r;
    r.w.ax = 2;
    int386(0x33,&r,&r);
  }
}

void THWMouse::setRange( ushort rx, ushort ry ) {
  if( buttonCount != 0 ) {
    union REGS r;
    r.w.dx = (rx << 3);
    r.w.cx = 0;
    r.w.ax = 7;
    int386(0x33,&r,&r);

    r.w.dx = (ry << 3);
    r.w.cx = 0;
    r.w.ax = 8;
    int386(0x33,&r,&r);
  }
}

void THWMouse::getEvent( MouseEventType& me ) {
  union REGS r;
  r.w.ax = 3;
  int386(0x33,&r,&r);
  me.buttons = r.h.bl;
  me.where.x = r.w.cx >> 3;
  me.where.y = r.w.dx >> 3;
  me.eventFlags = 0;
}

void THWMouse::registerHandler( unsigned mask, void (*func)() ) {
  if( !present() ) return;
  union REGS r;
  struct SREGS sregs;
  r.w.ax = 0xC;
  r.w.cx = mask;
  r.x.edx  = FP_OFF(func);
  sregs.es = FP_SEG(func);
  sregs.fs = sregs.gs = sregs.ds = sregs.es;
  int386x(0x33,&r,&r,&sregs);
  handlerInstalled = True;
}

TMouse::TMouse()
{
    show();
}

TMouse::~TMouse()
{
    hide();
}

#endif
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

#if defined(__NT__) || defined(__OS2__) || defined(__UNIX__)

THWMouse::THWMouse()
{
}

THWMouse::~THWMouse()
{
}

#ifndef __UNIX__
void THWMouse::suspend()
{
    if( present() == False ) return;
    hide();
    buttonCount = 0;
}
#endif

void THWMouse::setRange( ushort /* rx */, ushort /* ry */)
{
//  rx = rx;
//  ry = ry;
}

void THWMouse::getEvent( MouseEventType& me )
{
      me.buttons = 0;
      me.where.x = 0;
      me.where.y = 0;
      me.eventFlags = 0;
}

void THWMouse::registerHandler( unsigned /* mask */, void (* /*func*/)() )
{
   handlerInstalled = True;
//   mask = mask;
//   func = func;
}

TMouse::TMouse()
{
//    show();
}

TMouse::~TMouse()
{
//    hide();
}

#endif  // __NT__ || __OS2__ || __UNIX__

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

#ifdef __OS2__

void THWMouse::resume()
{
    if ( noMouse ) return;

    buttonCount = 2;
    USHORT bc;
    if ( TThreads::tiled->mouseHandle != 0xFFFF ) {
      assert( !MouGetNumButtons ((PUSHORT) &bc, TThreads::tiled->mouseHandle) );
      buttonCount = bc;
    } else
      buttonCount = 0;
}

void TV_CDECL THWMouse::show()
{
  if ( TThreads::tiled->mouseHandle != 0xFFFF )
    assert(! MouDrawPtr (TThreads::tiled->mouseHandle)  );
}

void TV_CDECL THWMouse::hide()
{
  if ( TThreads::tiled->mouseHandle != 0xFFFF ) {
    NOPTRRECT *ptrArea = &TThreads::tiled->ptrArea;
    ptrArea->row = ptrArea->col = 0;
    ptrArea->cRow = TScreen::screenHeight-1;
    ptrArea->cCol = TScreen::screenWidth-1;
    assert ( !MouRemovePtr (ptrArea, TThreads::tiled->mouseHandle) );
  }
}

void THWMouse::hide( const TRect& rect ) {
  if ( TThreads::tiled->mouseHandle != 0xFFFF ) {
    NOPTRRECT *ptrArea = &TThreads::tiled->ptrArea;
    ptrArea->row = rect.a.y;
    ptrArea->col = rect.a.x;
    ptrArea->cRow = rect.b.y-1;
    ptrArea->cCol = rect.b.x-1;
    assert ( !MouRemovePtr(ptrArea, TThreads::tiled->mouseHandle) );
  }
}

#endif  // __OS2__

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

#ifdef __NT__

void THWMouse::resume()
{
  if ( noMouse ) return;
  DWORD num;
  GetNumberOfConsoleMouseButtons(&num);
  buttonCount = (uchar)num;
}

inline void set_conin_mode(void)
{
  if ( TThreads::my_console )
    SetConsoleMode(TThreads::chandle[cnInput],TThreads::consoleMode);
}

void TV_CDECL THWMouse::show()
{
  TThreads::consoleMode |= ENABLE_MOUSE_INPUT;
  set_conin_mode();
}

void TV_CDECL THWMouse::hide()
{
  TThreads::consoleMode &= ~ENABLE_MOUSE_INPUT;
  set_conin_mode();
}

#endif  // __NT__
