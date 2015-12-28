#include <pro.h>
#include <prodir.h>

#ifdef __GNUC__

#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
#include <string.h>
#include <sys/stat.h>
#ifdef __MACOSX__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

//--------------------------------------------------------------------------
inline dirent *getfile(ffblk *blk, int idx)
{
  return ((dirent **)blk->filelist)[idx];
}

//--------------------------------------------------------------------------
int ida_export findfirst(const char *fname, ffblk *blk, int attr)
{
  qstrncpy(blk->dirpath, fname, sizeof(blk->dirpath));
  char *file = strrchr(blk->dirpath, DIRCHAR);
  if ( file == NULL )
    qstrncpy(blk->dirpath, ".", sizeof(blk->dirpath));
  else
    *file++ = '\0';

  blk->filelist = NULL;
  blk->fileqty = scandir(blk->dirpath, (dirent ***)&blk->filelist, NULL, alphasort);
  blk->fileidx = 0;
  qstrncpy(blk->pattern, file != NULL ? file : fname, sizeof(blk->pattern));
  blk->attr = attr;
  return findnext(blk);
}

//--------------------------------------------------------------------------
int ida_export findnext(ffblk *blk)
{
  while ( blk->fileidx < blk->fileqty )
  {
    dirent *de = getfile(blk, blk->fileidx++);
    int code = fnmatch(blk->pattern, de->d_name, FNM_PATHNAME|FNM_PERIOD|FNM_CASEFOLD);
    if ( code == 0 )
    { // match
      qstatbuf st;
      char fullpath[QMAXPATH];
      qmakepath(fullpath, sizeof(fullpath), blk->dirpath, de->d_name, NULL);
      if ( qstat(fullpath, &st) != 0 )
        continue;

      if ( (blk->attr & FA_DIREC) == 0 && S_ISDIR(st.st_mode) )
        continue;

      qstrncpy(blk->ff_name, de->d_name, sizeof(blk->ff_name));
      blk->ff_attrib = st.st_mode;
      return 0;
    }
  }
  return -1;
}

//--------------------------------------------------------------------------
void ida_export findclose(ffblk *blk)
{
  for ( int i=0; i < blk->fileqty; i++ )
    free(((dirent**)blk->filelist)[i]);
  free(blk->filelist);
}

#endif
