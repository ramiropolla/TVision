/*------------------------------------------------------------*/
/* filename -       tparamte.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TParamText member functions               */
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

#define Uses_TParamText
#include <tv.h>

#include <stdio.h>
#include <stdarg.h>

TParamText::TParamText( const TRect& bounds,
                        const char *aText,
                        int aParamCount ) :
    TStaticText(bounds, aText),
    paramCount( (short)aParamCount ),
    paramList( 0 )
{
}

size_t TParamText::dataSize()
{
    return paramCount * sizeof(int32);
}

void TParamText::getText( char *str, size_t strsize )
{
  if ( ssize_t(strsize) > 0 )
  {
    if( text == 0 )
      *str = EOS;
    else
      qvsnprintf( str, strsize, text, *(va_list*)&paramList );
  }
}

void TParamText::setData( void *rec )
{
    paramList = rec;
}

#ifndef NO_TV_STREAMS
void TParamText::write( opstream& os )
{
    TStaticText::write( os );
    os << paramCount;
}

void *TParamText::read( ipstream& is )
{
    TStaticText::read( is );
    is >> paramCount;
    paramList = 0;
    return this;
}

TStreamable *TParamText::build()
{
    return new TParamText( streamableInit );
}

TParamText::TParamText( StreamableInit ) : TStaticText( streamableInit )
{
}
#endif  // ifndef NO_TV_STREAMS


