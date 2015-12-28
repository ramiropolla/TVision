/*------------------------------------------------------------*/
/* filename -       tlstview.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TListViewer member functions              */
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
#define Uses_TListViewer
#define Uses_TScrollBar
#define Uses_TDrawBuffer
#define Uses_TPoint
#define Uses_TEvent
#define Uses_TGroup
#define Uses_opstream
#define Uses_ipstream
#include <tv.h>

#define cpListViewer "\x1A\x1A\x1B\x1C\x1D"

TListViewer::TListViewer( const TRect& bounds,
                          ushort aNumCols,
                          TScrollBar *aHScrollBar,
                          TScrollBar *aVScrollBar) :
    TView( bounds ),
    numCols( aNumCols ),
    topItem( 0 ),
    focused( 0 ),
    range( 0 ),
    first_selected( -1 ),
    last_selected( -1 )
{
    int arStep, pgStep;

    options |= ofFirstClick | ofSelectable;
    eventMask |= evBroadcast;
    if( aVScrollBar != 0 )
        {
        if( numCols == 1 )
            {
            pgStep = size.y - 1;
            arStep = 1;
            }
        else
            {
            pgStep = size.y * numCols;
            arStep = size.y;
            }
        aVScrollBar->setStep( pgStep, arStep );
        }

    if( aHScrollBar != 0 )
        aHScrollBar->setParams( 0,0,size.x-1,8,1);      // ig 17.03.94

    hScrollBar = aHScrollBar;
    vScrollBar = aVScrollBar;
}

void TListViewer::changeBounds( const TRect& bounds )
{
    TView::changeBounds( bounds );
//    if( hScrollBar != 0 )                             // ig 17.03.94
//        hScrollBar->setStep( size.x / numCols, 1 );
}

void TListViewer::draw()
{
    int i, j, item;
    ushort normalColor, selectedColor, color;
    short colWidth, curCol, indent;
    TDrawBuffer b;
    uchar scOff;
    bool  on_front;

    on_front = (state&(sfSelected|sfActive)) == (sfSelected|sfActive);
    normalColor = getColor(on_front ? 1 : 2);
    selectedColor = getColor(4);

    indent = 0;
    if( hScrollBar != 0 )
        indent = hScrollBar->value;

    colWidth = size.x / numCols + 1;
    for( i = 0; i < size.y; i++ )
        {
        for( j = 0; j < numCols; j++ )
            {
            item =  j * size.y + i + topItem;
            curCol = j * colWidth;

            if( focused == item && on_front && range > 0 )
                {
                color = getColor(3);
                setCursor( curCol + 1, i );
                scOff = 0;
                }
            else if( item < range && isItemSelected(item) )
                {
                color = selectedColor;
                scOff = 2;
                }
            else
                {
                color = normalColor;
                scOff = 4;
                }

            b.moveChar( curCol, ' ', color, colWidth );
            if( item < range )
                {
                char text[MAXSTR];
                getText( text, item, qmin(colWidth + indent, sizeof(text)) );
                char buf[MAXSTR];
                buf[0] = '\0';
                if ( strlen(text) > (size_t)indent ) {
                  memcpy( buf, text+indent, colWidth );
                  buf[colWidth] = EOS;
                }
                b.moveStr( curCol+1, buf, color );
                if( showMarkers )
                    {
                    b.putChar( curCol, specialChars[scOff] );
                    b.putChar( curCol+colWidth-2, specialChars[scOff+1] );
                    }
                }
            else if( i == 0 && j == 0 )
                b.moveStr( curCol+1, "<empty>", getColor(1) );

            b.moveChar( curCol+colWidth-1, char(179), getColor(5), 1 );
            }
        writeLine( 0, i, size.x, 1, b );
        }
}

