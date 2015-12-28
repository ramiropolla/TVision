/*------------------------------------------------------------*/
/* filename - teditor1.cpp                                    */
/*                                                            */
/* function(s)                                                */
/*            TEditor member functions                        */
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
#define Uses_TEditor
#define Uses_TIndicator
#define Uses_TEvent
#define Uses_TScrollBar
#define Uses_TFindDialogRec
#define Uses_TReplaceDialogRec
#define Uses_opstream
#define Uses_ipstream
#include <tv.h>

inline bool isWordChar( int ch )
{
    return isalnum(ch) || ch == '_';
}

static const ushort firstKeys[] =
{
    37,
    kbCtrlA, cmWordLeft,
    kbCtrlC, cmPageDown,
    kbCtrlD, cmCharRight,
    kbCtrlE, cmLineUp,
    kbCtrlF, cmWordRight,
    kbCtrlG, cmDelChar,
    kbCtrlH, cmBackSpace,
    kbCtrlK, 0xFF02,
    kbCtrlL, cmSearchAgain,
    kbCtrlM, cmNewLine,
    kbCtrlO, cmIndentMode,
    kbCtrlQ, 0xFF01,
    kbCtrlR, cmPageUp,
    kbCtrlS, cmCharLeft,
    kbCtrlT, cmDelWord,
    kbCtrlU, cmUndo,
    kbCtrlV, cmInsMode,
    kbCtrlX, cmLineDown,
    kbCtrlY, cmDelLine,
    kbLeft, cmCharLeft,
    kbRight, cmCharRight,
    kbCtrlLeft, cmWordLeft,
    kbCtrlRight, cmWordRight,
    kbHome, cmLineStart,
    kbEnd, cmLineEnd,
    kbUp, cmLineUp,
    kbDown, cmLineDown,
    kbPgUp, cmPageUp,
    kbPgDn, cmPageDown,
    kbCtrlPgUp, cmTextStart,
    kbCtrlPgDn, cmTextEnd,
    kbIns, cmInsMode,
    kbDel, cmDelChar,
    kbShiftIns, cmPaste,
    kbShiftDel, cmCut,
    kbCtrlIns, cmCopy,
    kbCtrlDel, cmClear
};

static const ushort quickKeys[] =
{   8,
    'A', cmReplace,
    'C', cmTextEnd,
    'D', cmLineEnd,
    'F', cmFind,
    'H', cmDelStart,
    'R', cmTextStart,
    'S', cmLineStart,
    'Y', cmDelEnd
};

static const ushort blockKeys[] =
{   5,
    'B', cmStartSelect,
    'C', cmPaste,
    'H', cmHideSelect,
    'K', cmCopy,
    'Y', cmCut
};

static const ushort *keyMap[] = { firstKeys, quickKeys, blockKeys };

ushort defEditorDialog( int, ... );

ushort scanKeyMap( const void *keyMap, int keyCode )
{
     ushort * map = (ushort*) keyMap;
     for (int count=0; count< map[0]; count++) {
       int i=count*2+1;
       if ((keyCode & 0xFF)==(map[i] & 0xFF) &&
           ((map[i] & 0xFF00)==0 || (keyCode & 0xFF00)==(map[i] & 0xFF00)) )
         return map[i+1];
     }
     return 0;
}

#define cpEditor    "\x06\x07"

TEditor::TEditor( const TRect& bounds,
                  TScrollBar *aHScrollBar,
                  TScrollBar *aVScrollBar,
                  TIndicator *aIndicator,
                  size_t aBufSize ) :
    TView( bounds ),
    hScrollBar( aHScrollBar ),
    vScrollBar( aVScrollBar ),
    indicator( aIndicator ),
    bufSize( aBufSize ),
    canUndo( True ),
    selecting( False ),
    overwrite( False ),
    autoIndent( False ) ,
    lockCount( 0 ),
    updateFlags( 0 ),
    keyState( 0 )
{
    growMode = gfGrowHiX | gfGrowHiY;
    options |= ofSelectable;
    eventMask = evMouseDown | evKeyDown | evCommand | evBroadcast;
    showCursor();
    initBuffer();
    if( bufSize == 0 || buffer != 0 )
        isValid = True;
    else
    {
        editorDialog( edOutOfMemory );
        bufSize = 0;
        isValid = False;
    }
    setBufLen(0);
}

