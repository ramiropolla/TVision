/*------------------------------------------------------------*/
/* filename -       new.cpp                                   */
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

#include <assert.h>
#ifdef __BORLANDC__
#include <alloc.h>
#endif
#ifdef __WATCOMC__
#include <malloc.h>
#endif

#define Uses_TVMemMgr
#include <tv.h>

TBufListEntry *TBufListEntry::bufList = 0;

TBufListEntry::TBufListEntry( void*& o ) : owner( o )
{
    next = bufList;
    prev = 0;
    bufList = this;
    if( next != 0 )
        next->prev = this;
}

TBufListEntry::~TBufListEntry()
{
    owner = 0;
    if( prev == 0 )
        bufList = next;
    else
        prev->next = next;
    if( next != 0 )
        next->prev = prev;
}

void *TBufListEntry::operator new( size_t sz, size_t extra )
{
    return malloc( sz + extra );        // 25.01.96 ig (by Denis Petroff, 2:5030/287.14)
}

void *TBufListEntry::operator new( size_t ) throw()
{
    return 0;
}

void TBufListEntry::operator delete( void *b )
{
    free( b );
}

Boolean TBufListEntry::freeHead()
{
    if( bufList == 0 )
        return False;
    else
        {
        delete bufList;
        return True;
        }
}

void *TVMemMgr::safetyPool = 0;
size_t TVMemMgr::safetyPoolSize = 0;
int TVMemMgr::inited = 0;

TVMemMgr memMgr;

TVMemMgr::TVMemMgr()
{
    if( !inited )
        resizeSafetyPool();
}

void TVMemMgr::resizeSafetyPool( size_t sz )
{
    inited = 1;
    free( safetyPool );
    if( sz == 0 )
        safetyPool = 0;
    else
        safetyPool = malloc( sz );
    safetyPoolSize = sz;
}

int TVMemMgr::safetyPoolExhausted()
{
    return inited && (safetyPool == 0);
}

void TVMemMgr::allocateDiscardable( void *&adr, size_t sz )
{
    if( safetyPoolExhausted() )
        adr = 0;
    else
        {
        TBufListEntry *newEntry = new( sz ) TBufListEntry( adr );
        if( newEntry == 0 )
            adr = 0;
        else
            adr = (char *)newEntry + sizeof(TBufListEntry);
        }
}

void TVMemMgr::freeDiscardable( void *block )
{
    delete (TBufListEntry *)((char *)block - sizeof(TBufListEntry));
}

#if !defined( NDEBUG )
const size_t BLK_SIZE = 16;
const size_t BLK_DATA = 0xA6;
#else
const size_t BLK_SIZE = 0;
#endif
