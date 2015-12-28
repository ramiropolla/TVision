#ifndef __TVDIR_H
#define __TVDIR_H

#ifdef __LINUX__
#include <dirent.h>
#include <sys/stat.h>
#ifdef __MACOSX__
#include <limits.h>
#endif
#define QMAXPATH PATH_MAX
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

extern "C" int findfirst(const char *fname, ffblk *blk, int attr);
extern "C" int findnext(ffblk *blk);
extern "C" void findclose(ffblk *blk);
#define MAXPATH               QMAXPATH
#define MAXDIR                QMAXPATH
#define MAXFILE               QMAXPATH
#define MAXEXT                QMAXPATH
#define FA_ARCH		      0x01
#define FA_DIREC              0x02
#define FA_RDONLY	      0x04
#define __FAT__               0
#define DIRCHAR               '/'
#define SDIRCHAR              "/"
#else
#define __FAT__               1
#define DIRCHAR               '\\'
#define SDIRCHAR              "\\"
#if __BORLANDC__ < 0x0540
#  define findclose(p)
#endif
#  include <dir.h>
#endif

#endif // define __TVDIR_H