void TListViewer::focusItem( int item )
{
    focused = item;
    if( vScrollBar != 0 )
        vScrollBar->setValue( item );
    else drawView();
    if( item < topItem )
        if( numCols == 1 )
            topItem = item;
        else
            topItem = item - item % size.y;
    else
        if( item >= topItem + size.y*numCols )
            if( numCols == 1 )
                topItem = item - size.y + 1;
            else
                topItem = item - item % size.y - (size.y * (numCols-1));
}

void TListViewer::focusItemNum( int item )
{
    if( item < 0 )
        item = 0;
    else
        if( item >= range && range > 0 )
            item = range - 1;

    if( range !=  0 )
        focusItem( item );
}

TPalette& TListViewer::getPalette() const
{
    static TPalette palette( cpListViewer, sizeof( cpListViewer )-1 );
    return palette;
}

void TListViewer::getText( char *dest, int, size_t destsize )
{
  if ( ssize_t(destsize) > 0 )
    *dest = EOS;
}

Boolean TListViewer::isSelected( int item )
{
    return Boolean( item == focused );
}

bool TListViewer::isItemSelected ( int item )
{
    if ( (first_selected | last_selected) < 0 ) return isSelected(item);

    if ( first_selected > last_selected )
      return Boolean( last_selected <= item && item <= first_selected );
    return Boolean( first_selected <= item && item <= last_selected );
}

void TListViewer::handleEvent( TEvent& event )
{
    TPoint mouse;
    ushort colWidth;
    int oldItem, newItem;
    int count;
    int mouseAutosToSkip = 4;

    TView::handleEvent(event);

    if( event.what == evMouseDown )
        {
        remove_selection();
        colWidth = size.x / numCols + 1;
        oldItem =  focused;
        mouse = makeLocal( event.mouse.where );
        if (mouseInView(event.mouse.where))
            newItem = mouse.y + (size.y * (mouse.x / colWidth)) + topItem;
        else
            newItem = oldItem;
        count = 0;
        do  {
            if( newItem != oldItem )
                {
                focusItemNum( newItem );
                drawView();
                }
            oldItem = newItem;
            mouse = makeLocal( event.mouse.where );
            if( mouseInView( event.mouse.where ) )
                newItem = mouse.y + (size.y * (mouse.x / colWidth)) + topItem;
            else
                {
                if( numCols == 1 )
                    {
                    if( event.what == evMouseAuto )
                        count++;
                    if( count == mouseAutosToSkip )
                        {
                        count = 0;
                        if( mouse.y < 0 )
                            newItem = focused - 1;
                        else if( mouse.y >= size.y )
                                newItem = focused + 1;
                        }
                    }
                else
                    {
                    if( event.what == evMouseAuto )
                        count++;
                    if( count == mouseAutosToSkip )
                        {
                        count = 0;
                        if( mouse.x < 0 )
                            newItem = focused - size.y;
                        else if( mouse.x >= size.x )
                            newItem = focused + size.y;
                        else if( mouse.y < 0 )
                            newItem = focused - focused % size.y;
                        else if( mouse.y > size.y )
                            newItem = focused - focused % size.y + size.y - 1;
                        }
                    }
                }
                if( event.mouse.eventFlags & meDoubleClick )
                    break;
            } while( mouseEvent( event, evMouseMove | evMouseAuto ) );
        focusItemNum( newItem );
        drawView();
        if( (event.mouse.eventFlags & meDoubleClick) && range > newItem )
            selectItem( newItem );
        clearEvent( event );
        }
    else if( event.what == evKeyDown )
        {
        if (event.keyDown.charScan.charCode ==  ' ' && focused < range )
            {
            selectItem( focused );
            newItem = focused;
            first_selected = -1;  // remove multi-selection
            }
        else
            {
            switch (ctrlToArrow(event.keyDown.keyCode))
                {
                case kbUp:
                    newItem = focused - 1;
                    break;
                case kbDown:
                    newItem = focused + 1;
                    break;
                case kbRight:
                    if( numCols > 1 )
                        newItem = focused + size.y;
                    else
                        return;
                    break;
                case kbLeft:
                    if( numCols > 1 )
                        newItem = focused - size.y;
                    else
                        return;
                    break;
                case kbPgDn:
                    newItem = focused + size.y * numCols;
                    break;
                case  kbPgUp:
                    newItem = focused - size.y * numCols;
                    break;
                case kbHome:
                    newItem = topItem;
                    break;
                case kbEnd:
                    newItem = topItem + (size.y * numCols) - 1;
                    break;
                case kbCtrlEnd:
                case kbCtrlPgDn:
                    newItem = range - 1;
                    break;
                case kbCtrlHome:
                case kbCtrlPgUp:
                    newItem = 0;
                    break;
                default:
                    remove_selection();
                    return;
                }
            // multi-selection
            if ( last_selected >= 0 )
                {
                if (event.keyDown.controlKeyState & kbShift)
                    {
                    if ( newItem < 0 )
                        newItem = 0;
                    else if ( range > 0 && newItem >= range )
                        newItem = range-1;
                    if ( focused < 0 )
                        focused = 0;
                    else if ( range >= 0 && focused >= range )
                        focused = range-1;

                    last_selected = newItem;
                    if ( first_selected < 0 )
                        first_selected = focused;
                    if ( first_selected == last_selected )
                        first_selected = -1;
                    }
                    else  // unshifted go
                        first_selected = -1;  // remove selection
                }
            }
        focusItemNum(newItem);
        drawView();
        clearEvent(event);
        }
    else if( event.what == evBroadcast )
        {
        if( (options & ofSelectable) != 0 )
            {
            if( event.message.command == cmScrollBarClicked &&
                  ( event.message.infoPtr == hScrollBar ||
                    event.message.infoPtr == vScrollBar ) )
                select();
            else if( event.message.command == cmScrollBarChanged )
                {
                if( vScrollBar == event.message.infoPtr )
                    {
                    focusItemNum( vScrollBar->value );
                    drawView();
                    }
                else if( hScrollBar == event.message.infoPtr )
                    drawView();
                }
            }
        }
}

