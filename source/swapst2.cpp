#define Uses_TSystemError
#define Uses_TScreen
#include <tv.h>

#pragma argsused
#pragma off(unreferenced)

void TV_CDECL TSystemError::swapStatusLine( TDrawBuffer& tb ) {
  ushort *scr = (ushort *)(TScreen::screenBuffer + TScreen::screenWidth * (TScreen::screenHeight-1));
  ushort *buf = (ushort *)&tb;
  for ( int i=0; i < TScreen::screenWidth; i++ ) {
    ushort x = *scr;
    *scr++ = *buf;
    *buf++ = x;
  }
}
