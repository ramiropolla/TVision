#ifndef ___extrn_h_
#define ___extrn_h__

#include <stdlib.h>     /* size_t, NULL, memory */
#include <stdarg.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#if defined(__BORLANDC__)
#  include <io.h>       /* open, ... */
#  include <dir.h>      /* mkdir */
#  ifdef __NT__
#    include <alloc.h>
#  endif
#  include <new.h>
#define WIN32_LEAN_AND_MEAN
#else
#if defined(__WATCOMC__) || defined(_MSC_VER)
#ifndef UNDER_CE
#  include <io.h>
#  include <direct.h>
#endif
#  include <limits.h>
#else
#  include <unistd.h>
#  include <sys/stat.h>
#endif
#endif
#ifdef UNDER_CE         // Windows CE does not have many files...
#define getenv(x) NULL  // no getenv under Windows CE
int rename(const char *ofile, const char *nfile);
int unlink(const char *file);
#else
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#ifdef __AMD64__
#include <time.h>
#endif

#if defined(__WATCOMC__) || !defined(__cplusplus)
#include <string.h>
#else
#define STL_SUPPORT_PRESENT
#include <string>
using std::string;
#endif

//-----------------------------------------------------------------------
typedef          char    int8;
typedef signed   char    sint8;
typedef unsigned char    uint8;
typedef          __int16 int16;
typedef unsigned __int16 uint16;
typedef          __int32 int32;
typedef unsigned __int32 uint32;
typedef          __int64 int64;
typedef unsigned __int64 uint64;
typedef int32 ssize_t;

#define qmin(a,b)   ((a) < (b)? (a): (b))
#define qmax(a,b)   ((a) > (b)? (a): (b))
#define qnumber(a)  (sizeof(a)/sizeof((a)[0]))

//------------------------------------------------------------------------
char *qstrncpy(char *dst, const char *src, size_t dstsize);
char *qstrncat(char *dst, const char *src, size_t dstsize);
int  qsnprintf(char *buffer, size_t n, const char *format, ...);
int  qvsnprintf(char *buffer, size_t n, const char *format, va_list va);

//-------------------------------------------------------------------------
#if defined(__MSDOS__) || defined(__OS2__) || defined(__NT__)
#define __FAT__
#define SDIRCHAR "\\"
#define DIRCHAR '\\'
#define DRVCHAR ':'
#else
#define SDIRCHAR "/"
#define DIRCHAR '/'
#endif

#define EXTCHAR '.'

#ifdef __AMD64__

#define MAXDRIVE  _MAX_DRIVE
#define MAXDIR    _MAX_DIR
#define MAXFILE   _MAX_FNAME
#define MAXEXT    _MAX_EXT
#define MAXPATH   _MAX_PATH

struct ffblk : public __finddata64_t
{
  intptr_t handle;
};

#define FA_DIREC    _A_SUBDIR
#define FA_RDONLY   _A_RDONLY
#define FA_ARCH     _A_ARCH

#define findfirst(file,blk,attr) (((blk)->handle=_findfirst64(file,blk))==intptr_t(-1L))
#define findnext(blk)            (_findnext64((blk)->handle,blk)!=0)
#define findclose(blk)           _findclose((blk)->handle)

#define ff_attrib   attrib
#define ff_name     name
#define ff_ftime    time_write
#define ff_fsize    size

#define localtime   _localtime32
#define time_t      __time32_t

#endif // __AMD64__

#endif // ___extrn_h__
