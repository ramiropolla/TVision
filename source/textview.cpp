/*-------------------------------------------------------------*/
/* filename -       textview.cpp                               */
/*                                                             */
/* function(s)                                                 */
/*                  TTerminal and TTextDevice member functions */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*                                                             */
/*    Turbo Vision -  Version 1.0                              */
/*                                                             */
/*                                                             */
/*    Copyright (c) 1991 by Borland International              */
/*    All Rights Reserved.                                     */
/*                                                             */
/*-------------------------------------------------------------*/

#define Uses_TTextDevice
#define Uses_TTerminal
#define Uses_otstream
#include <tv.h>

TTextDevice::TTextDevice( const TRect& bounds,
                          TScrollBar *aHScrollBar,
                          TScrollBar *aVScrollBar) :
    TScroller(bounds,aHScrollBar,aVScrollBar)
{
}

#if defined(__OS2__) && defined(__BORLANDC__)
int _RTLENTRY TTextDevice::overflow( int c )
#else
int TV_CDECL TTextDevice::overflow( int c )
#endif
{
    if( c != EOF )
        {
        char b = (char)c;
        do_sputn( &b, 1 );
        }
    return 1;
}

TTerminal::TTerminal( const TRect& bounds,
                      TScrollBar *aHScrollBar,
                      TScrollBar *aVScrollBar,
                      size_t aBufSize ) :
    TTextDevice(bounds, aHScrollBar, aVScrollBar),
    queFront( 0 ),
    queBack( 0 )
{
    growMode = gfGrowHiX + gfGrowHiY;
    bufSize = qmin( 32000U, aBufSize );
    buffer = new char[ bufSize ];
    setLimit( 0, 1 );
    setCursor( 0, 0 );
    showCursor();
}


TTerminal::~TTerminal()
{
    delete[] buffer;
}

void TTerminal::bufDec( size_t& val )
{
    if (val == 0)
        val = bufSize - 1;
    else
        val--;
}

void TTerminal::bufInc( size_t& val )
{
    if( ++val >= bufSize )
        val = 0;
}

Boolean TTerminal::canInsert( size_t amount )
{
    ssize_t T = (queFront < queBack) ?
        ssize_t( queFront +  amount ) :
        ssize_t( int32(queFront) - bufSize + amount);   // cast needed so we get
                                                // signed comparison
    return Boolean( int(queBack) > T );
}

void TTerminal::draw()
{
    ssize_t  i;
    size_t begLine, endLine;
    char s[maxViewWidth+1];
    uint32 bottomLine;

    bottomLine = uint32(size.y + delta.y);
    if( (uint32)limit.y > bottomLine )
        {
        endLine = prevLines( queFront, limit.y - bottomLine );
        bufDec( endLine );
        }
    else
        endLine = queFront;

    if( limit.y > size.y )
        i = size.y - 1;
    else
        {
        for( i = limit.y; i <= size.y - 1; i++ )
            writeChar(0, ushort(i), ' ', 1, ushort(size.x));
        i =  limit.y -  1;
        }

    for( ; i >= 0; i-- )
        {
        begLine = prevLines(endLine, 1);
        if (endLine >= begLine) {
          int T = int( endLine - begLine );
          T = qmin(T, sizeof(s)-2); // bugfix JS 26.11.94
          memcpy( s, &buffer[begLine], T );
          s[T] = EOS;
        } else {
          ssize_t T = ssize_t(bufSize - begLine);
          if ( T > (ssize_t)sizeof(s)-2) {         // bugfix JS 26.11.94
            memcpy( s, &buffer[begLine], sizeof(s)-2 );
            s[sizeof(s)-2] = EOS;
          } else {
            memcpy( s, &buffer[begLine], T );
            if (T+endLine>sizeof(s)-2) {
              memcpy( s+T, buffer, sizeof(s)-2-T );
              s[sizeof(s)-2] = EOS;
            } else {
              memcpy( s+T, buffer, endLine );
              s[T+endLine] = EOS;
            }
          }
        }
        if( delta.x >= (int)strlen(s) )
            *s = EOS;
        else if ( delta.x != 0 )
            qstrncpy( s, &s[delta.x], sizeof(s) );

        s[maxViewWidth-1] = EOS;
        writeStr( 0, ushort(i), s, 1 );
        const size_t sl=strlen(s);
        if (sl < (size_t)size.x) // bugfix JS
          writeChar( ushort(sl), ushort(i), ' ', 1, /*size.x*/ ushort(size.x-sl)  );
        endLine = begLine;
        bufDec( endLine );
        }
}

size_t TTerminal::nextLine( size_t pos )
{
    if( pos != queFront )
        {
        while( buffer[pos] != '\n' && pos != queFront )
            bufInc(pos);
        if( pos != queFront )
            bufInc( pos );
        }
    return pos;
}

int TTerminal::do_sputn( const char *s, int count )
{
    size_t screenLines = limit.y;
    size_t i;
    for( i = 0; i < (size_t)count; i++ )
        if( s[i] == '\n' )
            screenLines++;

    while( !canInsert( count ) )
        {
        queBack = nextLine( queBack );
        screenLines--;
        }

    if( queFront + count >= bufSize )
        {
        i = bufSize - queFront;
        memcpy( &buffer[queFront], s, i );
        memcpy( buffer, &s[i], count - i );
        queFront = count - i;
        }
    else
        {
        memcpy( &buffer[queFront], s, count );
        queFront += count;
        }

    setLimit( limit.x, ushort(screenLines) );
    scrollTo( 0, ushort(screenLines + 1) );
    i = prevLines( queFront, 1 );
    if( i <= queFront )
        i = queFront - i;
    else
        i = bufSize - (i - queFront);
    setCursor( ushort(i), ushort(screenLines - delta.y - 1) );
    drawView();
    return count;
}

Boolean TTerminal::queEmpty()
{
    return Boolean( queBack == queFront );
}

#if !defined(NO_TV_STREAMS)
otstream::otstream( TTerminal *tt)
{
    ios::init( tt );
}
#else
otstream& otstream::operator<<(char const *str)
{

  if ( str != NULL ) tty->do_sputn(str, (uint32)strlen(str));
  return(*this);
}

otstream& otstream::operator<<(char c)
{
  tty->do_sputn(&c, 1);
  return(*this);
}
#endif
