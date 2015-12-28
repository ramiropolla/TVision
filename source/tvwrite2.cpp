// TVWRITE.CPP
// Copyright 1993 by J”rn Sierwald

#define Uses_TScreen
#define Uses_TGroup
#define Uses_TEvent
#define Uses_TThreaded
#include <tv.h>

#include <stdio.h>

extern TPoint shadowSize;
extern uchar shadowAttr;

struct StaticVars1 {
  const void*   buf;
};

struct StaticVars2 {
  TView*        target;
  short         offset;
  short         y;
};

static StaticVars1 staticVars1;
static StaticVars2 staticVars2;


void TView::writeViewRec1(short x1, short x2, TView* p, int shadowCounter ) {
  while (1) {
/*20*/
    p=p->next;
    if (p==staticVars2.target) { // alle durch
      // printit!
      if (p->owner->buffer) {
// !!! Mouse: Cursor must be off if the buffer is the physical screen.
#ifdef __DOS32__
         if ( p->owner->buffer == TScreen::screenBuffer )
           TMouse::hide();
#endif
        int y         = staticVars2.y;
        ushort *src   = (ushort*)staticVars1.buf + (x1-staticVars2.offset);
        size_t offset = p->owner->size.x * y  + x1;
        ushort *dst   = p->owner->buffer + offset;
        int len       = x2 - x1;
        if ( shadowCounter == 0 )
        {
          memmove(dst, src, len*2 );
        }
        else
        { // paint with shadowAttr
          for ( int l=0; l < len; l++ )
            dst[l] = ((*src++) & 0xFF) | (shadowAttr << 8);
        }
#ifdef __DOS32__
         if ( p->owner->buffer == TScreen::screenBuffer )
           TMouse::show();
#endif
#ifdef __OS2__
        if ( p->owner->buffer == TScreen::screenBuffer ) {
          TMouse::hide(TRect(x1, y, x2, y+1));
          VioShowBuf( offset*2, len*2, 0);
          TMouse::show();
        }
#endif
#ifdef __NT__
        if ( p->owner->buffer == TScreen::screenBuffer )
        {
          int x = x1;
          uchar *start = (uchar *)dst;
          SMALL_RECT to = {SHORT(x),SHORT(y),SHORT(x+len-1),SHORT(y)};
          CHAR_INFO cbuf[maxViewWidth];
          register CHAR_INFO *cbufp = cbuf;
          for ( int i=0; i < len; i++,cbufp++ ) {
            cbufp->Char.AsciiChar = *start++;
            cbufp->Attributes     = *start++;
          }
          if ( TThreads::my_console )
          {
            COORD bsize = {SHORT(len),1};
            static COORD from = {0,0};
            WriteConsoleOutput(TThreads::chandle[cnOutput],cbuf,bsize,from,&to);
          }
        }
#endif
#ifdef __UNIX__
        if ( p->owner->buffer == TScreen::screenBuffer )
        {
          TScreen::writeRow(offset, dst, len);
        }
#endif
      }
      if (p->owner->lockFlag==0) writeViewRec2( x1, x2, p->owner, shadowCounter );
      return ; // (p->owner->lockFlag==0);
    }
    if ( !(p->state & sfVisible) || staticVars2.y<p->origin.y) continue; // keine Verdeckung

    if ( staticVars2.y<p->origin.y+p->size.y ) {
      // šberdeckung m”glich.
      if (x1<p->origin.x) { // f„ngt links vom Object an.
        if (x2<=p->origin.x) continue; // links vorbei
        writeViewRec1( x1, short(p->origin.x), p, shadowCounter );
        x1 = short(p->origin.x);
      }
                   //  if (x1>=p->origin.x) {
      if ( x2<=p->origin.x+p->size.x ) return; // komplett verdeckt.
      if ( x1<p->origin.x+p->size.x ) x1 = short(p->origin.x+p->size.x);
                  // if ( x1>=p->origin.x+p->size.x ) { // k”nnte h”chstens im Schatten liegen
      if ( (p->state & sfShadow) && (staticVars2.y>=p->origin.y+shadowSize.y)) {
        if (x1>=p->origin.x+p->size.x+shadowSize.x) {
          continue; // rechts vorbei
        } else {
          shadowCounter++;
          if (x2<=p->origin.x+p->size.x+shadowSize.x) {
            continue; // alles im Schatten
          } else { // aufteilen Schattenteil, rechts daneben
            writeViewRec1( x1, short(p->origin.x+p->size.x+shadowSize.x), p, shadowCounter );
            x1 = short(p->origin.x+p->size.x+shadowSize.x);
            shadowCounter--;
            continue;
          }
        }
      } else {
        continue; // rechts vorbei, 1.Zeile hat keinen Schatten
      }
    }
    if ( (p->state & sfShadow) && (staticVars2.y < p->origin.y+p->size.y+shadowSize.y) ) {
      // im y-Schatten von Object?
      if (x1<p->origin.x+shadowSize.x) {
        if (x2<= p->origin.x+shadowSize.x) continue; // links vorbei
        writeViewRec1( x1, short(p->origin.x+shadowSize.x), p, shadowCounter );
        x1 = short(p->origin.x+shadowSize.x);
      }
      if (x1>=p->origin.x+shadowSize.x+p->size.x) continue;
      shadowCounter++;
      if (x2<=p->origin.x+p->size.x+shadowSize.x) {
        continue; // alles im Schatten
      } else { // aufteilen Schattenteil, rechts daneben
        writeViewRec1( x1, short(p->origin.x+p->size.x+shadowSize.x), p, shadowCounter );
        x1 = short(p->origin.x+p->size.x+shadowSize.x);
        shadowCounter--;
        continue;
      }

    } else { // zu weit unten
      continue;
    }

  } // while

}

