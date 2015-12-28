/*------------------------------------------------------------*/
/* filename - teditor2.cpp                                    */
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

int countLines( void *buf, size_t count )
{
  int anzahl=0;
  char *str=(char*) buf;
  for (unsigned int i=0; i<count; i++) {
    if (*(str++)==015) anzahl++;
  }
  return anzahl;
}

// These Routines are taken from Rogue Wave Tools++
size_t scan( const void *block, size_t size, const char *str )
{
  const int32   q        = 33554393L;
  const int32   q32      = q<<5;

  size_t testLength      = size;
  size_t patternLength   = strlen(str);
  if( testLength < patternLength || patternLength == 0 ) return UINT_MAX;

  int32  patternHash     = 0;
  int32  testHash        = 0;

  register const char*  testP= (const char*)block;
  register const char*  patP = str;
  register int32 x = 1;
  size_t         i = patternLength-1;
  while(i--) x =  (x<<5)%q;

  for (i=0; i<patternLength; i++) {
    patternHash = ( (patternHash<<5) + *patP++  ) % q;
    testHash    = ( (testHash   <<5) + *testP++ ) % q;
  }

  testP = (const char*)block;
  const char* end = testP + testLength - patternLength;

  while (1) {

     if(testHash == patternHash)
         return testP-(const char*)block;

     if (testP >= end) break;

     // Advance & calculate the new hash value:
     testHash = ( testHash + q32 - *testP * x                 ) % q;
     testHash = ( (testHash<<5)  + *(patternLength + testP++) ) % q;
  }
  return UINT_MAX;              // Not found.

}

size_t iScan( const void *block, size_t size, const char *str )
{
  const int32   q        = 33554393L;
  const int32   q32      = q<<5;

  size_t testLength      = size;
  size_t patternLength   = strlen(str);
  if( testLength < patternLength || patternLength == 0 ) return UINT_MAX;

  int32  patternHash     = 0;
  int32  testHash        = 0;

  register const char* testP= (const char*)block;
  register const char* patP = str;
  register int32 x = 1;
  size_t         i = patternLength-1;
  while(i--) x =  (x<<5)%q;

  for (i=0; i<patternLength; i++) {
    patternHash = ( (patternHash<<5) + toupper(*patP++)  ) % q;
    testHash    = ( (testHash   <<5) + toupper(*testP++) ) % q;
  }

  testP = (const char*)block;
  const char* end = testP + testLength - patternLength;

  while (1) {

     if(testHash == patternHash)
         return testP-(const char*)block;

     if (testP >= end) break;

     // Advance & calculate the new hash value:
     testHash = ( testHash + q32 - toupper(*testP) * x                 ) % q;
     testHash = ( (testHash<<5)  + toupper(*(patternLength + testP++)) ) % q;
  }
  return UINT_MAX;              // Not found.
}

Boolean TEditor::hasSelection()
{
    return Boolean(selStart != selEnd);
}

void TEditor::hideSelect()
{
    selecting = False;
    setSelect(curPtr, curPtr, False);
}

void TEditor::initBuffer()
{
    buffer = new char[bufSize];
}

Boolean TEditor::insertBuffer( char *p,
                               size_t offset,
                               size_t length,
                               Boolean allowUndo,
                               Boolean selectText
                             )
{
    selecting = False;
    size_t selLen = selEnd - selStart;
    if( selLen == 0 && length == 0 )
        return True;

    size_t delLen = 0;
    if( allowUndo == True )
        if( curPtr == selStart )
            delLen = selLen;
        else
            if( selLen > insCount )
                delLen = selLen - insCount;

    size_t newSize = int32(bufLen + delCount - selLen + delLen) + length;

    if( newSize > (bufLen + delCount) )
        if( newSize > (UINT_MAX-0x1F) || setBufSize(newSize) == False )
            {
            editorDialog( edOutOfMemory );
            return False;
            }

    int selLines = (int)countLines( &buffer[bufPtr(selStart)], selLen );
    if( curPtr == selEnd )
        {
        if( allowUndo == True )
            {
            if( delLen > 0 )
                memmove(
                         &buffer[curPtr + gapLen - delCount - delLen],
                         &buffer[selStart],
                         delLen
                       );
            insCount -= selLen - delLen;
            }
        curPtr = selStart;
        curPos.y -= selLines;
        }
    if( delta.y > curPos.y )
        {
        delta.y -= selLines;
        if( delta.y < curPos.y )
            delta.y = curPos.y;
        }

    if( length > 0 )
        memmove(
                &buffer[curPtr],
                &p[offset],
                length
               );

    int lines = (int)countLines( &buffer[curPtr], length );
    curPtr += length;
    curPos.y += lines;
    drawLine = curPos.y;
    drawPtr = lineStart(curPtr);
    curPos.x = charPos(drawPtr, curPtr);
    if( selectText == False )
        selStart = curPtr;
    selEnd = curPtr;
    bufLen += length - selLen;
    gapLen -= length - selLen;
    if( allowUndo == True )
        {
        delCount += delLen;
        insCount += length;
        }
    limit.y += lines - selLines;
    delta.y = qmax(0, qmin(delta.y, limit.y - size.y));
    if( isClipboard() == False )
        modified = True;
    setBufSize(bufLen + delCount);
    if( selLines == 0 && lines == 0 )
        update(ufLine);
    else
        update(ufView);
    return True;
}

