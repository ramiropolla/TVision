/*------------------------------------------------------------*/
/* filename -       tfillist.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TFileList member functions                */
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

#define Uses_TVMemMgr
#define Uses_MsgBox
#define Uses_TFileList
#define Uses_TRect
#define Uses_TSearchRec
#define Uses_TEvent
#define Uses_TGroup
#define Uses_TKeys
#include <tv.h>
#include <tvdir.h>

#include <errno.h>
#include <stdio.h>
#include <assert.h>

#ifdef __LINUX__
#include <sys/types.h>
#include <sys/stat.h>
#include <glob.h>
#include <time.h>
#endif


TFileList::TFileList( const TRect& bounds,
                      TScrollBar *aScrollBar) :
    TSortedListBox( bounds, 2, aScrollBar )
{
}

TFileList::~TFileList()
{
   if ( list() )
      destroy ( list() );
}

void TFileList::focusItem( int item )
{
    TSortedListBox::focusItem( item );
    message( owner, evBroadcast, cmFileFocused, list()->at(item) );
}

void TFileList::getData( void * )
{
}

void TFileList::setData( void * )
{
}

size_t TFileList::dataSize()
{
    return 0;
}

void* TFileList::getKey( const char *s )
{
static TSearchRec sR;

    if( (shiftState & kbShift) != 0 || *s == '.' )
        sR.attr = FA_DIREC;
    else
        sR.attr = 0;
    strcpy( sR.name, s );
#ifdef __MSDOS__
    strupr( sR.name );
#endif
    return &sR;
}

void TFileList::getText( char *dest, int item, int maxChars )
{
  TSearchRec *f = (TSearchRec *)(list()->at(item));

  strncpy( dest, f->name, maxChars );
  dest[maxChars] = '\0';
  if ( f->attr & FA_DIREC )
    strcat( dest, SDIRCHAR );
}

void TFileList::handleEvent( TEvent & event )
{
    if( event.what == evMouseDown && (event.mouse.eventFlags & meDoubleClick) )
        {
        event.what = evCommand;
        event.message.command = cmOK;
        putEvent( event );
        clearEvent( event );
        }
    else
        TSortedListBox::handleEvent( event );
}

void TFileList::readDirectory( const char *dir, const char *wildCard )
{
    char path[MAXPATH];
    strcpy( path, dir );
    strcat( path, wildCard );
    readDirectory( path );
}

struct DirSearchRec : public TSearchRec
{
#if !__FAT__
  void readFf_blk(const char *filename, struct stat &s)
  {
    attr = FA_ARCH;
    if (S_ISDIR(s.st_mode)) attr |= FA_DIREC;
    strcpy(name, filename);
    size = s.st_size;

    ftime t;
    struct tm *broken = localtime(&s.st_mtime);
    t.ft_tsec = broken->tm_sec / 2;
    t.ft_min = broken->tm_min;
    t.ft_hour = broken->tm_hour;
    t.ft_day = broken->tm_mday;

    /*
     * Month value should begin at 1.
     * Date: Thu, 23 Jan 1997 11:34:50 +0100 (MET)
     */
    t.ft_month = broken->tm_mon + 1;
    t.ft_year = broken->tm_year - 80;
    time = *(long *) &t;
  }
#endif
  void *operator new( size_t );

};

void *DirSearchRec::operator new( size_t sz )
{
    void *temp = ::operator new( sz );
    if( TVMemMgr::safetyPoolExhausted() )
        {
        delete (char *)temp;
        temp = 0;
        }
    return temp;
}

