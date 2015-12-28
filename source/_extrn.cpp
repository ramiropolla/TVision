#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <ctype.h>
#ifdef __BORLANDC__
#include <mem.h>
#endif
#include <_extrn.h>

//--------------------------------------------------------------------------
char *qstrncpy(char *dst, const char *src, size_t dstsize)
{
  if((ssize_t)dstsize > 0) {
    char *end = dst + dstsize;
    char *ptr = dst;
    while(ptr < end) if((*ptr++ = *src++) == '\0') goto done;
    end[-1] = '\0';
  }
done:
  return dst;
}

//--------------------------------------------------------------------------
char *qstrncat(char *dst, const char *src, size_t dstsize)
{
  if((ssize_t)dstsize > 0) {
    char *end = dst + dstsize;
    char *ptr = strchr(dst, '\0');
    while(ptr < end) if((*ptr++ = *src++) == '\0') goto done;
    end[-1] = '\0';
  }
done:
  return dst;
}

//=======================================================================
#ifdef _UNICODE
#error "Unicode not supported yet!"
#endif

static const char *fix_format(const char *fmt, char *buf, size_t bufsize);

#if !defined(__GNUC__) && !defined(_MSC_VER) && !defined(__BORLANDC__)

static size_t cur_ost;

#ifdef __BORLANDC__
typedef unsigned __cdecl putnF(const char *, unsigned n, void *fp);

#ifdef __cplusplus
extern "C" {
#endif
int __cdecl __vprinter(putnF *, void *, const char *, int, int, va_list);
#ifdef __cplusplus
}
#endif

static size_t __cdecl strputn(char *S, size_t n, char **bufPP, int /*ok*/)
{
  if(n > cur_ost) n = cur_ost;
  if(n) {
    memcpy(*bufPP, S, n);
    *bufPP += n;
    **bufPP = '\0';
    cur_ost -= n;
  }
  return(n);
}

#elif defined(__WATCOMC__)

struct xxx {
  char  *ptr; //0
  int32  x1;   //4
  int32  x2;   //8
  int32  x3;   //C
  int32  cnt;  //10
};

static void mem_putc(xxx *xs, char c)
{
  if(cur_ost) {
    *xs->ptr++ = c;
    ++xs->cnt;
    --cur_ost;
  }
}

#ifdef __cplusplus
extern "C" {
#endif
int __prtf(char *, const char *, va_list, void (*)(xxx *, char));
#ifdef __cplusplus
}
#endif

#else //WATCOMC
#error "Unsupported compiler!"
#endif

//=======================================================================
int qvsnprintf(char *bufP, size_t max, const char *fmt, va_list ap)
{
  int  ret;
#ifdef __BORLANDC__
  int xxx = cur_ost;
#endif

  *bufP = '\0';
  if((int)max <= 1) return 0;
  cur_ost = max - 1;

  size_t len = strlen(fmt) + 100;
  char *buf = (char *)alloca(len);
  fmt = fix_format(fmt, buf, len);
  char *saved = bufP;

#ifdef __BORLANDC__
  __vprinter((putnF*)strputn, &bufP, fmt, 0, cur_ost, ap);
//  if ( !cur_ost ) ret = -1;
  cur_ost = xxx;

#elif defined(__WATCOMC__)
  ret = __prtf(bufP, fmt, ap, mem_putc);
  bufP[ret] = '\0';

#else
#error "Unsupported compiler!"
#endif
  return strlen(saved);
}

#else
//=======================================================================
// common version if snprintf exists:

#ifdef _MSC_VER
#define vsnprintf _vsnprintf
static int vfscanf(FILE *fp, const char *format, va_list va);
#endif

int qvsnprintf(char *bufP, size_t max, const char *format, va_list ap)
{
  if((ssize_t)max <= 1) return(0);    // we need a terminating zero
  size_t len = strlen(format) + 100;
  char *buf = (char *)alloca(len);
  format = fix_format(format, buf, len);
  int code = vsnprintf(bufP, max, format, ap);
  if(code < 0 || (size_t)code >= max) { // not enough space or error
    code = (int)(max - 1);
    bufP[code] = '\0';                  // put terminating zero!
  }
  return(code >= 0 ? code : (int)strlen(bufP));
}

#endif // !defined(__GNUC__) && !defined(_MSC_VER) && !defined(SNPRINTF_PRESENT)

static const char modifs[] = "+-0123456789*#.";

//=======================================================================
// New format specifier: %a - ea_t
// for 32bits, %a should be replaced by %lX
// for 64bits, %a should be replaced by %LX
static const char *fix_format(const char *fmt, char *buf, size_t bufsize)
{
  const char *ptr = strchr(fmt, '%');

  if(!ptr) return(fmt);

  ptr = fmt;
  char *out = buf;
  char *const end = buf + bufsize;
  for( ; ; ) {
    char c = *ptr++;
    if(out < end) *out++ = c;
    switch(c) {
      case '%':
        if(*ptr != '%') while(strchr(modifs, *ptr)) {
          if(out < end) *out++ = *ptr;
          ++ptr;
        }
        if(out < end) *out++ = *ptr;
        if(!*++ptr) {
      case '\0':
          return(buf);
        }
      default:
        continue;
    }
  }
}

//-------------------------------------------------------------------------
int qsnprintf(char *buffer, size_t n, const char *format, ...)
{
  va_list va;
  va_start(va, format);
  int code = qvsnprintf(buffer, n, format, va);
  va_end(va);
  return(code);
}

//-------------------------------------------------------------------------
