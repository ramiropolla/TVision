/*------------------------------------------------------------*/
/* filename -       tcheckbo.cpp                              */
/*                                                            */
/* function(s)                                                */
/*          TCheckBox member functions                        */
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

#define Uses_TCheckBoxes
#include <tv.h>

void TCheckBoxes::draw()
{
    drawBox( button, 'X' );
}

Boolean TCheckBoxes::mark(int item)
{
    return Boolean( (value & (1 <<  item)) != 0 );
}

void TCheckBoxes::press(int item)
{
    value = ushort(value^(1 << item));
    TCluster::press(item);
}

void TCheckBoxes::setItem( int item, Boolean on )
{
    ushort mask = 1 << item;
    if ( on )
      value |= mask;
    else
      value &= ~mask;
}

#ifndef NO_TV_STREAMS
TStreamable *TCheckBoxes::build()
{
    return new TCheckBoxes( streamableInit );
}

TCheckBoxes::TCheckBoxes( StreamableInit ) : TCluster( streamableInit )
{
}
#endif  // ifndef NO_TV_STREAMS
