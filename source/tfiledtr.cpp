/*------------------------------------------------------------*/
/* filename - tfiledtr.cpp                                    */
/*                                                            */
/* function(s)                                                */
/*            TFileEditor member functions                    */
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

#define Uses_TGroup
#define Uses_TEditor
#define Uses_TFileEditor
#define Uses_TEvent
#ifndef NO_TV_STREAMS
#define Uses_opstream
#define Uses_ipstream
#endif
#include <tv.h>

#ifndef NO_TV_STREAMS
#include <fstream.h>
#endif
#include <stdio.h>

inline ushort min( size_t u1, size_t u2 )
{
    return ushort(u1 < u2 ? u1 : u2);
}

TFileEditor::TFileEditor( const TRect& bounds,
                          TScrollBar *aHScrollBar,
                          TScrollBar *aVScrollBar,
                          TIndicator *aIndicator,
                          const char *aFileName
                        ) :
    TEditor( bounds, aHScrollBar, aVScrollBar, aIndicator, 0 )
{
    if( aFileName == 0 )
        fileName[0] = EOS;
    else
        {
        qstrncpy( fileName, aFileName, sizeof(fileName) );
        fexpand( fileName, sizeof(fileName) );
        if( isValid )
            isValid = loadFile();
        }
}

void TFileEditor::doneBuffer()
{
    delete buffer;
}

void TFileEditor::handleEvent( TEvent& event )
{
    TEditor::handleEvent(event);
    switch( event.what )
        {
        case evCommand:
            switch( event.message.command )
                {
                case cmSave:
                    save();
                    break;
                case cmSaveAs:
                    saveAs();
                    break;
                default:
                    return;
                }
            break;
        default:
            return;
        }
    clearEvent(event);
}

void TFileEditor::initBuffer()
{
    buffer = new char[bufSize];
}

Boolean TFileEditor::loadFile()
{
    FILE *fi = fopen(fileName, "rb");
    if( !fi )
        {
        setBufLen( 0 );
        return True;
        }
    else
        {
          fseek(fi, 0, SEEK_END);
          long fSize = ftell(fi);
          fseek(fi, 0, SEEK_SET);
        if( fSize > (int32)(UINT_MAX-0x1F) || setBufSize(size_t(fSize)) == False )
            {
            fclose(fi);
            editorDialog( edOutOfMemory );
            return False;
            }
        else
            {
            if ( fSize > INT_MAX )
            {
               fread( &buffer[bufSize - size_t(fSize)], 1, INT_MAX, fi);
               fread( &buffer[bufSize - size_t(fSize) + INT_MAX],
                                size_t(fSize - INT_MAX), 1, fi);

            }
            else
               fread( &buffer[bufSize - size_t(fSize)], size_t(fSize), 1, fi);
            if( ferror(fi) || feof(fi) )
                {
                fclose(fi);
                editorDialog( edReadError, fileName );
                return False;
                }
            else
                {
                fclose(fi);
                setBufLen(size_t(fSize));
                return True;
                }
            }
        }
}

Boolean TFileEditor::save()
{
    if( *fileName == EOS )
        return saveAs();
    else
        return saveFile();
}

Boolean TFileEditor::saveAs()
{
    Boolean res = False;
    if( editorDialog( edSaveAs, fileName, sizeof(fileName) ) != cmCancel )
        {
        fexpand( fileName, sizeof(fileName) );
        message( owner, evBroadcast, cmUpdateTitle, 0 );
        res = saveFile();
        if( isClipboard() == True )
            *fileName = EOS;
        }
    return res;
}

static void writeBlock( FILE *fo, char *buf, unsigned len )
{
    while( len > 0 )
        {
        int l = len < INT_MAX ? len : INT_MAX;
        fwrite( buf, l, 1, fo);
        buf += l;
        len -= l;
        }
}

Boolean TFileEditor::saveFile()
{
    if( (editorFlags & efBackupFiles) != 0 )
        {
        char backupName[MAXPATH];
#ifdef __FAT__
        char drive[MAXDRIVE];
        char dir[MAXDIR];
        char file[MAXFILE];
        char ext[MAXEXT];
        fnsplit( fileName, drive, dir, file, ext );
        fnmerge( backupName, drive, dir, file, backupExt );
#else
       	qsnprintf(backupName, sizeof(backupName), "%s%s", fileName, backupExt);
#endif
        unlink( backupName );
        rename( fileName, backupName );
        }

    FILE *f = fopen(fileName, "wb");

    if( !f )
        {
        editorDialog( edCreateError, fileName );
        return False;
        }
    else
        {
        writeBlock( f, buffer, uint32(curPtr) );
        writeBlock( f, buffer+curPtr+gapLen, uint32(bufLen-curPtr) );

        if(ferror(f) || feof(f))
            {
            fclose(f);
            editorDialog( edWriteError, fileName );
            return False;
            }
        else
            {
            fclose(f);
            modified = False;
            update(ufUpdate);
            }

        }
    return True;
}

Boolean TFileEditor::setBufSize( size_t newSize )
{
    newSize = (newSize + 0x0FFF) & (UINT_MAX-0x0FFF);
    if( newSize != bufSize )
        {
        char *temp = buffer;
        if( (buffer = new char[newSize]) == 0 )
            {
            delete temp;
            return False;
            }
        size_t n = bufLen - curPtr + delCount;
        memcpy( buffer, temp, min( newSize, bufSize ) );
        memmove( &buffer[newSize - n], &temp[bufSize - n], n );
        delete temp;
        bufSize = newSize;
        gapLen = bufSize - bufLen;
        }
    return True;
}

void TFileEditor::shutDown()
{
    setCmdState(cmSave, False);
    setCmdState(cmSaveAs, False);
    TEditor::shutDown();
}

void TFileEditor::updateCommands()
{
    TEditor::updateCommands();
    setCmdState(cmSave, True);
    setCmdState(cmSaveAs, True);
}

Boolean TFileEditor::valid( ushort command )
{
    if( command == cmValid )
        return isValid;
    else
        {
        if( modified == True )
            {
            int d;
            if( *fileName == EOS )
                d = edSaveUntitled;
            else
                d = edSaveModify;

            switch( editorDialog( d, fileName, sizeof(fileName) ) )
                {
                case cmYes:
                    return save();
                case cmNo:
                    modified = False;
                    return True;
                case cmCancel:
                    return False;
                }
            }
        }
    return True;
}

#ifndef NO_TV_STREAMS
void TFileEditor::write( opstream& os )
{
    TEditor::write( os );
    os.writeString( fileName );
    os << selStart << selEnd << curPtr;
}

void *TFileEditor::read( ipstream& is )
{
    TEditor::read( is );
    is.readString( fileName, sizeof( fileName ) );
    if( isValid )
        {
        isValid = loadFile();
        size_t sStart, sEnd, curs;
        is >> sStart >> sEnd >> curs;
        if( isValid && sEnd <= bufLen )
            {
            setSelect( sStart, sEnd, Boolean(curs == sStart) );
            trackCursor( True );
            }
        }
    return this;
}

TStreamable *TFileEditor::build()
{
    return new TFileEditor( streamableInit );
}

TFileEditor::TFileEditor( StreamableInit ) : TEditor( streamableInit )
{
}
#endif  // ifndef NO_TV_STREAMS

