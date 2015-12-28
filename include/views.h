/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   VIEWS.H                                                               */
/*                                                                         */
/*   Copyright (c) Borland International 1991                              */
/*   All Rights Reserved.                                                  */
/*                                                                         */
/*   defines the classes TView, TFrame, TScrollBar, TScroller,             */
/*   TListViewer, TGroup, and TWindow                                      */
/*                                                                         */
/* ------------------------------------------------------------------------*/

#include <cm_codes.h>

#if !defined( __COMMAND_CODES )
#define __COMMAND_CODES

const ushort
//  TView State masks

    sfVisible       = 0x001,
    sfCursorVis     = 0x002,
    sfCursorIns     = 0x004,
    sfShadow        = 0x008,
    sfActive        = 0x010,
    sfSelected      = 0x020,
    sfFocused       = 0x040,
    sfDragging      = 0x080,
    sfDisabled      = 0x100,
    sfModal         = 0x200,
    sfDefault       = 0x400,
    sfExposed       = 0x800,

// TView Option masks

    ofSelectable    = 0x001,
    ofTopSelect     = 0x002,
    ofFirstClick    = 0x004,
    ofFramed        = 0x008,
    ofPreProcess    = 0x010,
    ofPostProcess   = 0x020,
    ofBuffered      = 0x040,
    ofTileable      = 0x080,
    ofCenterX       = 0x100,
    ofCenterY       = 0x200,
    ofCentered      = 0x300,

// TView GrowMode masks

    gfGrowLoX       = 0x01,
    gfGrowLoY       = 0x02,
    gfGrowHiX       = 0x04,
    gfGrowHiY       = 0x08,
    gfGrowAll       = 0x0f,
    gfGrowRel       = 0x10,

// TView DragMode masks

    dmDragMove      = 0x01,
    dmDragGrow      = 0x02,
    dmLimitLoX      = 0x10,
    dmLimitLoY      = 0x20,
    dmLimitHiX      = 0x40,
    dmLimitHiY      = 0x80,
    dmLimitAll      = dmLimitLoX | dmLimitLoY | dmLimitHiX | dmLimitHiY,

// TView Help context codes

    hcNoContext     = 0,
    hcDragging      = 1,

// TScrollBar part codes

    sbLeftArrow     = 0,
    sbRightArrow    = 1,
    sbPageLeft      = 2,
    sbPageRight     = 3,
    sbUpArrow       = 4,
    sbDownArrow     = 5,
    sbPageUp        = 6,
    sbPageDown      = 7,
    sbIndicator     = 8,

// TScrollBar options for TWindow.StandardScrollBar

    sbHorizontal    = 0x000,
    sbVertical      = 0x001,
    sbHandleKeyboard = 0x002,

// TWindow Flags masks

    wfMove          = 0x01,
    wfGrow          = 0x02,
    wfClose         = 0x04,
    wfZoom          = 0x08,

//  TView inhibit flags

    noMenuBar       = 0x0001,
    noDeskTop       = 0x0002,
    noStatusLine    = 0x0004,
    noBackground    = 0x0008,
    noFrame         = 0x0010,
    noViewer        = 0x0020,
    noHistory       = 0x0040,

// TWindow number constants

    wnNoNumber      = 0,

// TWindow palette entries

    wpBlueWindow    = 0,
    wpCyanWindow    = 1,
    wpGrayWindow    = 2,

//  Event masks

    positionalEvents    = evMouse,
    focusedEvents       = evKeyboard | evCommand;

#endif  // __COMMAND_CODES

#if defined( Uses_TCommandSet ) && !defined( __TCommandSet )
#define __TCommandSet

class TCommandSet
{

public:

    TCommandSet();
    TCommandSet( const TCommandSet& );

    Boolean has( int cmd );

    void disableCmd( int cmd );
    void enableCmd( int cmd );
    void operator += ( int cmd );
    void operator -= ( int cmd );

    void disableCmd( const TCommandSet& );
    void enableCmd( const TCommandSet& );
    void operator += ( const TCommandSet& );
    void operator -= ( const TCommandSet& );

    Boolean isEmpty();

    TCommandSet& operator &= ( const TCommandSet& );
    TCommandSet& operator |= ( const TCommandSet& );

    friend TCommandSet operator & ( const TCommandSet&, const TCommandSet& );
    friend TCommandSet operator | ( const TCommandSet&, const TCommandSet& );

    friend int operator == ( const TCommandSet& tc1, const TCommandSet& tc2 );
    friend int operator != ( const TCommandSet& tc1, const TCommandSet& tc2 );

private:

    int loc( int );
    int mask( int );

    static int  masks[8];

    uchar cmds[32];

};

inline void TCommandSet::operator += ( int cmd )
{
    enableCmd( cmd );
}

inline void TCommandSet::operator -= ( int cmd )
{
    disableCmd( cmd );
}

inline void TCommandSet::operator += ( const TCommandSet& tc )
{
    enableCmd( tc );
}

