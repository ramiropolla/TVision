/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   UTIL.H                                                                */
/*                                                                         */
/*   Copyright (c) Borland International 1991                              */
/*   All Rights Reserved.                                                  */
/*                                                                         */
/* ------------------------------------------------------------------------*/

#if !defined( __UTIL_H )
#define __UTIL_H

// Declarations of basic system independent functions
// This comes from IDA Pro

#ifdef __IDA__
#include <pro.h>
#else
#include <_extrn.h>
#endif

void fexpand( char *path, size_t pathsize );

uint32 getTicks();
// returns a value that can be used as a substitute for the DOS Ticker at [0040:006C]
unsigned char getShiftState();
// returns a value that can be used as a substitute for the shift state at [0040:0017]

#ifndef __BORLANDC__
int fnsplit(const char *__path,
            char *__drive,
            char *__dir,
            char *__name,
            char *__ext);
void fnmerge(char *__path,
             const char *__drive,
             const char *__dir,
             const char *__name,
             const char *__ext);
#define WILDCARDS 0x01
#define EXTENSION 0x02
#define FILENAME  0x04
#define DIRECTORY 0x08
#define DRIVE     0x10

int getdisk(void);

#endif

int qgetcurdir(int __drive, char *buffer, size_t bufsize);
char hotKey( const char *s );
ushort ctrlToArrow( ushort );
char getAltChar( ushort keyCode );
ushort getAltCode( char ch );

ushort historyCount( uchar id );
const char *historyStr( uchar id, int index );
void historyAdd( uchar id, const char * );

int cstrlen( const char * );

class TView;
void *message( TView *receiver, ushort what, ushort command, void *infoPtr );
Boolean lowMemory();

char *newStr( const char * );

Boolean driveValid( char drive );

Boolean isDir( const char *str );

Boolean pathValid( const char *path );

Boolean validFileName( const char *fileName );

void getCurDir( char *dir, size_t dirsize );

Boolean isWild( const char *f );


void expandPath(const char *path,
                char *dir,
                size_t dirsize,
                char *file,
                size_t filesize);

#endif  // __UTIL_H
