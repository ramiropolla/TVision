/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   TVHELP.H                                                              */
/*                                                                         */
/*   Copyright (c) Borland International 1991                              */
/*   All Rights Reserved.                                                  */
/*                                                                         */
/*   defines the classes THelpViewer and THelpWindow                       */
/*                                                                         */
//
//      Changed by Ilfak Guilfanov 03.01.94 for:
//              - previous help screen
//              - zooming
//
/* ------------------------------------------------------------------------*/

#if !defined( __TVHELP_H )
#define __TVHELP_H

#define Uses_TStreamable
#define Uses_ipstream
#define Uses_opstream
#define Uses_fpstream
#define Uses_TObject
#define Uses_TPoint
#define Uses_TRect
#define Uses_TEvent
#define Uses_TScroller
#define Uses_TScrollBar
#define Uses_TWindow
#include <tv.h>

#include <helpbase.h>

// THelpViewer

typedef struct {
  ushort topic;
  TPoint delta;
  int selected;
} helppos_t;

class THelpViewer : public TScroller
{
public:

    THelpViewer( const TRect&, TScrollBar*, TScrollBar*, THelpFile*, ushort );
    ~THelpViewer();

    virtual void changeBounds( const TRect& );
    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& );
    void makeSelectVisible( int, TPoint&, uchar&, int& );
    void switchToTopic( int, const TPoint&, int );

    THelpFile *hFile;
    THelpTopic *topic;
    int selected;

};

// THelpWindow

class THelpWindow : public TWindow
{
public:

    THelpWindow( TRect &b, THelpFile*, ushort );

    virtual TPalette& getPalette() const;
};


extern void notAssigned( opstream& s, int value );

extern TCrossRefHandler crossRefHandler;

#endif  // __TVHELP_H