Boolean TEditor::insertFrom( TEditor *editor )
{
    return insertBuffer( editor->buffer,
                         editor->bufPtr(editor->selStart),
                         editor->selEnd - editor->selStart,
                         canUndo,
                         isClipboard()
                        );
}

Boolean TEditor::insertText( const void *text, size_t length, Boolean selectText )
{
  return insertBuffer( (char *)text, 0, length, canUndo, selectText);
}

Boolean TEditor::isClipboard()
{
    return Boolean(clipboard == this);
}

size_t TEditor::lineMove( size_t p, int count )
{
    size_t i = p;
    p = lineStart(p);
    int pos = charPos(p, i);
    while( count != 0 )
        {
        i = p;
        if( count < 0 )
            {
            p = prevLine(p);
            count++;
            }
        else
            {
            p = nextLine(p);
            count--;
            }
        }
    if( p != i )
        p = charPtr(p, pos);
    return p;
}

void TEditor::lock()
{
    lockCount++;
}

void TEditor::newLine()
{
    const char crlf[] = "\x0D\x0A";
    size_t p = lineStart(curPtr);
    size_t i = p;
    while( i < curPtr &&
           ( (buffer[i] == ' ') || (buffer[i] == '\x9'))
         )
         i++;
    insertText(crlf, 2, False);
    if( autoIndent == True )
        insertText( &buffer[p], i - p, False);
}

size_t TEditor::nextLine( size_t p )
{
    return nextChar(lineEnd(p));
}

size_t TEditor::nextWord( size_t p )
{
    while( p < bufLen &&  isWordChar(bufChar(p)) ) p = nextChar(p);
    while( p < bufLen && !isWordChar(bufChar(p)) ) p = nextChar(p);
    return p;
}

size_t TEditor::prevLine( size_t p )
{
  return lineStart(prevChar(p));
}

size_t TEditor::prevWord( size_t p )
{
    while( p > 0 && isWordChar(bufChar(prevChar(p))) == 0 )
        p = prevChar(p);
    while( p > 0 && isWordChar(bufChar(prevChar(p))) != 0 )
        p = prevChar(p);
    return p;
}

void TEditor::replace()
{
    TReplaceDialogRec replaceRec( findStr, replaceStr, editorFlags );
    if( editorDialog( edReplace, &replaceRec ) != cmCancel )
        {
        qstrncpy( findStr, replaceRec.find, sizeof(findStr) );
        qstrncpy( replaceStr, replaceRec.replace, sizeof(replaceStr) );
        editorFlags = replaceRec.options | efDoReplace;
        doSearchReplace();
        }

}

void TEditor::scrollTo( int x, int y )
{
    x = qmax(0, qmin(x, limit.x - size.x));
    y = qmax(0, qmin(y, limit.y - size.y));
    if( x != delta.x || y != delta.y )
        {
        delta.x = x;
        delta.y = y;
        update(ufView);
        }
}

Boolean TEditor::search( const char *findStr, ushort opts )
{
    size_t pos = curPtr;
    size_t i;
    do  {
        if( (opts & efCaseSensitive) != 0 )
            i = scan( &buffer[bufPtr(pos)], bufLen - pos, findStr);
        else
            i = iScan( &buffer[bufPtr(pos)], bufLen - pos, findStr);

        if( i != sfSearchFailed )
            {
            i += pos;
            if( (opts & efWholeWordsOnly) == 0 ||
                !(
                    ( i != 0 && isWordChar(bufChar(i - 1)) != 0 ) ||
                    ( i + strlen(findStr) != bufLen &&
                        isWordChar(bufChar(i + strlen(findStr)))
                    )
                 ))
                {
                lock();
                setSelect(i, i + strlen(findStr), False);
                trackCursor(Boolean(!cursorVisible()));
                unlock();
                return True;
                }
            else
                pos = i + 1;
            }
        } while( i != sfSearchFailed );
    return False;
}

void TEditor::setBufLen( size_t length )
{
    bufLen = length;
    gapLen = bufSize - length;
    selStart = 0;
    selEnd = 0;
    curPtr = 0;
    delta.x = 0;
    delta.y = 0;
    curPos = delta;
    limit.x = maxLineLength;
    limit.y = countLines( &buffer[gapLen], bufLen ) + 1;
    drawLine = 0;
    drawPtr = 0;
    delCount = 0;
    insCount = 0;
    modified = False;
    update(ufView);
}

Boolean TEditor::setBufSize( size_t newSize )
{
    return Boolean(newSize <= bufSize);
}

void TEditor::setCmdState( ushort command, Boolean enable )
{
    TCommandSet s;
    s += command;
    if( enable == True && (state & sfActive) != 0 )
        enableCommands(s);
    else
        disableCommands(s);
}