TEditor::~TEditor()
{
}

void TEditor::shutDown()
{
    doneBuffer();
    TView::shutDown();
}

void TEditor::changeBounds( const TRect& bounds )
{
    setBounds(bounds);
    delta.x = max(0, min(delta.x, limit.x - size.x));
    delta.y = max(0, min(delta.y, limit.y - size.y));
    update(ufView);
}

int TEditor::charPos( size_t p, size_t target )
{
    int pos = 0;
    while( p < target )
    {
        if( bufChar(p) == '\x9' )
            pos |= 7;
        pos++;
        p++;
    }
    return pos;
}

size_t TEditor::charPtr( size_t p, int target )
{
  int pos = 0;
  while( (pos < target) && (p < bufLen) && (bufChar(p) != '\x0D') )
    {
    if( bufChar(p) == '\x09' )
        pos |= 7;
    pos++;
    p++;
    }
  if( pos > target )
    p--;
  return p;
}

Boolean TEditor::clipCopy()
{
    Boolean res = False;
    if( (clipboard != 0) && (clipboard != this) )
        {
        res = clipboard->insertFrom(this);
        selecting = False;
        update(ufUpdate);
        }
    return res;
}

void TEditor::clipCut()
{
    if( clipCopy() == True )
        deleteSelect();
}

void TEditor::clipPaste()
{
    if( (clipboard != 0) && (clipboard != this) )
        insertFrom(clipboard);
}

void TEditor::convertEvent( TEvent& event )
{
    if( event.what == evKeyDown )
        {
        if( (event.keyDown.controlKeyState & kbShift) != 0 &&
            event.keyDown.charScan.scanCode >= 0x47 &&
            event.keyDown.charScan.scanCode <= 0x51
          )
            event.keyDown.charScan.charCode = 0;

        ushort key = event.keyDown.keyCode;
        if( keyState != 0 )
            {
            if( (key & 0xFF) >= 0x01 && (key & 0xFF) <= 0x1A )
                key += 0x40;
            if( (key & 0xFF) >= 0x61 && (key & 0xFF) <= 0x7A )
                key -= 0x20;
            }
        key = scanKeyMap(keyMap[keyState], key);
        keyState = 0;
        if( key != 0 )
            if( (key & 0xFF00) == 0xFF00 )
                {
                keyState = (key & 0xFF);
                clearEvent(event);
                }
            else
                {
                event.what = evCommand;
                event.message.command = key;
                }
        }
}

Boolean TEditor::cursorVisible()
{
  return Boolean((curPos.y >= delta.y) && (curPos.y < delta.y + size.y));
}

void TEditor::deleteRange( size_t startPtr,
                           size_t endPtr,
                           Boolean delSelect
                         )
{
    if( hasSelection() == True && delSelect == True )
        deleteSelect();
    else
        {
        setSelect(curPtr, endPtr, True);
        deleteSelect();
        setSelect(startPtr, curPtr, False);
        deleteSelect();
        }
}

void TEditor::deleteSelect()
{
    insertText( 0, 0, False );
}

void TEditor::doneBuffer()
{
    delete[] buffer;
}

void TEditor::doSearchReplace()
{
    int i;
    do  {
        i = cmCancel;
        if( search(findStr, editorFlags) == False )
            {
            if( (editorFlags & (efReplaceAll | efDoReplace)) !=
                (efReplaceAll | efDoReplace) )
                    editorDialog( edSearchFailed );
            }
        else
            if( (editorFlags & efDoReplace) != 0 )
                {
                i = cmYes;
                if( (editorFlags & efPromptOnReplace) != 0 )
                    {
                    TPoint c = makeGlobal( cursor );
                    i = editorDialog( edReplacePrompt, &c );
                    }
                if( i == cmYes )
                    {
                    lock();
                    insertText( replaceStr, strlen(replaceStr), False);
                    trackCursor(False);
                    unlock();
                    }
                }
        } while( i != cmCancel && (editorFlags & efReplaceAll) != 0 );
}

