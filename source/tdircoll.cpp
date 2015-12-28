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
#include <tvdir.h>

#include <stdio.h>
#include <sys/stat.h>

#if __FAT__
Boolean driveValid( char drive )
{
  drive = toupper(drive);
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
	/* SS: all changed */
	struct stat s;

	if (stat(str, &s) == 0 && S_ISDIR(s.st_mode)) return True;
	return False;
}

Boolean pathValid( const char *path )
{
    char expPath[MAXPATH];
    strcpy( expPath, path );
    fexpand( expPath );
    int len = strlen(expPath);
#if __FAT__
    if( len <= 3 )
        return driveValid(expPath[0]);
#endif
    if( expPath[len-1] == DIRCHAR )
        expPath[len-1] = EOS;

    return isDir( expPath );
}

Boolean validFileName( const char *fileName )
{
#if __FAT__
#ifdef __MSDOS__
    static const char * const illegalChars = ";,=+<>|\"[] " SDIRCHAR;
#else
    static const char * const illegalChars = "<>|\"" SDIRCHAR;
#endif

    char path[MAXPATH];
    char dir[MAXDIR];
    char name[MAXFILE];
    char ext[MAXEXT];

    ext[1] = 0; // V.Timonin
    fnsplit( fileName, path, dir, name, ext );
    strcat( path, dir );
    if( *dir != EOS && !pathValid( path ) )
        return False;
    if( strpbrk( name, illegalChars ) != 0 ||
        strpbrk( ext+1, illegalChars) != 0 ||
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

void getCurDir( char *dir )
{
    getcwd(dir,MAXPATH);
    int len = strlen(dir);
    if ( len > 3 && len < MAXPATH - 1 && dir[len-1] != DIRCHAR )
    {
      dir[len] = DIRCHAR;
      dir[len + 1] = '\0';
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
