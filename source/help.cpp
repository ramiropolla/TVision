/*------------------------------------------------------------*/
/* filename -       Help.cpp                                  */
/*                                                            */
/* function(s)                                                */
/*                  Member function(s) of following classes   */
/*                      THelpViewer                           */
/*                      THelpWindow                           */
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

#define Uses_TStreamableClass
#define Uses_TPoint
#define Uses_TStreamable
#define Uses_ipstream
#define Uses_opstream
#define Uses_fpstream
#define Uses_TRect
#define Uses_TScrollBar
#define Uses_TScroller
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TWindow
#define Uses_TKeys
#define Uses_TPalette
#include <tv.h>
#include "tvhelp.h"
#include "util.h"
#include <limits.h>
#include <sys/stat.h>

#ifdef __IDA__
#include <pro.h>
#endif

int tv_helpIndex;
int tv_idcIndex;
int tv_helponhelp;

// THelpViewer

static ushort curtop;           // current topic
static helppos_t olds[20];      // array of old topics
static int ntopics = 0;         // number of old topics

static void save_topic(ushort curtop,ushort selected,int x,int y) {
  if ( ntopics == qnumber(olds) ) {
    memmove(olds,&olds[1],sizeof(olds)-sizeof(olds[0]));
    ntopics--;
  }
  olds[ntopics  ].topic = curtop;
  olds[ntopics  ].selected = selected;
  olds[ntopics  ].delta.x = x;
  olds[ntopics++].delta.y = y;
}

THelpViewer::THelpViewer( const TRect& bounds, TScrollBar* aHScrollBar,
    TScrollBar* aVScrollBar, THelpFile *aHelpFile, ushort context )
    : TScroller( bounds, aHScrollBar, aVScrollBar )
{
    options = (options | ofSelectable);
    growMode = gfGrowHiX | gfGrowHiY;
    hFile = aHelpFile;
    topic = aHelpFile->getTopic(context);
    topic->setWidth(size.x);
    setLimit(78, topic->numLines());
    selected = 1;
    curtop = context;
}

THelpViewer::~THelpViewer()
{
    save_topic(curtop,(ushort)selected,delta.x,delta.y);
    delete hFile;
    delete topic;
}

void THelpViewer::changeBounds( const TRect& bounds )
{
    TScroller::changeBounds(bounds);
    topic->setWidth(size.x);
    setLimit(limit.x, topic->numLines());
}

void THelpViewer::draw()
{
    TDrawBuffer b;
    char line[maxViewWidth];
    char buffer[maxViewWidth];
    char *bufPtr;
    int i, j, l;
    int keyCount;
    ushort normal, keyword, selKeyword, c;
    TPoint keyPoint;
    uchar keyLength = 0;
    int keyRef;
    int nxrefs = topic->getNumCrossRefs();

    normal = getColor(1);
    keyword = getColor(2);
    selKeyword = getColor(3);
    keyCount = 0;
    keyPoint.x = 0;
    keyPoint.y = 0;
//    topic->setWidth(size.x);
    if (nxrefs > 0) {
      do
        topic->getCrossRef(keyCount++, keyPoint, keyLength, keyRef);
      while ( (keyCount <= nxrefs) && (keyPoint.y <= delta.y) );
      if ( selected < keyCount ) {
        selected = keyCount;
      } else {
        TPoint keyP = keyPoint;
        uchar keyL;
        int keyR;
        int maxk = keyCount;                    // number of last ref
        int maxy = delta.y + size.y;
        while ( (maxk < nxrefs) && (keyP.y <= maxy) )     // ig 25.02.94
          topic->getCrossRef(maxk++, keyP, keyL, keyR);
        if ( keyP.y > maxy ) maxk--;
        if ( selected > maxk ) selected = maxk;
      }
    }
    for (i = 1; i <= size.y; ++i)
        {
        b.moveChar(0, ' ', normal, ushort(size.x));
        topic->getLine(i + delta.y, line, sizeof(line));
        if ((int)strlen(line) > delta.x)
            {
            bufPtr = line + delta.x;
            qstrncpy(buffer, bufPtr, qmin(size.x+1, sizeof(buffer)));
            b.moveStr(0, buffer, normal);
            }
        else
            b.moveStr(0, "", normal);
        while (i + delta.y == keyPoint.y)
            {
            l = keyLength;
            if (keyPoint.x < delta.x )
                {
                l -= (delta.x - keyPoint.x);
                keyPoint.x = delta.x;
                }
            if (keyCount == selected)
                c = selKeyword;
            else
                c = keyword;
            for(j = 0; j < l; ++j)
                b.putAttribute(ushort(keyPoint.x - delta.x + j),c);
            if (keyCount < nxrefs)
                topic->getCrossRef(keyCount++, keyPoint, keyLength, keyRef);
            else
                keyPoint.y = 0;
            }
        writeLine(0, ushort(i-1), ushort(size.x), 1, b);
        }
}

TPalette& THelpViewer::getPalette() const
{
    static TPalette palette(cHelpViewer, sizeof( cHelpViewer)-1);
    return palette;
}