void TListViewer::selectItem( int )
{
    message( owner, evBroadcast, cmListItemSelected, this );
}

void TListViewer::setRange( int aRange )
{
    range = aRange;
    if( focused > aRange ) focused = 0;         /* ig 16.03.94 */
    if( vScrollBar != 0 )
        vScrollBar->setParams( focused,
                               0,
                               aRange - 1,
                               vScrollBar->pgStep,
                               vScrollBar->arStep
                             );
    else drawView();
}

void TListViewer::setState( ushort aState, Boolean enable )
{
    TView::setState( aState, enable );
    if( (aState & (sfSelected | sfActive)) != 0 )
        {
        if( hScrollBar != 0 )
            if( getState(sfActive) )
                hScrollBar->show();
            else
                hScrollBar->hide();
        if( vScrollBar != 0 )
            if( getState(sfActive) )
                vScrollBar->show();
            else
                vScrollBar->hide();
        drawView();
        }
}

void TListViewer::shutDown()
{
     hScrollBar = 0;
     vScrollBar = 0;
     TView::shutDown();
}

#ifndef NO_TV_STREAMS
void TListViewer::write( opstream& os )
{
    TView::write( os );
    os << hScrollBar << vScrollBar << numCols
       << long(topItem) << long(focused) << long(range);
}

void *TListViewer::read( ipstream& is )
{
    TView::read( is );
    is >> hScrollBar >> vScrollBar >> numCols;
    long z;
    is >> z; topItem = z;
    is >> z; focused = z;
    is >> z; range   = z;
    return this;
}

TStreamable *TListViewer::build()
{
    return new TListViewer( streamableInit );
}

TListViewer::TListViewer( StreamableInit ) : TView( streamableInit )
{
}
#endif  // ifndef NO_TV_STREAMS