void TView::writeViewRec2( short x1, short x2, TView* p, int shadowCounter ) {
  if (!(p->state & sfVisible) || p->owner==0 ) return;

  StaticVars2 savedStatics = staticVars2;

  staticVars2.y = short(staticVars2.y + p->origin.y);
  x1 = short(x1 + p->origin.x);
  x2 = short(x2 + p->origin.x);
  staticVars2.offset = short(staticVars2.offset + p->origin.x);
  staticVars2.target=p;

  TGroup* g=p->owner;
  if (staticVars2.y<g->clip.a.y || staticVars2.y >= g->clip.b.y) {
    staticVars2 = savedStatics;
    return;
  }
  if (x1<g->clip.a.x) x1 = short(g->clip.a.x);
  if (x2>g->clip.b.x) x2 = short(g->clip.b.x);
  if (x1>=x2) {
    staticVars2 = savedStatics;
    return;
  }

  writeViewRec1( x1, x2, g->last, shadowCounter );
  staticVars2 = savedStatics;
}

void TView::writeView( short x1, short x2, short y, const void* buf ) {
//  cerr << "Output ";
  if (y<0 || y>=size.y) return;
  if (x1<0) x1=0;
  if (x2>size.x) x2 = short(size.x);
  if (x1>=x2) return;
  staticVars2.offset=x1;
  staticVars1.buf=buf;
  staticVars2.y=y;

  writeViewRec2( x1, x2, this, 0 );
}

void TV_CDECL TView::writeBuf( short x, short y, short w, short h, const void *buf) {
  for (int i=0; i<h; i++) {
    writeView( x,x+w,short(y+i),(short*) buf + w*i );
  } /* endfor */
}

void TV_CDECL TView::writeChar( short x, short y, char c, uchar color, short count) {
  ushort b[maxViewWidth];
  ushort myChar= ( ((ushort)mapColor(color))<<8 ) + (unsigned char) c;
  short count2=count;
  if (x<0) x=0;
  if ((size_t)(x+count)>maxViewWidth) return;
  ushort* p = b;
  while ( count-- ) *p++ = myChar;
  writeView( x, x+count2, y, b);
}

void TV_CDECL TView::writeLine( short x, short y, short w, short h, const void *buf) {
  if (h==0) return;
  for (int i=0; i<h; i++) {
    writeView( x, x+w, short(y+i), buf );
  }
}

void TV_CDECL TView::writeStr( short x, short y, const char *str, uchar color) {
  if (!str) return;
  ushort l= (ushort)strlen(str);
  if (l==0) return;
  if ( l > maxViewWidth ) l = maxViewWidth;
  ushort l2=l;
  ushort myColor=( (ushort)mapColor(color) ) << 8;
  ushort b[maxViewWidth];
  ushort* p = b;
  while ( *p++ = myColor+(*(const unsigned char*)str++), --l );
  writeView( x, x+l2, y, b );
}
