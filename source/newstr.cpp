/*------------------------------------------------------------*/
/* filename -       newstr.cpp                                */
/*                                                            */
/* function(s)                                                */
/*                  newStr member function                    */
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


#include <tv.h>

char *newStr( const char *s )
{
    if( s == 0 )
        return 0;
    size_t size = strlen(s) + 1;
    char *temp = new char[ size ];
    memcpy( temp, s, size );
    return temp;
}
