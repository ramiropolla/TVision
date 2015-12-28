/*------------------------------------------------------------*/
/* filename -       tfildlg.cpp                               */
/*                                                            */
/* function(s)                                                */
/*                  TFileDialog member functions              */
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

#define Uses_TFileDialog
#define Uses_MsgBox
#define Uses_TRect
#define Uses_TFileInputLine
#define Uses_TButton
#define Uses_TLabel
#define Uses_TFileList
#define Uses_THistory
#define Uses_TScrollBar
#define Uses_TEvent
#define Uses_TFileInfoPane
#define Uses_opstream
#define Uses_ipstream
#include <tv.h>

#include <errno.h>
#include <stdio.h>

// File dialog flags
const int
    ffOpen        = 0x0001,
    ffSaveAs      = 0x0002;

const int
    cmOpenDialogOpen    = 100,
    cmOpenDialogReplace = 101;

TFileDialog::TFileDialog( const char *aWildCard,
                          const char *aTitle,
                          const char *inputName,
                          ushort aOptions,
                          uchar histId
                        ) :
    TDialog( TRect( 15, 1, 64, 21 ), aTitle ),
    TWindowInit( TFileDialog::initFrame ),
    directory( NULL )
{
    options |= ofCentered;
    qstrncpy( wildCard, aWildCard, sizeof(wildCard) );

    fileName = new TFileInputLine( TRect( 3, 3, 31, 4 ), MAXPATH );
    qstrncpy( fileName->data, wildCard, fileName->dataSize() );
    insert( fileName );

    insert( new TLabel( TRect( 2, 2, 3+cstrlen(inputName), 3 ),
                        inputName,
                        fileName
                      ) );
    insert( new THistory( TRect( 31, 3, 34, 4 ), fileName, histId ) );
    TScrollBar *sb = new TScrollBar( TRect( 3, 14, 34, 15 ) );
    insert( sb );
    insert( fileList = new TFileList( TRect( 3, 6, 34, 14 ), sb ) );
    insert( new TLabel( TRect( 2, 5, 8, 6 ), filesText, fileList ) );

    ushort opt = bfDefault;
    TRect r( 35, 3, 46, 5 );

    if( (aOptions & fdOpenButton) != 0 )
        {
        insert( new TButton( r, openText, cmFileOpen, opt ) );
        opt = bfNormal;
        r.a.y += 3;
        r.b.y += 3;
        }

    if( (aOptions & fdOKButton) != 0 )
        {
        insert( new TButton( r, okText, cmFileOpen, opt ) );
        opt = bfNormal;
        r.a.y += 3;
        r.b.y += 3;
        }

    if( (aOptions & fdReplaceButton) != 0 )
        {
        insert( new TButton( r, replaceText, cmFileReplace, opt ) );
        opt = bfNormal;
        r.a.y += 3;
        r.b.y += 3;
        }

    if( (aOptions & fdClearButton) != 0 )
        {
        insert( new TButton( r, clearText, cmFileClear, opt ) );
//        opt = bfNormal;
        r.a.y += 3;
        r.b.y += 3;
        }

    insert( new TButton( r, cancelText, cmCancel, bfNormal ) );
    r.a.y += 3;
    r.b.y += 3;

    if( (aOptions & fdHelpButton) != 0 )
        {
        insert( new TButton( r, helpText, cmHelp, bfNormal ) );
//        opt = bfNormal;
        r.a.y += 3;
        r.b.y += 3;
        }

    insert( new TFileInfoPane( TRect( 1, 16, 48, 19 ) ) );

    selectNext( False );
    if( (aOptions & fdNoLoadDir) == 0 )
        readDirectory();
}

TFileDialog::~TFileDialog()
{
    delete[] directory;
}

void TFileDialog::shutDown()
{
    fileName = 0;
    fileList = 0;
    TDialog::shutDown();
}

static Boolean relativePath( const char *path )
{
    if( path[0] == DIRCHAR )
        return False;
#ifdef __FAT__
    if( path[0] != EOS && path[1] == ':' )
        return False;
#endif
    return True;
}

#ifdef __FAT__
static void noWildChars( char *dest, const char *src )
{
    while( *src != EOS )
        {
        if( *src != '?' && *src != '*' )
            *dest++ = *src;
        src++;
        }
    *dest = EOS;
}
#endif

#ifdef __MSDOS__
static void trim( char *dest, const char *src, size_t destsize )
{
  if ( ssize_t(destsize) > 0 )
  {
    while( *src != EOS && isspace(uchar(*src)) )
        src++;
    char *end = dest + destsize;
    while( *src != EOS && !isspace(uchar(*src)) && dest < end )
        *dest++ = *src++;
    if ( dest >= end )
      dest--;
    *dest = EOS;
  }
}
#else
#define trim qstrncpy
#endif