void TEditor::setCurPtr( size_t p, uchar selectMode )
{
    size_t anchor;
    if( (selectMode & smExtend) == 0 )
        anchor = p;
    else if( curPtr == selStart )
        anchor = selEnd;
    else
        anchor = selStart;

    if( p < anchor )
        {
        if( (selectMode & smDouble) != 0 )
            {
            p = prevLine(nextLine(p));
            anchor = nextLine(prevLine(anchor));
            }
        setSelect(p, anchor, True);
        }
    else
        {
        if( (selectMode & smDouble) != 0 )
            {
            p = nextLine(p);
            anchor = prevLine(nextLine(anchor));
            }
        setSelect(anchor, p, False);
        }
}

void TEditor::setSelect( size_t newStart, size_t newEnd, Boolean curStart )
{
    size_t p;
    if( curStart != 0 )
        p = newStart;
    else
        p = newEnd;

    uchar flags = ufUpdate;

    if( newStart != selStart || newEnd != selEnd )
        if( newStart != newEnd || selStart != selEnd )
            flags = ufView;

    if( p != curPtr )
        {
        if( p > curPtr )
            {
            size_t l = p - curPtr;
            memmove( &buffer[curPtr], &buffer[curPtr + gapLen], l);
            curPos.y += countLines(&buffer[curPtr], l);
            curPtr = p;
            }
        else
            {
            size_t l = curPtr - p;
            curPtr = p;
            curPos.y -= countLines(&buffer[curPtr], l);
            memmove( &buffer[curPtr + gapLen], &buffer[curPtr], l);
            }
        drawLine = curPos.y;
        drawPtr = lineStart(p);
        curPos.x = charPos(drawPtr, p);
        delCount = 0;
        insCount = 0;
        setBufSize(bufLen);
    }
    selStart = newStart;
    selEnd = newEnd;
    update(flags);
}

void TEditor::setState( ushort aState, Boolean enable )
{
    TView::setState(aState, enable);
    switch( aState )
        {
        case sfActive:
            if( hScrollBar != 0 )
                hScrollBar->setState(sfVisible, enable);
            if( vScrollBar != 0 )
                vScrollBar->setState(sfVisible, enable);
            if( indicator != 0 )
                indicator->setState(sfVisible, enable);
            updateCommands();
            break;

        case sfExposed:
            if( enable == True )
                unlock();
        }
}

void TEditor::startSelect()
{
    hideSelect();
    selecting = True;
}

void TEditor::toggleInsMode()
{
    overwrite = Boolean(!overwrite);
    setState(sfCursorIns, Boolean(!getState(sfCursorIns)));
}

void TEditor::trackCursor( Boolean center )
{
    if( center == True )
        scrollTo( curPos.x - size.x + 1, curPos.y - size.y / 2);
    else
        scrollTo( qmax(curPos.x - size.x + 1, qmin(delta.x, curPos.x)),
                  qmax(curPos.y - size.y + 1, qmin(delta.y, curPos.y)));
}

void TEditor::undo()
{
    if( delCount != 0 || insCount != 0 )
        {
        selStart = curPtr - insCount;
        selEnd = curPtr;
        size_t length = delCount;
        delCount = 0;
        insCount = 0;
        insertBuffer(buffer, curPtr + gapLen - length, length, False, True);
        }
}

void TEditor::unlock()
{
    if( lockCount > 0 )
        {
        lockCount--;
        if( lockCount == 0 )
            doUpdate();
        }
}

void TEditor::update( uchar aFlags )
{
    updateFlags |= aFlags;
    if( lockCount == 0 )
        doUpdate();
}

void TEditor::updateCommands()
{
    setCmdState( cmUndo, Boolean( delCount != 0 || insCount != 0 ) );
    if( isClipboard() == False )
        {
        setCmdState(cmCut, hasSelection());
        setCmdState(cmCopy, hasSelection());
        setCmdState(cmPaste,
                    Boolean(clipboard != 0 && (clipboard->hasSelection())) );
        }
    setCmdState(cmClear, hasSelection());
    setCmdState(cmFind, True);
    setCmdState(cmReplace, True);
    setCmdState(cmSearchAgain, True);
}

Boolean TEditor::valid( ushort )
{
  return isValid;
}

#ifndef NO_TV_STREAMS
void TEditor::write( opstream& os )
{
    TView::write( os );
    os << hScrollBar << vScrollBar << indicator
       << bufSize << (int)canUndo;
}

void *TEditor::read( ipstream& is )
{
    TView::read( is );
    int temp;
    is >> hScrollBar >> vScrollBar >> indicator
       >> bufSize >> temp;
    canUndo = Boolean(temp);
    selecting = False;
    overwrite = False;
    autoIndent = False;
    lockCount = 0;
    keyState = 0;
    initBuffer();
    if( bufSize != 0 || buffer != 0 )
        isValid = True;
    else
        {
        TEditor::editorDialog( edOutOfMemory, 0 );
        bufSize = 0;
        }
    lockCount = 0;
    lock();
    setBufLen( 0 );
    return this;
}

TStreamable *TEditor::build()
{
    return new TEditor( streamableInit );
}

TEditor::TEditor( StreamableInit ) : TView( streamableInit )
{
}
#endif  // ifndef NO_TV_STREAMS