inline void TCommandSet::operator -= ( const TCommandSet& tc )
{
    disableCmd( tc );
}

inline int operator != ( const TCommandSet& tc1, const TCommandSet& tc2 )
{
    return !operator == ( tc1, tc2 );
}

inline int TCommandSet::loc( int cmd )
{
    return cmd / 8;
}

inline int TCommandSet::mask( int cmd )
{
    return masks[ cmd & 0x07 ];
}

#endif  // Uses_TCommandSet

#if defined( Uses_TPalette ) && !defined( __TPalette )
#define __TPalette

class TPalette
{

public:

    TPalette( const char *, ushort );
    TPalette( const TPalette& );
    ~TPalette();

    TPalette& operator = ( const TPalette& );

    char& operator[]( int ) const;

    char *data;

};

#endif  // Uses_TPalette

#if defined( Uses_TView ) && !defined( __TView )
#define __TView

struct write_args
{
    void *self;
    void *target;
    void *buf;
    ushort offset;
};

struct onchange_t
{
  virtual void changed(TView *obj,...) = 0;
  virtual ~onchange_t(void) {}
};

class TRect;
class TEvent;
class TGroup;

class TView : public TObject
#ifndef NO_TV_STREAMS
                                , public TStreamable
#endif  // !NO_TV_STREAMS
{

public:

    friend void genRefs();

    enum phaseType { phFocused, phPreProcess, phPostProcess };
    enum selectMode{ normalSelect, enterSelect, leaveSelect };

    TView( const TRect& bounds );
    ~TView();

    virtual void sizeLimits( TPoint& min, TPoint& max );
    TRect getBounds();
    TRect getExtent();
    TRect getClipRect();
    Boolean mouseInView( TPoint mouse );
    Boolean containsMouse( TEvent& event );

    void locate( TRect& bounds );
    virtual void dragView( TEvent& event, uchar mode,   //  temporary fix
      TRect& limits, TPoint minSize, TPoint maxSize ); //  for Miller's stuff
    virtual void calcBounds( TRect& bounds, TPoint delta );
    virtual void changeBounds( const TRect& bounds );
    void growTo( short x, short y );
    void moveTo( short x, short y );
    void setBounds( const TRect& bounds );

    virtual ushort getHelpCtx();

    virtual Boolean valid( ushort command );

    // set a callback object to be called when the view is modified by the user.
    // the object will be deleted by the view's destructor.
    // this function deletes any existing callback object.
    void set_onchange(onchange_t *cb);

    void hide();
    void show();
    virtual void draw();
    void drawView();
    Boolean TV_CDECL exposed();
    void hideCursor();
    void drawHide( TView *lastView );
    void drawShow( TView *lastView );
    void drawUnderRect( TRect& r, TView *lastView );
    void drawUnderView( Boolean doShadow, TView *lastView );

    virtual size_t dataSize();
    virtual void getData( void *rec, size_t recsize );
    virtual void setData( void *rec );

    void blockCursor();
    void normalCursor();
    virtual void TV_CDECL resetCursor();
    void setCursor( short x, short y );
    void showCursor();
    void drawCursor();

    void clearEvent( TEvent& event );
    Boolean eventAvail();
    virtual void getEvent( TEvent& event );
    virtual void handleEvent( TEvent& event );
    virtual void putEvent( TEvent& event );

    static Boolean commandEnabled( ushort command );
    static void disableCommands( TCommandSet& commands );
    static void enableCommands( TCommandSet& commands );
    static void disableCommand( ushort command );
    static void enableCommand( ushort command );
    static void getCommands( TCommandSet& commands );
    static void setCommands( TCommandSet& commands );

    virtual void endModal( ushort command );
    virtual ushort execute();

    ushort getColor( ushort color );
    virtual TPalette& getPalette() const;
    uchar TV_CDECL mapColor( uchar );

    Boolean getState( ushort aState );
    void select();
    virtual void setState( ushort aState, Boolean enable );

    void keyEvent( TEvent& event );
    Boolean mouseEvent( TEvent& event, ushort mask );


    TPoint makeGlobal( TPoint source );
    TPoint makeLocal( TPoint source );

    TView *nextView();
    TView *prevView();
    TView *prev();
    TView *next;

    void makeFirst();
    void putInFrontOf( TView *Target );
    TView *TopView();

    void writeBuf(  short x, short y, short w, short h, const TDrawBuffer& b );
    void writeLine( short x, short y, short w, short h, const TDrawBuffer& b );
    void TV_CDECL writeBuf(  short x, short y, short w, short h, const void *b );
    void TV_CDECL writeChar( short x, short y, char c, uchar color, short count );
    void TV_CDECL writeLine( short x, short y, short w, short h, const void *b );
    void TV_CDECL writeStr( short x, short y, const char *str, uchar color );

    TPoint size;
    ushort options;
    ushort eventMask;
    ushort state;
    TPoint origin;
    TPoint cursor;
    uchar growMode;
    uchar dragMode;
    ushort helpCtx;
    static Boolean commandSetChanged;
    static TCommandSet curCommandSet;
    TGroup *owner;
    onchange_t *onchange;

    static Boolean showMarkers;
    static uchar errorAttr;

    virtual void shutDown();

private:

    void moveGrow( TPoint p,
                   TPoint s,
                   TRect& limits,
                   TPoint minSize,
                   TPoint maxSize,
                   uchar mode
                 );
    void change( uchar mode, TPoint delta, TPoint& p, TPoint& s, uint32 ctrlState );
    void writeViewRec1( short x1, short x2, TView* p, int shadowCounter );
    void writeViewRec2( short x1, short x2, TView* p, int shadowCounter );
    void writeView( short x1, short x2, short y, const void* buf );

    int exposedRec1(short x1, short x2, TView* p );
    int exposedRec2( short x1, short x2, TView* p );

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TView( StreamableInit );

public:

    static const char * const name;
    static TStreamable *build();

protected:

    virtual void write( opstream& );
    virtual void *read( ipstream& );
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TView& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TView*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TView& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TView* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

inline void TView::writeBuf( short x, short y, short w, short h,
                             const TDrawBuffer& b )
{
    writeBuf( x, y, w, h, b.data );
}

inline void TView::writeLine( short x, short y, short w, short h,
                              const TDrawBuffer& b )
{
    writeLine( x, y, w, h, b.data );
}

#endif  // Uses_TView

/* ---------------------------------------------------------------------- */
/*      class TFrame                                                      */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Passive frame                                               */
/*        2 = Passive title                                               */
/*        3 = Active frame                                                */
/*        4 = Active title                                                */
/*        5 = Icons                                                       */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TFrame ) && !defined( __TFrame )
#define __TFrame

