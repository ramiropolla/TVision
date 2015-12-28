/*------------------------------------------------------------*/
/* filename -       thstview.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  THistoryViewer member functions           */
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

#define Uses_TKeys
#define Uses_THistoryViewer
#define Uses_TScrollBar
#define Uses_TEvent
#include <tv.h>

#define cpHistoryViewer "\x06\x06\x07\x06\x06"

THistoryViewer::THistoryViewer( const TRect& bounds,
                                TScrollBar *aHScrollBar,
                                TScrollBar *aVScrollBar,
                                ushort aHistoryId) :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar),
    historyId( aHistoryId )
{
    setRange( historyCount( (uchar)aHistoryId ) );
//    if( range > 1 )
//        focusItem( 1 );
    hScrollBar->setRange( 0, historyWidth() - size.x + 3 );
}

TPalette& THistoryViewer::getPalette() const
{
    static TPalette palette( cpHistoryViewer, sizeof( cpHistoryViewer )-1 );
    return palette;
}

void THistoryViewer::getText( char *dest, int item, size_t destsize )
{
  if ( ssize_t(destsize) > 0 )
  {
    const char *str = historyStr( (uchar)historyId, item );
    if( str != NULL )
      qstrncpy( dest, str, destsize );
    else
      *dest = EOS;
  }
}

void THistoryViewer::handleEvent( TEvent& event )
{
    if( (event.what == evMouseDown && (event.mouse.eventFlags & meDoubleClick) ) ||
        (event.what == evKeyDown && event.keyDown.keyCode == kbEnter)
      )
        {
        endModal( cmOK );
        clearEvent( event );
        }
    else
        if( (event.what ==  evKeyDown && event.keyDown.keyCode == kbEsc) ||
            (event.what ==  evCommand && event.message.command ==  cmCancel)
          )
            {
            endModal( cmCancel );
            clearEvent( event );
            }
        else
            TListViewer::handleEvent( event );
}

int THistoryViewer::historyWidth()
{
    size_t width = 0;
    int count = historyCount( (uchar)historyId );
    for( int i = 0; i < count; i++ )
        {
        size_t T = strlen( historyStr( (uchar)historyId, i ) );
        width = qmax( width, T );
        }
    return (int)width;
}
