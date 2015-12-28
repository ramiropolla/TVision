/*------------------------------------------------------------*/
/* filename -       tinputli.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TInputLine member functions               */
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
#define Uses_TInputLine
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_opstream
#define Uses_ipstream
#include <tv.h>

const int CONTROL_Y = 25;

char hotKey( const char *s )
{
    const char *p;

    if( (p = strchr( s, '~' )) != 0 )
        return toupper(p[1]);
    else
        return 0;
}

#define cpInputLine "\x13\x13\x14\x15"

TInputLine::TInputLine( const TRect& bounds, int aMaxLen ) :
    TView(bounds),
    data( new char[aMaxLen] ),
    maxLen( aMaxLen-1 ),
    curPos( 0 ),
    firstPos( 0 ),
    selStart( 0 ),
    selEnd( 0 ),
    anchor( -1 )
{
    state |= sfCursorVis;
    options |= ofSelectable | ofFirstClick;
    *data = EOS;
}

TInputLine::~TInputLine()
{
    delete[] data;
}

Boolean TInputLine::canScroll( int delta )
{
    if( delta < 0 )
        return Boolean( firstPos > 0 );
    else
        if( delta > 0 )
            return Boolean( strlen(data) - firstPos + 2 > (size_t)size.x );
        else
            return False;
}

size_t TInputLine::dataSize()
{
    return maxLen+1;
}

void TInputLine::draw()
{
    int l, r;
    TDrawBuffer b;

    uchar color = (state & sfFocused) ? getColor( 2 ) : getColor( 1 );

    b.moveChar( 0, ' ', color, size.x );
    char buf[MAXSTR];
    strncpy( buf, data+firstPos, size.x - 2 );
    buf[size.x - 2 ] = EOS;
    b.moveStr( 1, buf, color );

    if( canScroll(1) )
        b.moveChar( size.x-1, rightArrow, getColor(4), 1 );
    if( (state & sfSelected) != 0 )
        {
        if( canScroll(-1) )
            b.moveChar( 0, leftArrow, getColor(4), 1 );
        l = selStart - firstPos;
        r = selEnd - firstPos;
        l = max( 0, l );
        r = min( size.x - 2, r );
        if (l <  r)
            b.moveChar( l+1, 0, getColor(3), r - l );
        }
    writeLine( 0, 0, size.x, size.y, b );
    setCursor( curPos-firstPos+1, 0);
}

void TInputLine::getData( void *rec )
{
    memcpy( rec, data, dataSize() );
}

TPalette& TInputLine::getPalette() const
{
    static TPalette palette( cpInputLine, sizeof( cpInputLine )-1 );
    return palette;
}

int TInputLine::mouseDelta( TEvent& event )
{
    TPoint mouse = makeLocal( event.mouse.where );

    if( mouse.x <= 0 )
        return -1;
    else
        if( mouse.x >= size.x - 1 )
            return 1;
        else
            return 0;
}

int TInputLine::mousePos( TEvent& event )
{
    TPoint mouse = makeLocal( event.mouse.where );
    mouse.x = max( mouse.x, 1 );
    int pos = mouse.x + firstPos - 1;
    pos = max( pos, 0 );
    pos = min( pos, strlen(data) );
    return pos;
}

void  TInputLine::deleteSelect()
{
    if( selStart < selEnd )
        {
        strcpy( data+selStart, data+selEnd );
        curPos = selStart;
        }
}

void TInputLine::adjustSelectBlock()
{
#ifndef __UNPATCHED
    if(anchor < 0)
        selEnd = selStart = 0;
    else
#endif
    if (curPos < anchor)
        {
        selStart = curPos;
        selEnd =  anchor;
        }
    else
        {
        selStart = anchor;
        selEnd = curPos;
        }
}