void TEditor::doUpdate()
{
    if( updateFlags != 0 )
        {
        setCursor(curPos.x - delta.x, curPos.y - delta.y);
        if( (updateFlags & ufView) != 0 )
            drawView();
        else
            if( (updateFlags & ufLine) != 0 )
                drawLines( curPos.y-delta.y, 1, lineStart(curPtr) );
        if( hScrollBar != 0 )
            hScrollBar->setParams(delta.x, 0, limit.x - size.x, size.x / 2, 1);
        if( vScrollBar != 0 )
            vScrollBar->setParams(delta.y, 0, limit.y - size.y, size.y - 1, 1);
        if( indicator != 0 )
            indicator->setValue(curPos, modified);
        if( (state & sfActive) != 0 )
            updateCommands();
        updateFlags = 0;
        }
}

void TEditor::draw()
{
    if( drawLine != delta.y )
        {
        drawPtr = lineMove( drawPtr, delta.y - drawLine );
        drawLine = delta.y;
        }
    drawLines( 0, size.y, drawPtr );
}

void TEditor::drawLines( int y, int count, size_t linePtr )
{
    ushort color = getColor(0x0201);
    while( count-- > 0 )
        {
        ushort b[maxLineLength];
        formatLine( b, linePtr, delta.x+size.x, color );
        writeBuf(0, y, size.x, 1, &b[delta.x]);
        linePtr = nextLine(linePtr);
        y++;
        }
}

void TEditor::find()
{
    TFindDialogRec findRec( findStr, editorFlags );
    if( editorDialog( edFind, &findRec ) != cmCancel )
        {
        strcpy( findStr, findRec.find );
        editorFlags = findRec.options & ~efDoReplace;
        doSearchReplace();
        }
}

size_t TEditor::getMousePtr( TPoint m )
{
    TPoint mouse = makeLocal( m );
    mouse.x = max(0, min(mouse.x, size.x - 1));
    mouse.y = max(0, min(mouse.y, size.y - 1));
    return charPtr(lineMove(drawPtr, mouse.y + delta.y - drawLine),
        mouse.x + delta.x);
}

TPalette& TEditor::getPalette() const
{
    static TPalette palette( cpEditor, sizeof( cpEditor )-1 );
    return palette;
}

void TEditor::checkScrollBar( const TEvent& event,
                              TScrollBar *p,
                              int& d
                            )
{
    if( (event.message.infoPtr == p) && (p->value != d) )
        {
        d = p->value;
        update( ufView );
        }
}

