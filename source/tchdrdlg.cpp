/*------------------------------------------------------------*/
/* filename -       tchdrdlg.cpp                              */
/*                                                            */
/* function(s)                                                */
/*          TChDirDialog member functions                     */
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

#define Uses_MsgBox
#define Uses_TChDirDialog
#define Uses_TRect
#define Uses_TInputLine
#define Uses_TLabel
#define Uses_THistory
#define Uses_TScrollBar
#define Uses_TDirListBox
#define Uses_TButton
#define Uses_TEvent
#define Uses_TDirEntry
#define Uses_TDirCollection
#define Uses_TChDirDialog
#define Uses_opstream
#define Uses_ipstream
#include <tv.h>

TChDirDialog::TChDirDialog(
        ushort opts,
        ushort histId,
        const char *title,
        const char *_inidir) :
    TDialog( TRect( 16, 2, 64, 20 ),
             (opts & cdCancelButton) != 0 ? "Select directory" : changeDirTitle ),
    TWindowInit( TChDirDialog::initFrame ),
    inidir(_inidir)
{
    options |= ofCentered;

    dirInput = new TInputLine( TRect( 3, 3, 30, 4 ), 68 );
    insert( dirInput );
    insert( new TLabel( TRect( 2, 2, 62, 3 ), title ? title : dirNameText, dirInput ));
    insert( new THistory( TRect( 30, 3, 33, 4 ), dirInput, histId ) );

    TScrollBar *sb = new TScrollBar( TRect( 32, 6, 33, 16 ) );
    insert( sb );
    dirList = new TDirListBox( TRect( 3, 6, 32, 16 ), sb );
    insert( dirList );
    insert( new TLabel( TRect( 2, 5, 17, 6 ), dirTreeText, dirList ) );

    okButton = new TButton( TRect( 35, 6, 45, 8 ), okText, cmOK, bfDefault );
    insert( okButton );

    chDirButton = new TButton( TRect( 35, 9, 45, 11 ), chdirText, cmChangeDir, bfNormal );
    insert( chDirButton );
    insert( new TButton( TRect( 35, 12, 45, 14 ), revertText, cmRevert, bfNormal ) );

    if( (opts & cdCancelButton) != 0 )
        insert( new TButton( TRect( 35, 15, 45, 17 ), "Cancel", cmCancel, bfNormal ) );
    else if( (opts & cdHelpButton) != 0 )
        insert( new TButton( TRect( 35, 15, 45, 17 ), helpText, cmHelp, bfNormal ) );
    if( (opts & cdNoLoadDir) == 0 )
        setUpDialog();
    selectNext( False );
}

size_t TChDirDialog::dataSize()
{
    return dirInput->dataSize();
}

void TChDirDialog::getDirName(char *buf, size_t bufsize)
{
  return dirInput->getData(buf, bufsize);
}

void TChDirDialog::shutDown()
{
    dirList = 0;
    dirInput = 0;
    okButton = 0;
    chDirButton = 0;
    TDialog::shutDown();
}

void TChDirDialog::getData( void *buf, size_t size )
{
    dirInput->getData(buf, size);
}

void TChDirDialog::handleEvent( TEvent& event )
{
    TDialog::handleEvent( event );
    switch( event.what )
        {
        case evCommand:
            {
            char curDir[MAXPATH];
            switch( event.message.command )
                {
                case cmRevert:
                    if ( inidir == NULL || inidir[0] == '\0' )
                      getCurDir(curDir, sizeof(curDir));
                    else
                      qstrncpy(curDir, inidir, sizeof(curDir));
                    break;
                case cmChangeDir:
                    {
                    TDirEntry *p = dirList->list()->at( dirList->focused );
                    qstrncpy( curDir, p->dir(), sizeof(curDir) );
#ifdef __FAT__
                    if( strcmp( curDir, drivesText ) == 0 )
                        break;
                    else if( !driveValid( curDir[0] ) )
                        return;
                    else
#endif
                        {
                        if( curDir[strlen(curDir)-1] != DIRCHAR )
                            qstrncat( curDir, SDIRCHAR, sizeof(curDir) );
                        }
                    break;
                    }
                default:
                    return;
                }
            dirList->newDirectory( curDir );
            int len = (int)strlen( curDir );
            if( len > 3 && curDir[len-1] == DIRCHAR )
                curDir[len-1] = EOS;
            qstrncpy( dirInput->data, curDir, dirInput->dataSize() );
            dirInput->drawView();
            dirList->select();
            clearEvent( event );
            }
        default:
            break;
        }
}

void TChDirDialog::setData( void *data )
{
    inidir = (const char *)data;
    setUpDialog();
}

void TChDirDialog::setUpDialog()
{
    if( dirList != 0 )
        {
        char curDir[MAXPATH];
        if ( inidir == NULL || inidir[0] == '\0' )
          getCurDir( curDir, sizeof(curDir) );
        else
          qstrncpy(curDir, inidir, sizeof(curDir));
        dirList->newDirectory( curDir );
        if( dirInput != 0 )
            {
            int len = (int)strlen( curDir );
            if( len > 3 && curDir[len-1] == DIRCHAR )
                curDir[len-1] = EOS;
            qstrncpy( dirInput->data, curDir, dirInput->dataSize() );
            dirInput->drawView();
            }
        }
}

#ifdef _MSC_VER
inline int setdisk(int disk) { return _chdrive(disk); }
#endif

static int changeDir( const char *path )
{
#ifdef __FAT__
    if( path[1] == ':' )
        setdisk( toupper(path[0]) - 'A' );
#endif
    return chdir( path );
}

Boolean TChDirDialog::valid( ushort command )
{
    if( command != cmOK )
        return True;

    char path[MAXPATH];
    qstrncpy( path, dirInput->data, sizeof(path) );
    fexpand( path, sizeof(path) );

    size_t len = strlen( path );
    if( len > 3 && path[len-1] == DIRCHAR )
        path[len-1] = EOS;

    if( changeDir( path ) != 0 )
        {
        messageBox( invalidText, mfError | mfOKButton );
        return False;
        }
    return True;
}

#ifndef NO_TV_STREAMS
void TChDirDialog::write( opstream& os )
{
    TDialog::write( os );
    os << dirList << dirInput << okButton << chDirButton;
}

void *TChDirDialog::read( ipstream& is )
{
    TDialog::read( is );
    is >> dirList >> dirInput >> okButton >> chDirButton;
    setUpDialog();
    return this;
}

TStreamable *TChDirDialog::build()
{
    return new TChDirDialog( streamableInit );
}
#endif  // ifndef NO_TV_STREAMS
