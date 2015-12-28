/*------------------------------------------------------------*/
/* filename -       tdircoll.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TDirCollection member functions           */
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

#define Uses_TDirCollection
#define Uses_TDirEntry
#define Uses_opstream
#define Uses_ipstream
#define Uses_TThreaded
#include <tv.h>
#ifdef __IDA__
#include <prodir.h>
#endif

#include <stdio.h>
#include <sys/stat.h>

#ifdef __FAT__
Boolean driveValid( char drive )
{
  drive = (char)toupper(drive);
#ifdef __MSDOS__
  struct diskfree_t df;
  return Boolean(_dos_getdiskfree(drive-'@',&df) == 0);
#elif defined(__OS2__)
  FSALLOCATE a;
  return Boolean(!DosQueryFSInfo(drive - '@', FSIL_ALLOC, &a, sizeof(a) ) );
#elif defined(__NT__)
  DWORD mask = 0x01 << (drive - 'A');
  return Boolean((GetLogicalDrives() & mask) != 0);
#else
#error Unknown platform!
#endif
}
#endif

Boolean isDir( const char *str )
{
#ifdef __NT__
  DWORD a = GetFileAttributes(str);
  return a != (DWORD)INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) != 0
        ? True
        : False;
#else
  struct stat s;
  return stat(str, &s) == 0 && S_ISDIR(s.st_mode)
        ? True
        : False;
#endif
}

Boolean pathValid( const char *path )
{
    char expPath[MAXPATH];
    qstrncpy( expPath, path, sizeof(expPath) );
    fexpand( expPath, sizeof(expPath) );
    int len = (int)strlen(expPath);
#ifdef __FAT__
    if( len <= 3 )
        return driveValid(expPath[0]);
#endif
    if( expPath[len-1] == DIRCHAR )
        expPath[len-1] = EOS;

    return isDir( expPath );
}

Boolean validFileName( const char *fileName )
{
#ifdef __FAT__
#ifdef __MSDOS__
    static const char illegalChars[] = ";,=+<>|\"[] " SDIRCHAR;
#else
    static const char illegalChars[] = "<>|\"" SDIRCHAR;
#endif

    char path[MAXPATH];
    char dir[MAXDIR];
    char name[MAXFILE];
    char ext[MAXEXT];

    ext[1] = 0; // V.Timonin
    fnsplit( fileName, path, dir, name, ext );
    qstrncat( path, dir, sizeof(path) );
    if( *dir != EOS && !pathValid( path ) )
        return False;
    if( strpbrk( name, illegalChars ) != NULL ||
        strpbrk( ext+1, illegalChars) != NULL ||
        strchr( ext+1, '.' ) != 0
      )
        return False;
    return True;
#else
    /* SS: all changed */
    FILE *f;

    /*
     * Patch from: Vasily Egoshin <wasa@nica.marstu.mari.su>
     * Date: Thu, 9 Jan 1997 16:36:10 +0300 (MSK)
     */
    if ((f = fopen(fileName, "r")) != NULL)
    {
    	/* the file exists and is readable ===> file name ok */

      fclose(f);
      return True;
    }
    if ((f = fopen(fileName, "w")) != NULL)
    {
      /* file doesn't exist but it is writable ===> file name ok */

      fclose(f);
      remove(fileName);
      return True;
    }
    return False;	/* illegal file name */
#endif
}

void getCurDir( char *dir, size_t dirsize )
{
  if ( ssize_t(dirsize) > 0 )
  {
    getcwd(dir, (int)dirsize);
    size_t len = strlen(dir);
    if ( len > 3 && len < dirsize - 1 && dir[len-1] != DIRCHAR )
    {
      dir[len] = DIRCHAR;
      dir[len + 1] = '\0';
    }
  }
}

Boolean isWild( const char *f )
{
    return Boolean( strpbrk( f, "?*" ) != 0 );
}

#ifndef NO_TV_STREAMS
TStreamable *TDirCollection::build()
{
    return new TDirCollection( streamableInit );
}

void TDirCollection::writeItem( void *obj, opstream& os )
{
    TDirEntry *item = (TDirEntry *)obj;
    os.writeString( item->text() );
    os.writeString( item->dir() );
}

void *TDirCollection::readItem( ipstream& is )
{
    const char *txt = is.readString();
    const char *dir = is.readString();
    return new TDirEntry( txt, dir );
}
#endif  // ifndef NO_TV_STREAMS