void TEditor::handleEvent( TEvent& event )
{
    TView::handleEvent( event );
    convertEvent( event );
    Boolean centerCursor = Boolean(!cursorVisible());
    uchar selectMode = 0;

    if( selecting == True ||
        (event.what & evMouse && (event.mouse.controlKeyState & kbShift) != 0) ||
        (event.what & evKeyboard && (event.keyDown.controlKeyState & kbShift ) != 0)
      )
        selectMode = smExtend;

    convertEvent( event );

    switch( event.what )
        {

        case evMouseDown:
            if( event.mouse.eventFlags & meDoubleClick )
                selectMode |= smDouble;

            do  {
                lock();
                if( event.what == evMouseAuto )
                    {
                    TPoint mouse = makeLocal( event.mouse.where );
                    TPoint d = delta;
                    if( mouse.x < 0 )
                        d.x--;
                    if( mouse.x >= size.x )
                        d.x++;
                    if( mouse.y < 0 )
                        d.y--;
                    if( mouse.y >= size.y )
                        d.y++;
                    scrollTo(d.x, d.y);
                    }
                setCurPtr(getMousePtr(event.mouse.where), selectMode);
                selectMode |= smExtend;
                unlock();
                } while( mouseEvent(event, evMouseMove + evMouseAuto) );
            break;

        case evKeyDown:
            if( event.keyDown.charScan.charCode == 9 ||
                ( event.keyDown.charScan.charCode >= 32 && event.keyDown.charScan.charCode < 255 ) )
                    {
                    lock();
                    if( overwrite == True && hasSelection() == False )
                        if( curPtr != lineEnd(curPtr) )
                            selEnd = nextChar(curPtr);
                    insertText( &event.keyDown.charScan.charCode, 1, False);
                    trackCursor(centerCursor);
                    unlock();
                    }
            else
                return;
            break;

        case evCommand:
            switch( event.message.command )
                {
                case cmFind:
                    find();
                    break;
                case cmReplace:
                    replace();
                    break;
                case cmSearchAgain:
                    doSearchReplace();
                    break;
                default:
                    lock();
                    switch( event.message.command )
                        {
                        case cmCut:
                            clipCut();
                            break;
                        case cmCopy:
                            clipCopy();
                            break;
                        case cmPaste:
                            clipPaste();
                            break;
                        case cmUndo:
                            undo();
                            break;
                        case cmClear:
                            deleteSelect();
                            break;
                        case cmCharLeft:
                            setCurPtr(prevChar(curPtr), selectMode);
                            break;
                        case cmCharRight:
                            setCurPtr(nextChar(curPtr), selectMode);
                            break;
                        case cmWordLeft:
                            setCurPtr(prevWord(curPtr), selectMode);
                            break;
                        case cmWordRight:
                            setCurPtr(nextWord(curPtr), selectMode);
                            break;
                        case cmLineStart:
                            setCurPtr(lineStart(curPtr), selectMode);
                            break;
                        case cmLineEnd:
                            setCurPtr(lineEnd(curPtr), selectMode);
                            break;
                        case cmLineUp:
                            setCurPtr(lineMove(curPtr, -1), selectMode);
                            break;
                        case cmLineDown:
                            setCurPtr(lineMove(curPtr, 1), selectMode);
                            break;
                        case cmPageUp:
                            setCurPtr(lineMove(curPtr, -(size.y-1)), selectMode);
                            break;
                        case cmPageDown:
                            setCurPtr(lineMove(curPtr, size.y-1), selectMode);
                            break;
                        case cmTextStart:
                            setCurPtr(0, selectMode);
                            break;
                        case cmTextEnd:
                            setCurPtr(bufLen, selectMode);
                            break;
                        case cmNewLine:
                            newLine();
                            break;
                        case cmBackSpace:
                            deleteRange(prevChar(curPtr), curPtr, True);
                            break;
                        case cmDelChar:
                            deleteRange(curPtr, nextChar(curPtr), True);
                            break;
                        case cmDelWord:
                            deleteRange(curPtr, nextWord(curPtr), False);
                            break;
                        case cmDelStart:
                            deleteRange(lineStart(curPtr), curPtr, False);
                            break;
                        case cmDelEnd:
                            deleteRange(curPtr, lineEnd(curPtr), False);
                            break;
                        case cmDelLine:
                            deleteRange(lineStart(curPtr), nextLine(curPtr), False);
                            break;
                        case cmInsMode:
                            toggleInsMode();
                            break;
                        case cmStartSelect:
                            startSelect();
                            break;
                        case cmHideSelect:
                            hideSelect();
                            break;
                        case cmIndentMode:
                            autoIndent = Boolean(!autoIndent);
                            break;
                        default:
                            unlock();
                            return;
                        }
                    trackCursor(centerCursor);
                    unlock();
                    break;
                }

        case evBroadcast:
            switch( event.message.command )
                {
                case cmScrollBarChanged:
                    checkScrollBar( event, hScrollBar, delta.x );
                    checkScrollBar( event, vScrollBar, delta.y );
                    break;
                default:
                    return;
                }
        }
    clearEvent(event);
}

