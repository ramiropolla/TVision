/*------------------------------------------------------------*/
/* filename -       tradiobu.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TRadioButton member functions             */
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

#define Uses_TRadioButtons
#include <tv.h>

void TRadioButtons::draw()
{
    drawBox( button, 7 );
}

Boolean TRadioButtons::mark( int item )
{
    return Boolean( item == value );
}

void TRadioButtons::press( int item )
{
    value = (ushort)item;
    TCluster::press(item);
}

void TRadioButtons::movedTo( int item )
{
    value = (ushort)item;
}

void TRadioButtons::setData( void * rec )
{
    TCluster::setData(rec);
    sel = value;
}

void TRadioButtons::setItem( int item, Boolean on )
{
    if ( on )
        press(item);
}

#ifndef NO_TV_STREAMS
TStreamable *TRadioButtons::build()
{
    return new TRadioButtons( streamableInit );
}

TRadioButtons::TRadioButtons( StreamableInit ) : TCluster( streamableInit )
{
}
#endif  // ifndef NO_TV_STREAMS
