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
#if defined(__NT__) || defined(__X11__) 
#define Uses_TThreaded
#else
#define Uses_TEditor
#endif
#include <tv.h>

const int CONTROL_Y = 25;

char hotKey( const char *s )
{
    const char *p;

    if( (p = strchr( s, '~' )) != 0 )
        return (char)toupper(p[1]);
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

    uchar color = uchar((state & sfFocused) ? getColor( 2 ) : getColor( 1 ));

    b.moveChar( 0, ' ', color, ushort(size.x) );
    char buf[MAXSTR];
    qstrncpy( buf, data+firstPos, size.x - 1 );
    b.moveStr( 1, buf, color );

    if( canScroll(1) )
        b.moveChar( ushort(size.x-1), rightArrow, getColor(4), 1 );
    if( (state & sfSelected) != 0 )
        {
        if( canScroll(-1) )
            b.moveChar( 0, leftArrow, getColor(4), 1 );
        l = selStart - firstPos;
        r = selEnd - firstPos;
        l = qmax( 0, l );
        r = qmin( size.x - 2, r );
        if (l <  r)
            b.moveChar( ushort(l+1), 0, getColor(3), ushort(r - l) );
        }
    writeLine( 0, 0, ushort(size.x), ushort(size.y), b );
    setCursor( ushort(curPos-firstPos+1), 0);
}

void TInputLine::getData( void *rec, size_t recsize )
{
    memcpy( rec, data, qmin(recsize, dataSize()) );
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
    mouse.x = qmax( mouse.x, 1 );
    size_t pos = mouse.x + firstPos - 1;
    pos = qmax( pos, 0 );
    pos = qmin( pos, strlen(data) );
    return (int)pos;
}

void  TInputLine::deleteSelect()
{
    if( selStart < selEnd )
        {
        qstrncpy( data+selStart, data+selEnd, maxLen+1-selStart );
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

bool TInputLine::clip_put(void)
{
#if defined(__NT__) || defined(__X11__) 
    return TThreads::clipboard_put(data, selStart, selEnd);
#else
    if ( TEditor::clipboard == NULL )
        return false;
    return TEditor::clipboard->insertBuffer(data, selStart, selEnd,
                                            False, True) == True;
#endif
}

char *TInputLine::clip_get(size_t &clipsz)
{
#if defined(__NT__) || defined(__X11__)
    return TThreads::clipboard_get(clipsz, true);
#else
    char *answer = NULL;
    if ( TEditor::clipboard != NULL )
        {
        char *ptr = TEditor::clipboard->buffer;
        char *end = ptr + TEditor::clipboard->selEnd;
        ptr += TEditor::clipboard->selStart;
        if ( end > ptr )
            {
            size_t  tot = end - ptr;
            if ( tot > clipsz )
                tot = clipsz;
            answer = (char *)malloc(tot + 1);
            if ( answer != NULL )
                {
                char *p = answer;
                while ( ptr < end && *ptr != 0 )  // second - paranoya
                    {
                    *p = *ptr++;
                    if ( *p < ' ' || *p == 127 )
                        {
                        if ( p == answer ) continue;
                        *p = ' ';
                        }
                        ++p;
                    }
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
                        clipsz = p - answer;
                    }
                }
            }
        }
    return answer;
#endif
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
                        curPos = (int)strlen(data);
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
                    case kbShiftIns:
                        {
                        size_t len = strlen(data);
                        if ( selEnd > selStart )
                              len -= (selEnd - selStart);
                        //              signed comparision (see above)
                        if ( (int)len < 0 || (int)len >= maxLen )
                            break;
                        size_t clipsz = maxLen - len;
                        char *pcl = clip_get(clipsz);
                        if ( pcl == NULL )
                            break;
                        deleteSelect();
                        if( firstPos > curPos )
                            firstPos = curPos;
                        memmove(data+curPos+clipsz, data+curPos,
                                strlen(data+curPos) + 1);
                        memcpy(data+curPos, pcl, clipsz);
                        free(pcl);
                        }
                        break;
                    case kbCtrlIns:
                    case kbShiftDel:
                        if (   !clip_put()
                            || event.keyDown.keyCode == kbCtrlIns )
                            {
                              clearEvent ( event );
                              return; // do not remove selection
                            }
                        deleteSelect();
                        break;
                    default:
                        if( event.keyDown.charScan.charCode >= ' ' )
                            {
                            deleteSelect();
                            if( (state & sfCursorIns) != 0 )
                                /* The following must be a signed comparison! */
                                if( curPos < (int) strlen(data) )
                                    qstrncpy( data + curPos, data + curPos + 1, maxLen+1-curPos );

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

        event.keyDown.keyCode = (ushort)oldKeyCode;
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

bool TInputLine::disableReselect = false; // for 'ovelapped' wait_boxes
void TInputLine::selectAll( Boolean enable )
{
    if( !disableReselect )
    {
        selStart = 0;
        if( enable )
            curPos = selEnd = (int)strlen(data);
        else
            curPos = selEnd = 0;
        firstPos = qmax( 0, curPos-size.x+2 );
#ifndef __UNPATCHED
        anchor = 0;               //<----- This sets anchor to avoid deselect
#endif
    }
    drawView();
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