void TFileDialog::getFileName( char *str, size_t strsize )
{
char buf[2*MAXPATH];

#ifdef __FAT__
char drive[MAXDRIVE];
char path[MAXDIR];
char name[MAXFILE];
char ext[MAXEXT];
char TName[MAXFILE];
char TExt[MAXEXT];

    if ( fileName->data[0] == '.'                                // +++ yjh
      && (fileName->data[1] == DIRCHAR || fileName->data[1] == '\0') ) // +++ yjh
    {                                                            // +++ yjh
      getcwd(buf, sizeof(buf));                                  // +++ yjh
      if ( fileName->data[2] )                                   // +++ yjh
        trim( buf + strlen(buf), fileName->data+1, sizeof(buf)-strlen(buf) ); // +++ yjh
    }                                                            // +++ yjh
    else                                                         // +++ yjh
    {
      trim( buf, fileName->data, sizeof(buf) );
    }
    if( relativePath( buf ) == True )
        {
        qstrncpy( buf, directory, sizeof(buf) );
        trim( buf + strlen(buf), fileName->data, sizeof(buf)-strlen(buf) );
        }
    fexpand( buf, sizeof(buf) );
    fnsplit( buf, drive, path, name, ext );
//    printf("split %s\n drive=%s,path=%s,name=%s,ext=%s\n",buf,drive,path,name,ext);
    if( (name[0] == EOS || ext[0] == EOS) && !isDir( buf ) )
        {
        fnsplit( wildCard, 0, 0, TName, TExt );
        if( name[0] == EOS && ext[0] == EOS )
            fnmerge( buf, drive, path, TName, TExt );
        else if( name[0] == EOS )
            fnmerge( buf, drive, path, TName, ext );
        else if( ext[0] == EOS )
            {
            if( isWild( name ) )
                fnmerge( buf, drive, path, name, TExt );
            else
                {
                fnmerge( buf, drive, path, name, 0 );
                noWildChars( buf + strlen(buf), TExt );
                }
            }
        }
#else
  qstrncpy( buf, fileName->data, sizeof(buf) );
  if( relativePath( buf ) == True )
  {
    qstrncpy( buf, directory, sizeof(buf) );
    qstrncpy( buf + strlen(buf), fileName->data, sizeof(buf)-strlen(buf) );
  }
  fexpand( buf, sizeof(buf) );
#endif
  qstrncpy( str, buf, strsize );
}

void TFileDialog::handleEvent(TEvent& event)
{
  TDialog::handleEvent(event);
  if( event.what == evCommand )
    switch( event.message.command )
    {
      case cmFileOpen:
      case cmFileReplace:
      case cmFileClear:
        endModal(event.message.command);
        clearEvent(event);
        break;
      default:
        break;
    }
}

void TFileDialog::readDirectory()
{
    char curDir[MAXPATH];
#ifdef __FAT__
    if ( relativePath(wildCard) )
    {
      getCurDir( curDir, sizeof(curDir) );
    }
    else
    {
      char drive[MAXDRIVE], dir[MAXDIR], name[MAXFILE], ext[MAXEXT];
      fnsplit( wildCard, drive, dir, name, ext );
      qstrncpy(curDir, drive, sizeof(curDir));
      qstrncat(curDir, dir, sizeof(curDir));
      qstrncpy(wildCard, name, sizeof(wildCard));
      qstrncat(wildCard, ext, sizeof(wildCard));
    }
#else
    getCurDir( curDir, sizeof(curDir) );
#endif
    if ( directory != NULL )
      delete[] directory;
    directory = newStr( curDir );
    fileList->readDirectory( directory, wildCard );
}

void TFileDialog::setData( void *rec )
{
  TDialog::setData( rec );
  if( *(char *)rec != EOS && isWild( (char *)rec ) )
  {
      valid( cmFileInit );
      fileName->select();
  }
}

void TFileDialog::getData( void *rec, size_t recsize )
{
    getFileName( (char *)rec, recsize );
}

Boolean TFileDialog::checkDirectory( const char *str )
{
  if( pathValid( str ) )
    return True;

  messageBox( invalidDriveText, mfError | mfOKButton );
  fileName->select();
  return False;
}

Boolean TFileDialog::valid(ushort command)
{
  if ( !TDialog::valid( command ) ) return False;
  if ( command == cmValid || command == cmCancel || command == cmFileClear )
       return True;

  char fName[MAXPATH], name[MAXFILE];
#ifdef __FAT__
  char dir[MAXDIR];
#endif

  getFileName(fName, sizeof(fName));
  if ( isWild(fName) )
  {
    char path[MAXPATH];
#ifdef __FAT__
    char drive[MAXDRIVE];
    char ext[MAXEXT];
    fnsplit( fName, drive, dir, name, ext );
    qstrncpy( path, drive, sizeof(path) );
    qstrncat( path, dir, sizeof(path) );
#else
    expandPath(fName, path, sizeof(path), name, sizeof(name));
#endif
    if ( checkDirectory( path ) )
    {
      delete[] directory;
      directory = newStr( path );
      qstrncpy( wildCard, name, sizeof(wildCard) );
      if( command != cmFileInit ) fileList->select();
      fileList->readDirectory( directory, wildCard );
    }
    return False;
  }
  if ( isDir(fName) )
  {
    if ( checkDirectory(fName) )
    {
      delete[] directory;
      if ( fName[strlen(fName)-1] != DIRCHAR )
        qstrncat( fName, SDIRCHAR, sizeof(fName) );
      directory = newStr( fName );
      if ( command != cmFileInit )
        fileList->select();
      fileList->readDirectory( directory, wildCard );
    }
    return False;
  }
  if ( validFileName(fName) ) return True;

  messageBox( invalidFileText, mfError | mfOKButton );
  return False;
}

#ifndef NO_TV_STREAMS
void TFileDialog::write( opstream& os )
{
    TDialog::write( os );
    os.writeString( wildCard );
    os << fileName << fileList;
}

void *TFileDialog::read( ipstream& is )
{
    TDialog::read( is );
    is.readString( wildCard, sizeof(wildCard) );
    is >> fileName >> fileList;
    readDirectory();
    return this;
}

TStreamable *TFileDialog::build()
{
    return new TFileDialog( streamableInit );
}
#endif  // ifndef NO_TV_STREAMS