void TFileList::readDirectory( const char *aWildCard )
{
#if __FAT__
  ffblk s;

  char path[MAXPATH];
  char drive[MAXDRIVE];
  char dir[MAXDIR];
  char file[MAXFILE];
  char ext[MAXEXT];
  int res;
  DirSearchRec *p;

  const unsigned int findAttr = FA_RDONLY | FA_ARCH;

  TFileCollection *fileList = new TFileCollection( 5, 5 );

  strcpy( path, aWildCard );
  fexpand( path );
  fnsplit( path, drive, dir, file, ext );

  res = findfirst( aWildCard, &s, findAttr );
  p = (DirSearchRec *)&p;
  while( p != 0 && res == 0 )
  {
    if( (s.ff_attrib & FA_DIREC) == 0 )
    {
      p = new DirSearchRec;
      if( p != 0 )
      {
        p->attr = s.ff_attrib;
        p->time = s.ff_ftime + (long(s.ff_fdate) << 16);
        p->size = s.ff_fsize;
        strcpy(p->name,s.ff_name);
        fileList->insert( p );
      }
    }
    res = findnext( &s );
  }
  findclose( &s );

  fnmerge( path, drive, dir, "*", ".*" );

  int  upattr = FA_DIREC;
  long uptime = 0x210000uL;
  long upsize = 0;

  res = findfirst( path, &s, FA_DIREC );
  while( p != 0 && res == 0 )
  {
    if ( (s.ff_attrib & FA_DIREC) != 0 )
    {
      if ( strcmp(s.ff_name,"..") == 0 )
      {
        upattr = s.ff_attrib;
        uptime = s.ff_ftime + (long(s.ff_fdate) << 16);
        upsize = s.ff_fsize;
      }
      else if ( s.ff_name[0] != '.' || s.ff_name[1] != '\0' )
      {
        p = new DirSearchRec;
        if ( p != 0 )
        {
          p->attr = s.ff_attrib;
          p->time = s.ff_ftime + (long(s.ff_fdate) << 16);
          p->size = s.ff_fsize;
          strcpy(p->name,s.ff_name);
          fileList->insert( p );
        }
      }
    }
    res = findnext( &s );
  }
  findclose( &s );
  if ( dir[0] != '\0' && dir[1] != '\0' )
  {
    p = new DirSearchRec;
    if ( p != 0 )
    {
      p->attr = upattr;
      p->time = uptime;
      p->size = upsize;
      strcpy( p->name, ".." );
      fileList->insert( p );
    }
  }
#else
  DIR *dp;
  DirSearchRec *p = NULL;
  char dir[PATH_MAX];
  char file[PATH_MAX];
  char path[PATH_MAX];
  char *np;
  dirent *de;
  glob_t gl;
  struct stat s;

//  printf("path=%s\n", aWildCard);
  strcpy( path, aWildCard );
  if (!isWild(path)) strcat(path, "*");
  fexpand( path );
  expandPath(path, dir, file);
//  printf("expand=%s, %s\n", dir, file);
  TFileCollection *fileList = new TFileCollection( 5, 5 );

  /* find all filenames that match our wildcards */

  /*
   * The use of 'glob' function was proposed by:
   * Rainer Keuchel <r_keuchel@smaug.netwave.de>
   * Date: 18 Jan 1997 22:52:12 +0000
   */
#ifdef GLOB_PERIOD
  if (glob(path, GLOB_PERIOD, NULL, &gl) == 0)
#else
  if (glob(path, 0, NULL, &gl) == 0)
#endif
  for (int i = 0; i < gl.gl_pathc; i++)
  {
    /* is this a regular file ? */

    if (stat(gl.gl_pathv[i], &s) == 0 && S_ISREG(s.st_mode))
    {
      if ((p = new DirSearchRec) == NULL) break;

      /* strip directory part */

      if ((np = strrchr(gl.gl_pathv[i], '/')) != NULL) np++;
      else np = gl.gl_pathv[i];
      p->readFf_blk(np, s);
      fileList->insert( p );
//      printf("file: %s\n", np);
    }
  }
  globfree(&gl);

  /* now read all directory names */

  sprintf(path, "%s.", dir);
  if ((dp = opendir(path)) != NULL)
  {
    while ((de = readdir(dp)) != NULL)
    {
      /* we don't want these directories */

      if (strcmp(de->d_name, ".") == 0 ||
        strcmp(de->d_name, "..") == 0) continue;

      /* is it a directory ? */

      sprintf(path, "%s%s", dir, de->d_name);
      if (stat(path, &s) == 0 && S_ISDIR(s.st_mode))
      {
        if ((p = new DirSearchRec) == NULL) break;
        p->readFf_blk(de->d_name, s);
//        printf("  sub=%s\n", de->d_name);
        fileList->insert( p );
      }
    }
    closedir(dp);
  }

  if( strlen( dir ) > 1 )
  {
    p = new DirSearchRec;
    if( p != 0 )
      {
          sprintf(path, "%s..", dir);
          if (stat(path, &s) == 0) p->readFf_blk("..", s);
          else
          {
                  strcpy( p->name, ".." );
                  p->size = 0;
                  p->time = 0x210000uL;
                  p->attr = FA_DIREC;
          }
          fileList->insert( p );
      }
  }
#endif // if __FAT__
  if ( p == 0 )
    messageBox( tooManyFiles, mfOKButton | mfWarning );
  newList(fileList);
  if( list()->getCount() > 0 )
  {
    message( owner, evBroadcast, cmFileFocused, list()->at(0) );
  }
  else
  {
    static DirSearchRec noFile;
    message( owner, evBroadcast, cmFileFocused, &noFile );
  }
}