class TRect;
class TEvent;
class TDrawBuffer;

/**
 * TFrame provides the distinctive frames around windows and dialog boxes.
 * Users will probably never need to deal with frame objects directly, as they
 * are added to window objects by default.
 * @short The frame around the windows
 */
class TFrame : public TView
{

public:
    /**
     * Calls TView constructor TView(bounds), then sets @ref growMode to
     * @ref gfGrowHiX | @ref gfGrowHiY and sets @ref eventMask to
     * @reg eventMask | @ref evBroadcast, so TFrame objects default to
     * handling broadcast events.
     * `bounds' is the bounding rectangle of the frame.
     */
    TFrame( const TRect& bounds );
    /**
     * Draws the frame with color attributes and icons appropriate to the
     * current state flags: active, inactive, being dragged. Adds zoom, close
     * and resize icons depending on the owner window's flags. Adds the title,
     * if any, from the owning window's title data member.
     *
     * Active windows are drawn with a double-lined frame and any icons;
     * inactive windows are drawn with a single-lined frame and no icons.
     * @see TView::draw
     */
    virtual void draw();
    /**
     * Returns a reference to the default frame palette string.
     * @see TView::getPalette
     */
    virtual TPalette& getPalette() const;
    /**
     * Calls @ref TView::handleEvent(), then handles mouse events.
     *
     * If the mouse is clicked on the close icon, TFrame::handleEvent()
     * generates a cmClose event. Clicking on the zoom icon or double-clicking
     * on the top line of the frame generates a cmZoom event.
     *
     * Dragging the top line of the frame moves the window, and dragging the
     * resize icon moves the lower right corner of the view and therefore
     * changes its size.
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Changes the state of the frame.
     * Calls TView::setState(aState, enable). If the new state is
     * @ref sfActive or @ref sfDragging, calls @ref TView::drawView() to
     * redraw the view.
     * @see TView::setState
     * @see TView::state
     */
    virtual void setState( ushort aState, Boolean enable );
    /**
     * Undocumented.
     */
    static char frameChars[33];
    /**
     * The character showed in the close box.
     */
    static const char * closeIcon;
    /**
     * The character showed in the lower right corner of the screen.
     */
    static const char * dragIcon;
private:
    /**
     * Undocumented.
     */
    void TV_CDECL frameLine( TDrawBuffer& frameBuf, short y, short n, uchar color );
    /**
     * Undocumented.
     */
    void dragWindow( TEvent& event, uchar dragMode );
    /**
     * Undocumented.
     */
    friend class TDisplay;
    /**
     * Undocumented.
     */
    static const char initFrame[19];
    /**
     * Undocumented.
     */
    static const char * zoomIcon;
    /**
     * Undocumented.
     */
    static const char *unZoomIcon;
    /**
     * Undocumented.
     */
#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:
    /**
     * Used to recover the view from a stream.
     * Each streamable class needs a "builder" to allocate the correct memory
     * for its objects together with the initialized vtable pointers.
     * This is achieved by calling this constructor with an argument of type
     * @ref StreamableInit.
     */
    TFrame( StreamableInit );

public:
    /**
     * Undocumented.
     */
    static const char * const name;
    /**
     * Used to recover the view from a stream.
     * Called to create an object in certain stream-reading situations.
     */
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TFrame& cl )
    { return is >> (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline ipstream& operator >> ( ipstream& is, TFrame*& cl )
    { return is >> (void *&)cl; }

/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TFrame& cl )
    { return os << (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TFrame* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TFrame

/* ---------------------------------------------------------------------- */
/*      class TScrollBar                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Page areas                                                  */
/*        2 = Arrows                                                      */
/*        3 = Indicator                                                   */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TScrollBar ) && !defined( __TScrollBar )
#define __TScrollBar

class TRect;
class TEvent;

/**
 * An array representing the characters used to draw a TScrollBar.
 * @see TScrollBar
 * @see TScrollBar::draw
 */
typedef char TScrollChars[5];

/**
 * @short Implements a scroll bar
 */
class TScrollBar : public TView
{
public:
    /**
     * Creates and initializes a scroll bar with the given bounds by calling
     * the TView constructor. Sets @ref value, @ref maxVal and @ref minVal to
     * zero. Sets @ref pgStep and @ref arStep to 1.
     *
     * The shapes of the scroll bar parts are set to the defaults in
     * @ref chars data member.
     *
     * If `bounds' produces size.x = 1, scroll bar is vertical; otherwise, it
     * is horizontal. Vertical scroll bars have the @ref growMode data member
     * set to @ref gfGrowLoX | @ref gfGrowHiX | @ref gfGrowHiY; horizontal
     * scroll bars have the @ref growMode data member set to @ref gfGrowLoY |
     * @ref gfGrowHiX | @ref gfGrowHiY.
     */
    TScrollBar( const TRect& bounds );
    /**
     * Draws the scroll bar depending on the current bounds, value, and
     * palette.
     */
    virtual void draw();
    /**
     * Returns the default palette.
     */
    virtual TPalette& getPalette() const;
    /**
     * Handles scroll bar events by calling @ref TView::handleEvent(). Mouse
     * events are broadcast to the scroll bar's owner, which must handle the
     * implications of the scroll bar changes.
     *
     * handleEvent() also determines which scroll bar part has received a
     * mouse click (or equivalent keystroke). Data member @ref value is
     * adjusted according to the current @ref arStep or @ref pgStep values.
     * The scroll bar indicator is redrawn.
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Is called whenever @ref value data member changes. This virtual member
     * function defaults by sending a cmScrollBarChanged message to the scroll
     * bar's owner:
     *
     * <pre>
     * message(owner, @ref evBroadcast, cmScrollBarChanged, this);
     * </pre>
     * @see message
     */
    virtual void scrollDraw();
    /**
     * By default, scrollStep() returns a positive or negative step value,
     * depending on the scroll bar part given by `part', and the current
     * values of @ref arStep and @ref pgStep. Parameter `part' should be one
     * of the following constants:
     *
     * <pre>
     * Constant     Value Meaning
     *
* @ref sbLeftArrow  0     Left arrow of horizontal scroll bar
* @ref sbRightArrow 1     Right arrow of horizontal scroll bar
* @ref sbPageLeft   2     Left paging area of horizontal scroll bar
* @ref sbPageRight  3     Right paging area of horizontal scroll bar
* @ref sbUpArrow    4     Top arrow of vertical scroll bar
* @ref sbDownArrow  5     Bottom arrow of vertical scroll bar
* @ref sbPageUp     6     Upper paging area of vertical scroll bar
* @ref sbPageDown   7     Lower paging area of vertical scroll bar
* @ref sbIndicator  8     Position indicator on scroll bar
     * </pre>
     *
     * These constants define the different areas of a TScrollBar in which the
     * mouse can be clicked. The scrollStep() function converts these
     * constants into actual scroll step values.
     * Although defined, the sbIndicator constant is never passed to
     * scrollStep().
     */
    virtual int scrollStep( int part );
    /**
     * Sets the @ref value, @ref minVal, @ref maxVal, @ref pgStep and
     * @ref arStep with the given argument values. Some adjustments are made
     * if your arguments conflict.
     *
     * The scroll bar is redrawn by calling @ref drawView(). If value is
     * changed, @ref scrollDraw() is also called.
     */
    void setParams( int aValue, int aMin, int aMax,
                    int aPgStep, int aArStep );
    /**
     * Sets the legal range for value by setting @ref minVal and @ref maxVal
     * to the given arguments `aMin' and `aMax'.
     *
     * Calls @ref setParams(), so @ref drawView() and @ref scrollDraw() will
     * be called if the changes require the scroll bar to be redrawn.
     */
    void setRange( int aMin, int aMax );
    /**
     * Sets @ref pgStep and @ref arStep to the given arguments `aPgStep' and
     * `aArStep'.
     * Calls @ref setParams() with the other arguments set to their current
     * values.
     */
    void setStep( int aPgStep, int aArStep );
    /**
     * Sets @ref value to `aValue' by calling @ref setParams() with the other
     * arguments set to their current values.
     * Note: @ref drawView() and @ref scrollDraw() will be called if this
     * call changes value.
     */
    void setValue( int aValue );
    /**
     * Undocumented.
     */
    void drawPos( int pos );
    /**
     * Undocumented.
     */
    int getPos();
    /**
     * Undocumented.
     */
    int getSize();
    /**
     * This variable represents the current position of the scroll bar
     * indicator. This marker moves along the scroll bar strip to indicate the
     * relative position of the scrollable text being viewed relative to the
     * total text available for scrolling.
     *
     * The TScrollBar constructor sets value to zero by default.
     */
    int value;
    /**
     * TScrollChars is defined as:
     *
     * <pre>
     * typedef char TScrollChars[5];
     * </pre>
     *
     * Variable chars is set with the five basic character patterns used to
     * draw the scroll bar parts.
     */
    TScrollChars chars;
    /**
     * Variable minVal represents the minimum value for the @ref value data
     * member. The TScrollBar constructor sets minVal to zero by default.
     */
    int minVal;
    /**
     * Variable maxVal represents the maximum value for the @ref value data
     * member. The TScrollBar constructor sets maxVal to zero by default.
     */
    int maxVal;
    /**
     * Variable pgStep is the amount added or subtracted to the scroll bar's
     * @ref value data member when a mouse click event occurs in any of the
     * page areas (@ref sbPageLeft, @ref sbPageRight, @ref sbPageUp, or
     * @ref sbPageDown) or an equivalent keystroke is detected (Ctrl-Left,
     * Ctrl-Right, PgUp, or PgDn).
     *
     * The TScrollBar constructor sets pgStep to 1 by default. You can change
     * pgStep using @ref setParams(), @ref setStep() or
     * @ref TScroller::setLimit().
     */
    int pgStep;
    /**
     * Variable arStep is the amount added or subtracted to the scroll bar's
     * @ref value data member when an arrow area is clicked (@ref sbLeftArrow,
     * @ref sbRightArrow, @ref sbUpArrow, or @ref sbDownArrow) or the
     * equivalent keystroke made.
     *
     * The TScrollBar constructor sets arStep to 1 by default.
     */
    int arStep;
    /**
     * Undocumented.
     */
    static TScrollChars vChars;
    /**
     * Undocumented.
     */
    static TScrollChars hChars;
private:
    /**
     * Undocumented.
     */
    int getPartCode(void);
#ifndef NO_TV_STREAMS
    /**
     * Undocumented.
     */
    virtual const char *streamableName() const
        { return name; }

protected:
    /**
     * Each streamable class needs a "builder" to allocate the correct memory
     * for its objects together with the initialized virtual table pointers.
     * This is achieved by calling this constructor with an argument of type
     * @ref StreamableInit.
     */
    TScrollBar( StreamableInit );
    /**
     * Writes to the output stream `os'.
     */
    virtual void write( opstream& os );
    /**
     * Reads from the input stream `is'.
     */
    virtual void *read( ipstream& is );
public:
    /**
     * Undocumented.
     */
    static const char * const name;
    /**
     * Called to create an object in certain stream-reading situations.
     */
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TScrollBar& cl )
    { return is >> (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline ipstream& operator >> ( ipstream& is, TScrollBar*& cl )
    { return is >> (void *&)cl; }

/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TScrollBar& cl )
    { return os << (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TScrollBar* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TScrollBar

/* ---------------------------------------------------------------------- */
/*      class TScroller                                                   */
/*                                                                        */
/*      Palette layout                                                    */
/*      1 = Normal text                                                   */
/*      2 = Selected text                                                 */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TScroller ) && !defined( __TScroller )
#define __TScroller

class TRect;
class TScrollBar;
class TEvent;

/**
 * TScroller provides a scrolling virtual window onto a larger view. That is,
 * a scrolling view lets the user scroll a large view within a clipped
 * boundary.
 *
 * The scroller provides an offset from which the @ref TView::draw() method
 * fills the visible region. All methods needed to provide both scroll bar
 * and keyboard scrolling are built into TScroller.
 *
 * The basic scrolling view provides a useful starting point for scrolling
 * views such as text views.
 * @short Provides a scrolling virtual window onto a larger view
 */
class TScroller : public TView
{
public:
    /**
     * Creates and initializes a TScroller object with the given size and
     * scroll bars. Calls @ref TView constructor to set the view's size.
     *
     * `aHScrollBar' should be 0 if you do not want a horizontal scroll bar;
     * `aVScrollBar' should be 0 if you do not want a vertical scroll bar.
     */
    TScroller( const TRect& bounds,
               TScrollBar *aHScrollBar,
               TScrollBar *aVScrollBar
             );
    /**
     * Changes the scroller's size by calling @ref TView::setbounds(). If
     * necessary, the scroller and scroll bars are then redrawn by calling
     * @ref setLimit() and @ref drawView().
     */
    virtual void changeBounds( const TRect& bounds );
    /**
     * Returns the default scroller palette string.
     */
    virtual TPalette& getPalette() const;
    /**
     * Handles most events by calling @ref TView::handleEvent().
     *
     * Broadcast events such as cmScrollBarChanged from either @ref hScrollBar
     * or @ref vScrollBar result in a call to @ref scrollDraw().
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Checks to see if @ref delta matches the current positions of the scroll
     * bars. If not, @ref delta is set to the correct value and
     * @ref drawView() is called to redraw the scroller.
     */
    virtual void scrollDraw();
    /**
     * Sets the scroll bars to (x,y) by calling hScrollBar->setValue(x) and
     * vScrollBar->setValue(y) and redraws the view by calling @ref drawView().
     * @see TScrollBar::hScrollBar
     * @see TScrollBar::vScrollBar
     * @see TScrollBar::setValue
     */
    void scrollTo( int x, int y );
    /**
     * Sets the @ref limit data member and redraws the scrollbars and
     * scroller if necessary.
     */
    void setLimit( int x, int y );
    /**
     * This member function is called whenever the scroller's state changes.
     * Calls @ref TView::setState() to set or clear the state flags in
     * `aState'.
     * If the new @ref state is @ref sfSelected and @ref sfActive, setState()
     * displays the scroll bars; otherwise, they are hidden.
     */
    virtual void setState( ushort aState, Boolean enable );
    /**
     * If @ref drawLock is zero and @ref drawFlag is True, @ref drawFlag is set
     * False and @ref drawView() is called.
     * If @ref drawLock is non-zero or @ref drawFlag is False, checkDraw()
     * does nothing.
     *
     * Methods @ref scrollTo() and @ref setLimit() each call checkDraw() so
     * that @ref drawView() is only called if needed.
     */
    void checkDraw();
    /**
     * Used internally by @ref TObject::destroy() to ensure correct
     * destruction of derived and related objects. shutDown() is overridden
     * in many classes to ensure the proper setting of related data members
     * when @ref destroy() is called.
     */
    virtual void shutDown();
    /**
     * Holds the x (horizontal) and y (vertical) components of the scroller's
     * position relative to the virtual view being scrolled.
     *
     * Automatic scrolling is achieved by changing either or both of these
     * components in response to scroll bar events that change the value data
     * member(s).
     *
     * Manual scrolling changes delta, triggers changes in the scroll bar
     * @ref TScrollBar::value data members, and leads to updating of the
     * scroll bar indicators.
     */
    TPoint delta;
protected:
    /**
     * A semaphore used to control the redrawing of scrollers.
     */
    uchar drawLock;
    /**
     * Set True if the scroller has to be redrawn.
     */
    Boolean drawFlag;
    /**
     * Points to the horizontal scroll bar object associated with the
     * scroller. If there is no such scroll bar, hScrollBar is 0.
     */
    TScrollBar *hScrollBar;
    /**
     * Points to the vertical scroll bar object associated with the
     * scroller. If there is no such scroll bar, vScrollBar is 0.
     */
    TScrollBar *vScrollBar;
    /**
     * Data members limit.x and limit.y are the maximum allowed values for
     * delta.x and delta.y data members.
     * @see TScroller::delta
     */
    TPoint limit;

private:
    /**
     * Undocumented.
     */
    void showSBar( TScrollBar *sBar );

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:
    /**
     * Each streamable class needs a "builder" to allocate the correct memory
     * for its objects together with the initialized virtual table pointers.
     * This is achieved by calling this constructor with an argument of type
     * @ref StreamableInit.
     */
    TScroller( StreamableInit );
    /**
     * Writes to the output stream `os'.
     */
    virtual void write( opstream& os );
    /**
     * Reads from the input stream `is'.
     */
    virtual void *read( ipstream& is );
public:
    /**
     * Undocumented.
     */
    static const char * const name;
    /**
     * Called to create an object in certain stream-reading situations.
     */
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TScroller& cl )
    { return is >> (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline ipstream& operator >> ( ipstream& is, TScroller*& cl )
    { return is >> (void *&)cl; }

/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TScroller& cl )
    { return os << (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TScroller* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TScroller

#if defined( Uses_TListViewer ) && !defined( __TListViewer )
#define __TListViewer

class TRect;
class TScrollBar;
class TEvent;

/**
 * TListViewer is an abstract class from which you can derive list viewers of
 * various kinds, such as @ref TListBox. TListViewer's members offer the
 * following functionality:
 *
 * -# A view for displaying linked lists of items (but no list)
 * -# Control over one or two scroll bars
 * -# Basic scrolling of lists in two dimensions
 * -# Reading and writing the view and its scroll bars from and to a stream
 * -# Ability to use a mouse or the keyboard to select (highlight) items on
 *    list
 * -# Draw member function that copes with resizing and scrolling
 *
 * TListViewer has an abstract @ref getText() method, so you need to supply
 * the mechanism for creating and manipulating the text of the items to be
 * displayed.
 *
 * TListViewer has no list storage mechanism of its own. Use it to display
 * scrollable lists of arrays, linked lists, or similar data structures. You
 * can also use its descendants, such as @ref TListBox, which associates a
 * collection with a list viewer.
 * @short An abstract class from which you can derive list viewers of various
 * kinds, such as TListBox.
 */
class TListViewer : public TView
{

public:
    /**
     * Creates and initializes a TListViewer object with the given size by
     * first calling @ref TView::TView(bounds).
     *
     * The @ref numCols data member is set to `aNumCols'. @ref TView::options
     * is set to (@ref ofFirstClick | @ref ofSelectable) so that mouse clicks
     * that select this view will be passed first to @ref handleEvent().
     *
     * The @ref TView::eventMask is set to @ref evBroadcast. The initial
     * values of @ref range and @ref focused are zero.
     *
     * You can supply pointers to vertical and/or horizontal scroll bars by
     * way of the `aVScrollBar' and `aHScrollBar' arguments. Setting either or
     * both to 0 suppresses one or both scroll bars. These two pointer
     * arguments are assigned to the @ref vScrollBar and @ref hScrollBar data
     * members.
     */
    TListViewer( const TRect& bounds,
                 ushort aNumCols,
                 TScrollBar *aHScrollBar,
                 TScrollBar *aVScrollBar
               );
    /**
     * Changes the size of the TListViewer object by calling
     * TView::changeBounds(bounds). If a horizontal scroll bar has been
     * assigned, @ref TScrollBar::pgStep is updated by way of
     * @ref TScrollBar::setStep().
     * @see TView::changeBounds
     */
    virtual void changeBounds( const TRect& bounds );
    /**
     * Draws the TListViewer object with the default palette by repeatedly
     * calling @ref getText() for each visible item. Takes into account the
     * @ref focused and selected items and whether the view is @ref sfActive.
     * @see TView::state
     */
    virtual void draw();
    /**
     * Makes the given item focused by setting the @ref focused data member to
     * `item'. Also sets the @ref TScrollBar::value data member of the
     * vertical scroll bar (if any) to `item' and adjusts @ref topItem.
     */
    virtual void focusItem( int item );
    virtual TPalette& getPalette() const;
    /**
     * Derived classes must override it with a function that writes a string
     * at address `dest', given an item index referenced by `item'.
     *
     * Note that @ref draw() needs to call getText().
     */
    virtual void getText( char *dest, int item, size_t destsize );
    /**
     * Returns True if the given item is selected (focused), that is, if
     * `item' == @ref focused.
     */
    virtual Boolean isSelected( int item );
    /**
     * Handles events by first calling TView::handleEvent(event).
     * @see TView::handleEvent
     *
     * Mouse clicks and "auto" movements over the list will change the focused
     * item. Items can be selected with double mouse clicks.
     *
     * Keyboard events are handled as follows: Spacebar selects the currently
     * focused item; the arrow keys, PgUp, PgDn, Ctrl-PgDn, Ctrl-PgUp, Home,
     * and End keys are tracked to set the focused item.
     *
     * Broadcast events from the scroll bars are handled by changing the
     * @ref focused item and redrawing the view as required.
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Selects the item'th element of the list, then broadcasts this fact to
     * the owning group by calling:
     *
     * <pre>
     * message(owner, @ref evBroadcast, cmListItemSelected, this);
     * </pre>
     * @see message
     */
    virtual void selectItem( int item );
    /**
     * Sets the @ref range data member to `aRange'.
     *
     * If a vertical scroll bar has been assigned, its parameters are adjusted
     * as necessary (and @ref TScrollBar::drawView() is invoked if redrawing is
     * needed).
     *
     * If the currently focused item falls outside the new range, the
     * @ref focused data member is set to zero.
     */
    void setRange( int aRange );
    /**
     * Calls TView::setState(aState, enable) to change the TListViewer
     * object's state. Depending on the `aState' argument, this can result in
     * displaying or hiding the view.
     * @see TView::setState
     * @see TView::state
     *
     * Additionally, if `aState' is @ref sfSelected and @ref sfActive, the
     * scroll bars are redrawn; if `aState' is @ref sfSelected but not
     * @ref sfActive, the scroll bars are hidden.
     */
    virtual void setState( ushort aState, Boolean enable );
    /**
     * Used internally by @ref focusItem(). Makes the given item focused by
     * setting the @ref focused data member to `item'.
     */
    virtual void focusItemNum( int item );
    /**
     * Used internally by @ref TObject::destroy() to ensure correct
     * destruction of derived and related objects. shutDown() is overridden
     * in many classes to ensure the proper setting of related data members
     * when @ref destroy() is called.
     */
    virtual void shutDown();
    /**
     * Pointer to the horizontal scroll bar associated with this view. If 0,
     * the view does not have such a scroll bar.
     */
    TScrollBar *hScrollBar;
    /**
     * Pointer to the vertical scroll bar associated with this view. If 0,
     * the view does not have such a scroll bar.
     */
    TScrollBar *vScrollBar;
    /**
     * The number of columns in the list control.
     */
    short numCols;
    /**
     * The item number of the top item to be displayed. This value changes
     * during scrolling. Items are numbered from 0 to @ref range - 1. This
     * number depends on the number of columns, the size of the view, and the
     * value of variable @ref range.
     */
    int topItem;
    int focused;
    int range;

private:

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TListViewer( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

protected:
    // next two variables are initialized in the constructor as -1
    int first_selected; // if < 0 - no selected item(s)
    int last_selected;  // if < 0 - multiselection disabled
    // return if the item has been selected
    virtual bool isItemSelected( int item );
private:
    inline void remove_selection(void)
    {
        if ( first_selected >= 0 )
            {
            first_selected = -1;
            drawView();
            }
    }

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TListViewer& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TListViewer*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TListViewer& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TListViewer* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TListViewer

#if defined( Uses_TGroup ) && !defined( __TGroup )
#define __TGroup

class TView;

class TGroup : public TView
{

public:

    friend void genRefs();

    TGroup( const TRect& bounds );
    ~TGroup();

    virtual void shutDown();

    ushort execView( TView *p );
    virtual ushort execute();

    void insertView( TView *p, TView *Target );
    void TV_CDECL remove( TView *p );
    void removeView( TView *p );
    void TV_CDECL resetCurrent();
    void setCurrent( TView *p, selectMode mode );
    void selectNext( Boolean forwards );
    TView *firstThat( Boolean (*func)( TView *, void * ), void *args );
    void forEach( void (*func)( TView *, void * ), void *args );
    void insert( TView *p );
    void insertBefore( TView *p, TView *Target );
    TView *current;
    TView *at( short index );
    TView *firstMatch( ushort aState, ushort aOptions );
    short indexOf( TView *p );
    Boolean matches( TView *p );
    TView *first();

    virtual void setState( ushort aState, Boolean enable );

    virtual void handleEvent( TEvent& event );

    void drawSubViews( TView *p, TView *bottom );

    virtual void changeBounds( const TRect& bounds );

    virtual size_t dataSize();
    virtual void getData( void *rec, size_t recsize );
    virtual void setData( void *rec );

    virtual void draw();
    void redraw();
    void lock();
    void unlock();
    virtual void TV_CDECL resetCursor();

    virtual void endModal( ushort command );

    virtual void eventError( TEvent& event );

    virtual ushort getHelpCtx();

    virtual Boolean valid( ushort command );

    void freeBuffer();
    void getBuffer();

    TView *last;

    TRect clip;
    phaseType phase;

    ushort *buffer;
    uchar lockFlag;
    ushort endState;

private:

    Boolean invalid( TView *p, ushort command );
    void focusView( TView *p, Boolean enable );
    void selectView( TView *p, Boolean enable );

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TGroup( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TGroup& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TGroup*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TGroup& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TGroup* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TGroup

#if defined( Uses_TWindow ) && !defined( __TWindow )
#define __TWindow

class TFrame;
class TRect;
class TPoint;
class TEvent;
class TFrame;
class TScrollBar;

class TWindowInit
{

public:

    TWindowInit( TFrame *(*cFrame)( TRect ) );

protected:

    TFrame *(*createFrame)( TRect );

};

/* ---------------------------------------------------------------------- */
/*      class TWindow                                                     */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Frame passive                                               */
/*        2 = Frame active                                                */
/*        3 = Frame icon                                                  */
/*        4 = ScrollBar page area                                         */
/*        5 = ScrollBar controls                                          */
/*        6 = Scroller normal text                                        */
/*        7 = Scroller selected text                                      */
/*        8 = Reserved                                                    */
/* ---------------------------------------------------------------------- */

class TWindow: public TGroup, public virtual TWindowInit
{

public:

    TWindow( const TRect& bounds,
             const char *aTitle,
             short aNumber
           );
    ~TWindow();

    virtual void close();
    virtual TPalette& getPalette() const;
    virtual const char *getTitle( short maxSize );
    virtual void handleEvent( TEvent& event );
    static TFrame *initFrame( TRect );
    virtual void setState( ushort aState, Boolean enable );
    virtual void sizeLimits( TPoint& min, TPoint& max );
    TScrollBar *standardScrollBar( ushort aOptions );
    virtual void zoom();
    virtual void shutDown();

    uchar flags;
    TRect zoomRect;
    short number;
    short palette;
    TFrame *frame;
    const char *title;

private:

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TWindow( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TWindow& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TWindow*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TWindow& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TWindow* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TWindow