void THelpViewer::makeSelectVisible( int selected, TPoint& keyPoint,
         uchar& keyLength, int& keyRef )
{
    TPoint d;

    topic->getCrossRef(selected, keyPoint, keyLength, keyRef);
    d = delta;
    if (keyPoint.x < d.x)
        d.x = keyPoint.x;
    if (keyPoint.x > d.x + size.x)
        d.x = keyPoint.x - size.x;
    if (keyPoint.y <= d.y)              // ig 24.02.94
        d.y = keyPoint.y-1;             // ig 24.02.94
    if (keyPoint.y > d.y + size.y)
        d.y = keyPoint.y - size.y;
    if ((d.x != delta.x) || (d.y != delta.y))
         scrollTo(d.x, d.y);
}

void THelpViewer::switchToTopic( int keyRef, const TPoint &d, int s )
{
    if (topic != 0)
      delete topic;
    curtop = (ushort)keyRef;
    topic = hFile->getTopic(keyRef);
    topic->setWidth(size.x);
    scrollTo(d.x, d.y);
    scrollDraw();
    delta = d;
    setLimit(limit.x, topic->numLines());
    selected = s;
    drawView();
}

void THelpViewer::handleEvent( TEvent& event )
{

    TPoint keyPoint, mouse;
    uchar keyLength;
    int keyRef = 0;
    int keyCount;


    TScroller::handleEvent(event);
    switch (event.what)
        {

        case evKeyDown:
            switch (event.keyDown.keyCode)
                {
                case kbTab:
                    ++selected;
                    if (selected > topic->getNumCrossRefs())
                        selected = 1;
                    if ( topic->getNumCrossRefs() != 0 )
                        makeSelectVisible(selected-1,keyPoint,keyLength,keyRef);
                    break;
                case kbShiftTab:
                    --selected;
                    if (selected == 0)
                        selected = topic->getNumCrossRefs();
                    if ( topic->getNumCrossRefs() != 0 )
                        makeSelectVisible(selected-1,keyPoint,keyLength,keyRef);
                    break;
                case kbF1:
HHelp:
                    keyRef = tv_helponhelp;
                    goto SwitchSaving;
                case kbShiftF1:
                    keyRef = tv_helpIndex;
                    goto SwitchSaving;
                case kbCtrlF1:
                    keyRef = tv_idcIndex;
                    goto SwitchSaving;
                case kbEnter:
                    if (selected <= topic->getNumCrossRefs()) {
                      topic->getCrossRef(selected-1, keyPoint, keyLength, keyRef);
SwitchSaving:
                      if ( curtop != keyRef ) {
                        save_topic(curtop,(ushort)selected,delta.x,delta.y);
                        TPoint z;
                        z.x = z.y = 0;
                        switchToTopic(keyRef,z,1);
                      }
                    }
                    break;
                case kbEsc:
                    event.what = evCommand;
                    event.message.command = cmClose;
                    putEvent(event);
                    break;
                case kbAltF1:
                case kbBack:
                    if ( ntopics > 0 ) {
                      ntopics--;
                      switchToTopic(olds[ntopics].topic,olds[ntopics].delta,olds[ntopics].selected);
                    }
                    break;
                case kbF5:
                    event.what = evCommand;
                    event.message.command = cmZoom;
                    putEvent(event);
                    break;
                default:
                    if ( keyRef != tv_helpIndex ) return;
                }
            drawView();
            clearEvent(event);
            break;

        case evMouseDown:
            mouse = makeLocal(event.mouse.where);
            mouse.x += delta.x;
            mouse.y += delta.y;
            keyCount = 0;

            do
            {
                ++keyCount;
                if (keyCount > topic->getNumCrossRefs())
                    return;
                topic->getCrossRef(keyCount-1, keyPoint, keyLength, keyRef);
            } while (!((keyPoint.y == mouse.y+1) && (mouse.x >= keyPoint.x) &&
                  (mouse.x < keyPoint.x + keyLength)));
            selected = keyCount;
            drawView();
            if ( event.mouse.doubleClick() )
              goto SwitchSaving;
            clearEvent(event);
            break;

        case evCommand:
            if ( event.message.command == cmHelp ) goto HHelp;
            if ((event.message.command == cmClose) && ((owner->state & sfModal) != 0))
                {
                endModal(cmClose);
                clearEvent(event);
                }
            break;
        }
}

// THelpWindow

THelpWindow::THelpWindow( TRect &b, THelpFile *hFile, ushort context ):
       TWindow( b, "Backspace=last Shift-F1=Index Esc=return", wnNoNumber ),
       TWindowInit(THelpWindow::initFrame)
{

//    options = (options | ofCentered);
    b.grow(-2,-1);
    b.move(-b.a.x+1,-b.a.y+1);
    insert(new THelpViewer (b,
      standardScrollBar(sbHorizontal | sbHandleKeyboard),
      standardScrollBar(sbVertical | sbHandleKeyboard), hFile, context));
}

TPalette& THelpWindow::getPalette() const
{
    static TPalette palette(cHelpWindow, sizeof( cHelpWindow)-1);
    return palette;
}
