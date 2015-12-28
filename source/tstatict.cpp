/*------------------------------------------------------------*/
/* filename -       tstatict.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TStaticText member functions              */
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

#define Uses_TStaticText
#define Uses_TDrawBuffer
#define Uses_opstream
#define Uses_ipstream
#include <tv.h>

#define cpStaticText "\x06"

#define TEXTBUF_SIZE    (4096+1)

TStaticText::TStaticText( const TRect& bounds, const char *aText ) :
    TView( bounds ),
    text( newStr( aText ) )
{
}

TStaticText::~TStaticText()
{
    delete[] (char*)text;
}

void TStaticText::setText( const char *buf )
{
    delete[] (char *)text;
    text = newStr(buf);
}

void TStaticText::draw()
{
    uchar color;
    Boolean center;
    int i, j, l, p, y;
    TDrawBuffer b;
    char s[TEXTBUF_SIZE];

    color = (uchar)getColor(1);
    getText(s, sizeof(s));
    l = (int)strlen(s);
    p = 0;
    y = 0;
    center = False;
    while (y < size.y)
        {
        b.moveChar(0, ' ', color, ushort(size.x));
        if (p < l)
            {
            if (s[p] == 3)
                {
                center = True;
                ++p;
                }
            i = p;
            do {
               j = p;
               while ((p < l) && (s[p] == ' '))
                   ++p;
               while ((p < l) && (s[p] != ' ') && (s[p] != '\n'))
                   ++p;
               } while ((p < l) && (p < i + size.x) && (s[p] != '\n'));
            if (p > i + size.x)
                if (j > i)
                    p = j;
                else
                    p = i + size.x;
            if (center == True)
               j = (size.x - p + i) / 2 ;
            else
               j = 0;
            b.moveBuf(ushort(j), &s[i], color, ushort((p - i)));
            while ((p < l) && (s[p] == ' '))
                p++;
            if ((p < l) && (s[p] == '\n'))
                {
                center = False;
                p++;
                if ((p < l) && (s[p] == 10))
                    p++;
                }
            }
        writeLine(0, ushort(y++), ushort(size.x), 1, b);
        }
}

TPalette& TStaticText::getPalette() const
{
    static TPalette palette( cpStaticText, sizeof( cpStaticText )-1 );
    return palette;
}

void TStaticText::getText( char *str, size_t strsize )
{
  if ( ssize_t(strsize) > 0 )
  {
    if ( text == 0 )
      *str = EOS;
    else
      qstrncpy( str, text, strsize );
  }
}

#ifndef NO_TV_STREAMS
void TStaticText::write( opstream& os )
{
    TView::write( os );
    os.writeString( text );
}

void *TStaticText::read( ipstream& is )
{
    TView::read( is );
    text = is.readString();
    return this;
}

TStreamable *TStaticText::build()
{
    return new TStaticText( streamableInit );
}

TStaticText::TStaticText( StreamableInit ) : TView( streamableInit )
{
}
#endif  // ifndef NO_TV_STREAMS
