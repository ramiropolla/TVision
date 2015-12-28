/*
 *      Interactive disassembler (IDA).
 *      Copyright (c) 1990-97 by Ilfak Guilfanov.
 *      ALL RIGHTS RESERVED.
 *                              E-mail: ig@estar.msk.su
 *                              FIDO:   2:5020/209
 *
 */

#ifndef _PRODIR_H
#define _PRODIR_H
#pragma pack(push, 1)

//
//      This file contains unified interface to findfirst/findnext/findclose
//      functions.
//      You may continue to use find...() functions as you would do in BCC.
//      (don't forget to call findclose(), though)
//
//      It is better to use enumerate_files() from diskio.hpp
//

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

#if defined(_MSC_VER)

#  include <io.h>

#ifdef __AMD64__
#define prodir_base __finddata64_t
#define prodir_first _findfirst64
#define prodir_next  _findnext64
#else
#define prodir_base _finddata_t
#define prodir_first _findfirst
#define prodir_next  _findnext
#endif

struct ffblk : public prodir_base
{
#ifdef __AMD64__
  intptr_t handle;
#else
  long handle;
#endif
};

#  define MAXPATH               _MAX_PATH
#  define MAXDRIVE              _MAX_DRIVE
#  define MAXDIR                _MAX_DIR
#  define MAXFILE               _MAX_FNAME
#  define MAXEXT                _MAX_EXT
#  define FA_DIREC              _A_SUBDIR
#  define FA_RDONLY             _A_RDONLY
#  define FA_ARCH               _A_ARCH
#  define ff_attrib             attrib
#  define ff_name               name
#  define ff_ftime              time_write
#  define ff_fsize              size
#  define findfirst(file,blk,attr) (((blk)->handle=prodir_first(file,blk))==-1L)
#  define findnext(blk)         (prodir_next((blk)->handle,blk)!=0)
#  define findclose(blk)         _findclose((blk)->handle)

#elif defined(__GNUC__) // unix systems

#include <limits.h>
#ifndef __MACOSX__
#include <linux/limits.h>
#endif

struct ffblk
{
// user fields:
  int ff_attrib;
  char ff_name[QMAXPATH];
// private fields:
  void *filelist;
  int fileidx, fileqty;
  char dirpath[QMAXPATH];
  char pattern[QMAXPATH];
  int attr;
};

idaman int ida_export findfirst(const char *fname, ffblk *blk, int attr);
idaman int ida_export findnext(ffblk *blk);
idaman void ida_export findclose(ffblk *blk);
#define MAXPATH               QMAXPATH
#define MAXDIR                QMAXPATH
#define MAXFILE               QMAXPATH
#define MAXEXT                QMAXPATH
#define FA_DIREC              S_IFDIR
#define FA_ARCH		      0
#define FA_RDONLY	      0

#else

#  include <dir.h>
#if __BORLANDC__ < 0x0540
#  define findclose(p)
#endif

#endif

#ifdef __LINUX__
#define FA_FULL 0
#else
#define FA_FULL (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH)
#endif


#pragma pack(pop)
#endif // _PRODIR_H
