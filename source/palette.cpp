/*------------------------------------------------------------*/
/* filename -       palette.cpp                               */
/*                                                            */
/* function(s)                                                */
/*                  TPalette member functions                 */
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

#define Uses_TPalette
#include <tv.h>

TPalette::TPalette( const char* d, ushort len ) :
    data( new char[ len+1 ] )
{
    data[0] = (uchar)len;
    memcpy( data+1, d, len );
}

TPalette::TPalette( const TPalette& tp ) :
    data( new char[ tp.data[0]+1 ] )
{
    memcpy( data, tp.data, tp.data[0]+1 );
}

TPalette::~TPalette()
{
    delete[] data;
}

TPalette& TPalette::operator = ( const TPalette& tp )
{
    if( data != tp.data )
        {
        if( data[0] != tp.data[0] )
            {
            delete data;
            data = new char[ tp.data[0]+1 ];
            data[0] = tp.data[0];
            }
        memcpy( data+1, tp.data+1, data[0] );
        }
    return *this;
}

char& TPalette::operator[]( int index ) const
{
    return data[index];
}
