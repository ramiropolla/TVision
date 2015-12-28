/*------------------------------------------------------------*/
/* filename -       tdirlist.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TDirListBox member functions              */
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

#define Uses_TDirListBox
#define Uses_TEvent
#define Uses_TDirCollection
#define Uses_TChDirDialog
#define Uses_TDirEntry
#define Uses_TButton
#include <tv.h>

#include <sys/stat.h>

TDirListBox::TDirListBox( const TRect& bounds, TScrollBar *aScrollBar ) :
    TListBox( bounds, 1, aScrollBar ),
    cur( 0 )
{
    *dir = EOS;
}

TDirListBox::~TDirListBox()
{
   if ( list() )
      destroy( list() );
}

void TDirListBox::getText( char *text, int item, size_t textsize )
{
        qstrncpy( text, list()->at(item)->text(), textsize );
}

void TDirListBox::handleEvent( TEvent& event )
{
    if( event.what == evMouseDown && (event.mouse.eventFlags & meDoubleClick) )
        {
        event.what = evCommand;
        event.message.command = cmChangeDir;
        putEvent( event );
        clearEvent( event );
        }
    else
        TListBox::handleEvent( event );
}

Boolean TDirListBox::isSelected( int item )
{
    return Boolean( item == cur );
}

void TDirListBox::showDrives( TDirCollection *dirs )
{
#ifdef __FAT__
    Boolean isFirst = True;
    char oldc[5];
    qstrncpy( oldc, "0:" SDIRCHAR, sizeof(oldc) );
    for ( char c = 'A'; c <= 'Z'; c++ )
    {
      if ( c < 'C' || driveValid( c ) )
      {
        if ( oldc[0] != '0' )
        {
          char s[ 16 ];
          char drv[2];
          drv[0] = oldc[0];
          drv[1] = '\0';
          const char *dirsign = isFirst ? firstDir : middleDir;
          qstrncpy( s, dirsign, sizeof(s) );
          qstrncat( s, drv, sizeof(s) );
          isFirst = False;
          dirs->insert( new TDirEntry( s, oldc ) );
        }
        if ( c == getdisk() + 'A' )
          cur = ushort(dirs->getCount());
        oldc[0] = c;
      }
    }
    if( oldc[0] != '0' )
    {
      char s[ 16 ];
      char drv[2];
      drv[0] = oldc[0];
      drv[1] = '\0';
      qstrncpy( s, lastDir, sizeof(s) );
      qstrncat( s, drv, sizeof(s) );
      dirs->insert( new TDirEntry( s, oldc ) );
    }
#else
    (void)dirs;
#endif
}

void TDirListBox::showDirs( TDirCollection *dirs )
{
    const int indentSize = 2;
    int indent = indentSize;

    char buf[2*MAXPATH];
    memset( buf, ' ', sizeof( buf ) );
    char *bufend = buf + sizeof(buf);
    char *const name = bufend - MAXPATH;

    char *org = name - strlen(pathDir);
    qstrncpy( org, pathDir, bufend-org );

    char *curDir = dir;
    char *end = dir + 3;
    char hold = *end;
    *end = EOS;         // mark end of drive name
    qstrncpy( name, curDir, bufend-name );
    dirs->insert( new TDirEntry( org, name ) );

    *end = hold;        // restore full path
    curDir = end;
    while( (end = strchr( curDir, DIRCHAR )) != NULL )
        {
        *end = EOS;
        memcpy( name, curDir, end-curDir+1 );
        dirs->insert( new TDirEntry( org - indent, dir ) );
        *end = DIRCHAR;
        curDir = end+1;
        indent += indentSize;
        }

    cur = ushort(dirs->getCount() - 1);

    end = strrchr( dir, DIRCHAR );
    char path[MAXPATH];
    char *pathend = path + sizeof(path);
    size_t len = end-dir+1;
    memcpy( path, dir, len );
    end = path + len;
    qstrncpy( end, "*",  pathend-end);

    Boolean isFirst = True;
    ffblk ff;
    int res = findfirst( path, &ff, FA_DIREC );
    while( res == 0 )
        {
        if( (ff.ff_attrib & FA_DIREC) != 0 && ff.ff_name[0] != '.' )
            {
            if( isFirst )
                {
                memcpy( org, firstDir, strlen(firstDir)+1 );
                isFirst = False;
                }
            else
                memcpy( org, middleDir, strlen(middleDir)+1 );
            qstrncpy( name, ff.ff_name, bufend-name );
            qstrncpy( end, ff.ff_name, pathend-end );
            dirs->insert( new TDirEntry( org - indent, path ) );
            }
        res = findnext( &ff );
        }
    findclose( &ff );

    char *p = dirs->at(dirs->getCount()-1)->text();
    char *i = strchr( p, graphics[0] );
    if( i == 0 )
        {
        i = strchr( p, graphics[1] );
        if( i != 0 )
            *i = graphics[0];
        }
    else
        {
        *(i+1) = graphics[2];
        *(i+2) = graphics[2];
        }
}

void TDirListBox::newDirectory( const char *str )
{
    qstrncpy( dir, str, sizeof(dir) );
    TDirCollection *dirs = new TDirCollection( 5, 5 );
    dirs->insert( new TDirEntry( drives, drives ) );
    if( strcmp( dir, drives ) == 0 )
        showDrives( dirs );
    else
        showDirs( dirs );
    newList( dirs );
    focusItem( cur );
}

void TDirListBox::setState( ushort nState, Boolean enable )
{
    TListBox::setState( nState, enable );
    if( (nState & sfFocused) && ((TChDirDialog *)owner)->chDirButton )
        ((TChDirDialog *)owner)->chDirButton->makeDefault( enable );
}

#ifndef NO_TV_STREAMS
TStreamable *TDirListBox::build()
{
    return new TDirListBox( streamableInit );
}
#endif  // ifndef NO_TV_STREAMS


