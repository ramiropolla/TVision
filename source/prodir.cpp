#include <tvdir.h>

#ifdef __GNUC__

#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
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
int findfirst(const char *fname, ffblk *blk, int attr)
{
  strncpy(blk->dirpath, fname, sizeof(blk->dirpath));
  blk->dirpath[sizeof(blk->dirpath)-1] = '\0';
  char *file = strrchr(blk->dirpath, DIRCHAR);
  if ( file == NULL )
    strcpy(blk->dirpath, ".");
  else
    *file++ = '\0';

  blk->filelist = NULL;
  blk->fileqty = scandir(blk->dirpath, (dirent ***)&blk->filelist, NULL, alphasort);
  blk->fileidx = 0;
  strncpy(blk->pattern, file != NULL ? file : fname, sizeof(blk->pattern));
  blk->pattern[sizeof(blk->pattern)-1] = '\0';
  blk->attr = attr;
  return findnext(blk);
}

//--------------------------------------------------------------------------
int findnext(ffblk *blk)
{
  while ( blk->fileidx < blk->fileqty )
  {
    dirent *de = getfile(blk, blk->fileidx++);
    int code = fnmatch(blk->pattern, de->d_name, FNM_PATHNAME|FNM_PERIOD|FNM_CASEFOLD);
//    printf("pattern=%s name=%s code=%d %s\n", blk->pattern, de->d_name, code, strerror(code));
    if ( code == 0 )
    { // match
      struct stat st;
      char fullpath[QMAXPATH];
      strcpy(fullpath, blk->dirpath);
      char *end = strchr(fullpath, '\0');
      if ( end[-1] != DIRCHAR )
        *end++ = DIRCHAR;
      strcpy(end, de->d_name);
//      printf("stat: '%s'\n", fullpath);
      if ( stat(fullpath, &st) != 0 )
        return -1;

//      printf("isdir?\n");
      if ( (blk->attr & FA_DIREC) == 0 && S_ISDIR(st.st_mode) )
        continue;

//      printf("all ok\n");
      strncpy(blk->ff_name, de->d_name, sizeof(blk->ff_name));
      blk->ff_name[sizeof(blk->ff_name)-1] = '\0';
      blk->ff_attrib = st.st_mode;
      return 0;
    }
  }
  return -1;
}

//--------------------------------------------------------------------------
void findclose(ffblk *blk)
{
  for ( int i=0; i < blk->fileqty; i++ )
    free(((dirent**)blk->filelist)[i]);
  free(blk->filelist);
}

#endif