void  TInputLine::handleEvent( TEvent& event )
{
    /* Home, Left Arrow, Right Arrow, End, Ctrl-Left Arrow, Ctrl-Right Arrow */
    static char padKeys[] = {0x47,0x4b,0x4d,0x4f,0x73,0x74, 0};
    TView::handleEvent(event);

    int delta, i;
    if( (state & sfSelected) != 0 )
        switch( event.what )
            {
            case  evMouseDown:
                if( canScroll(delta = mouseDelta(event)) )
                    do  {
                        if( canScroll(delta) )
                            {
                            firstPos += delta;
                            drawView();
                            }
                        } while( mouseEvent( event, evMouseAuto ) );
                else if (event.mouse.eventFlags & meDoubleClick)
                    selectAll(True);
                else
                    {
                    anchor =  mousePos(event);
                    do  {
                        if( event.what == evMouseAuto)
                            {
                            delta = mouseDelta(event);
                            if (canScroll(delta))
                                firstPos += delta;
                            }
                        curPos = mousePos(event);
                        adjustSelectBlock();
                        drawView();
                        }
                        while (mouseEvent(event,evMouseMove | evMouseAuto));
                    }
                clearEvent(event);
                break;
            case  evKeyDown:
		int oldKeyCode = event.keyDown.keyCode;
                event.keyDown.keyCode = ctrlToArrow(event.keyDown.keyCode);

		/* SS: scanCode must be non zero */

		if (event.keyDown.charScan.scanCode != 0 &&
		    strchr(padKeys, event.keyDown.charScan.scanCode ) &&
                    (event.keyDown.controlKeyState & kbShift) != 0
                  )
                    {
                    event.keyDown.charScan.charCode = 0;
#ifndef __UNPATCHED
                    if(anchor < 0)
                        anchor = curPos;
                }
                else
		    anchor = -1;
#else
                    if (curPos == selEnd)
                        anchor = selStart;
                    else
                        anchor = selEnd;
                    extendBlock = True;
                    }
                else
                    extendBlock = False;
#endif
                switch( event.keyDown.keyCode )
                    {
                    case kbLeft:
                        if( curPos > 0 )
                            curPos--;
                        break;
                    case kbRight:
                        if( curPos < (int)strlen(data) )
                            curPos++;
                        break;
                    case kbHome:
                        curPos =  0;
                        break;
                    case kbEnd:
                        curPos = strlen(data);
                        break;
                    case kbBack:
                        if( curPos > 0 )
                        {
                          char *ptr = data + curPos;
                          memmove(ptr-1, ptr, strlen(ptr)+1);
                          curPos--;
                          if( firstPos > 0 )
                            firstPos--;
                        }
                        break;
                    case kbDel:
                        if( selStart == selEnd )
                            if( curPos < (int)strlen(data) )
                                {
                                selStart = curPos;
                                selEnd = curPos + 1;
                                }
                        deleteSelect();
                        break;
                    case kbIns:
                        setState(sfCursorIns, Boolean(!(state & sfCursorIns)));
                        break;
                    default:
                        if( event.keyDown.charScan.charCode >= ' ' )
                            {
                            deleteSelect();
                            if( (state & sfCursorIns) != 0 )
                                /* The following must be a signed comparison! */
                                if( curPos < (int) strlen(data) )
                                    strcpy( data + curPos, data + curPos + 1 );

                                if( (int)strlen(data) < maxLen )
                                    {
                                    if( firstPos > curPos )
                                        firstPos = curPos;
                                    memmove( data+curPos+1, data+curPos, strlen(data+curPos)+1 );
                                    data[curPos++] = event.keyDown.charScan.charCode;
                                    }
                            }
                        else if( event.keyDown.charScan.charCode == CONTROL_Y)
                            {
                            *data = EOS;
                            curPos = 0;
                            }
                            else
			    {
				/* SS: restore the old value before exit */

				event.keyDown.keyCode = oldKeyCode;
				return;
			    }
                    }
#ifndef __UNPATCHED
		adjustSelectBlock();
#else
                if (extendBlock)
                    adjustSelectBlock();
                else
                    {
                    selStart = 0;
                    selEnd = 0;
                    }
#endif
                    if( firstPos > curPos )
                        firstPos = curPos;
                    i = curPos - size.x + 2;
                    if( firstPos < i )
                        firstPos = i;
                    drawView();
                    clearEvent( event );
                    break;
            }
}

void TInputLine::selectAll( Boolean enable )
{
    selStart = 0;
    if( enable )
        curPos = selEnd = strlen(data);
    else
        curPos = selEnd = 0;
    firstPos = max( 0, curPos-size.x+2 );
#ifndef __UNPATCHED
    anchor = 0;                   //<----- This sets anchor to avoid deselect
    drawView();                   //       on initial selection
#else
    drawView();
#endif
}

void TInputLine::setData( void *rec )
{
    memcpy( data, rec, dataSize()-1 );
    data[dataSize()-1] = EOS;
    selectAll( True );
}

void TInputLine::setState( ushort aState, Boolean enable )
{
    TView::setState( aState, enable );
    if( aState == sfSelected ||
        ( aState == sfActive && (state & sfSelected) != 0 )
      )
        selectAll( enable );
}

#ifndef NO_TV_STREAMS
void TInputLine::write( opstream& os )
{
    TView::write( os );
    os << maxLen << curPos << firstPos
       << selStart << selEnd;
    os.writeString( data);
}

void *TInputLine::read( ipstream& is )
{
    TView::read( is );
    is >> maxLen >> curPos >> firstPos
       >> selStart >> selEnd;
    data = new char[maxLen + 1];
    is.readString(data, maxLen+1);
    state |= sfCursorVis;
    options |= ofSelectable | ofFirstClick;
    return this;
}

TStreamable *TInputLine::build()
{
    return new TInputLine( streamableInit );
}

TInputLine::TInputLine( StreamableInit ) : TView( streamableInit )
{
}
#endif  // ifndef NO_TV_STREAMS