#if __FAT__
/*
    fexpand:    reimplementation of pascal's FExpand routine.  Takes a
                relative DOS path and makes an absolute path of the form

                    drive:\[subdir\ ...]filename.ext

                works with '/' or '\' as the subdir separator on input;
                changes all to '\' on output.

*/

static void squeeze( char *path )
{
    char *
    dest = path;
    char *
    src = path;
    while( *src != 0 )
        {
        if (*src == '.')
          if (src[1] == '.') {
            src += 2;
            if (dest > path) {
              dest--;
              while ((*--dest != DIRCHAR)&&(dest > path)) // back up to the previous '\'
                ;
              dest++;         // move to the next position
              }
            }
          else if (src[1] == DIRCHAR)
            src++;
          else
            *dest++ = *src++;
        else
          *dest++ = *src++;
        }
    *dest = EOS;                // zero terminator
    dest = path;
    src = path;
    while( *src != 0 )
        {
        if ((*src == DIRCHAR)&&(src[1] == DIRCHAR))
          src++;
        else
          *dest++ = *src++;
        }
    *dest = EOS;                // zero terminator
}

void fexpand( char *rpath )
{
char path[MAXPATH];
char drive[MAXDRIVE];
char dir[MAXDIR];
char file[MAXFILE];
char ext[MAXEXT];

    int flags = fnsplit( rpath, drive, dir, file, ext );
    if( (flags & DRIVE) == 0 )
        {
        drive[0] = getdisk() + 'A';
        drive[1] = ':';
        drive[2] = '\0';
        }
    drive[0] = toupper(drive[0]);
    if( (flags & DIRECTORY) == 0 || dir[0] != DIRCHAR )
        {
        char curdir[MAXDIR];
        getcurdir( drive[0] - 'A' + 1, curdir );
        // ++ V.Timonin : better more than nothing
        int
        len = strlen(curdir);
        if (curdir[len - 1] != DIRCHAR) {
          curdir[len] = DIRCHAR;
          curdir[len + 1] = EOS;
          }
        // -- V.Timonin
        strcat( curdir, dir );
        if( *curdir != DIRCHAR )
            {
            *dir = DIRCHAR;
            strcpy( dir+1, curdir );
            }
        else
            strcpy( dir, curdir );
        }

    //++ V.Timonin - squeeze must be after '/' --> '\\'
    char *p = dir;
    while( (p = strchr( p, '/' )) != 0 )
      *p = DIRCHAR;
    squeeze( dir );
    //-- V.Timonin
    fnmerge( path, drive, dir, file, ext );
#ifdef __MSDOS__
    strupr( path );
#endif
    strcpy( rpath, path );
}
#endif


#ifndef NO_TV_STREAMS
TStreamable *TFileList::build()
{
    return new TFileList( streamableInit );
}
#endif  // ifndef NO_TV_STREAMS


