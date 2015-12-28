/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   DIALOGS.H                                                             */
/*                                                                         */
/*   Copyright (c) Borland International 1991                              */
/*   All Rights Reserved.                                                  */
/*                                                                         */
/*   defines the classes TDialog, TInputLine, TButton, TCluster,           */
/*   TRadioButtons, TCheckBoxes, TStaticText, TParamText, TLabel,          */
/*   THistoryViewer, and THistoryWindow.                                   */
/*                                                                         */
/* ------------------------------------------------------------------------*/

#include <cm_codes.h>

#if !defined( __BUTTON_TYPE )
#define __BUTTON_TYPE

const int
    bfNormal    = 0x00,
    bfDefault   = 0x01,
    bfLeftJust  = 0x02,
    bfBroadcast = 0x04,
    bfNoShadows = 0x08;

#endif  // __BUTTON_TYPE

/* ---------------------------------------------------------------------- */
/*      class TDialog                                                     */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Frame passive                                               */
/*        2 = Frame active                                                */
/*        3 = Frame icon                                                  */
/*        4 = ScrollBar page area                                         */
/*        5 = ScrollBar controls                                          */
/*        6 = StaticText                                                  */
/*        7 = Label normal                                                */
/*        8 = Label selected                                              */
/*        9 = Label shortcut                                              */
/*       10 = Button normal                                               */
/*       11 = Button default                                              */
/*       12 = Button selected                                             */
/*       13 = Button disabled                                             */
/*       14 = Button shortcut                                             */
/*       15 = Button shadow                                               */
/*       16 = Cluster normal                                              */
/*       17 = Cluster selected                                            */
/*       18 = Cluster shortcut                                            */
/*       19 = InputLine normal text                                       */
/*       20 = InputLine selected text                                     */
/*       21 = InputLine arrows                                            */
/*       22 = History arrow                                               */
/*       23 = History sides                                               */
/*       24 = HistoryWindow scrollbar page area                           */
/*       25 = HistoryWindow scrollbar controls                            */
/*       26 = ListViewer normal                                           */
/*       27 = ListViewer focused                                          */
/*       28 = ListViewer selected                                         */
/*       29 = ListViewer divider                                          */
/*       30 = InfoPane                                                    */
/*       31 = Reserved                                                    */
/*       32 = Reserved                                                    */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TDialog ) && !defined( __TDialog )
#define __TDialog

class TRect;
class TEvent;

class TDialog : public TWindow
{

public:

    TDialog( const TRect& bounds, const char *aTitle );

    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual Boolean valid( ushort command );

private:

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TDialog( StreamableInit );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TDialog& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDialog*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDialog& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDialog* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TDialog

/* ---------------------------------------------------------------------- */
/*      class TInputLine                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Passive                                                     */
/*        2 = Active                                                      */
/*        3 = Selected                                                    */
/*        4 = Arrows                                                      */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TInputLine ) && !defined( __TInputLine )
#define __TInputLine

class TRect;
class TEvent;

/**
 * A TInputLine object provides a basic input line string editor. It handles
 * keyboard input and mouse clicks and drags for block marking and a variety
 * of line editing functions.
 * @short Provides a basic input line string editor
 */
class TInputLine : public TView
{
public:
    static bool disableReselect; // for 'ovelapped' wait_boxes
    /**
     * Creates an input box control with the given values by calling
     * TView::TView(bounds).
     * @see TView::TView
     *
     * Variable @ref state is then set to @ref sfCursorVis, @ref options is
     * set to (@ref ofSelectable | @ref ofFirstClick), and @ref maxLen is set
     * to `aMaxLen'.
     *
     * Memory is allocated and cleared for `aMaxlen' + 1 bytes and the
     * @ref data data member set to point at this allocation.
     *
     * An input line is sometimes used with a validator. Data validators are
     * objects that detect when the user has entered valid or invalid
     * information. In case of invalid data, the validator will provide
     * appropriate feedback to the user.
     * @see TValidator
     */
    TInputLine( const TRect& bounds, int aMaxLen );
    /**
     * Deletes the data memory allocation, then calls TView destructor to
     * destroy the TInputLine object.
     * @see TView::~TView
     */
    ~TInputLine();
    /**
     * Returns the size of the record for @ref getData() and @ref setData()
     * calls.
     * By default, it returns @ref maxLen + 1.
     *
     * Override this member function if you define descendants to handle other
     * data types.
     */
    virtual size_t dataSize();
    /**
     * Draws the input box and its data.
     *
     * The box is drawn with the appropriate colors depending on whether the
     * box is @ref sfFocused (that is, whether the box view owns the cursor),
     * and arrows are drawn if the input string exceeds the size of the view
     * (in either or both directions).
     * @see TView::state
     *
     * Any selected (block-marked) characters are drawn with the appropriate
     * palette.
     */
    virtual void draw();
    /**
     * Writes the number of bytes (obtained from a call to @ref dataSize())
     * from the data string to the array given by `rec'. Used with
     * @ref setData() for a variety of applications; for example, temporary
     * storage, or passing on the input string to other views.
     *
     * Override getData() if you define TInputLine descendants to
     * handle non-string data types. You can also use getData() to
     * convert from a string to other data types after editing by TInputLine.
     */
    virtual void getData( void *rec, size_t recsize );
    /**
     * Returns the default palette string.
     */
    virtual TPalette& getPalette() const;
    /**
     * Calls @ref TView::handleEvent(), then handles all mouse and keyboard
     * events if the input box is selected.
     *
     * This member function implements the standard editing capability of the
     * input box. Editing features include:
     *
     * -# block marking with mouse click and drag
     * -# block deletion
     * -# insert or overwrite control with automatic cursor shape change
     * -# automatic and manual scrolling as required (depending on relative
     *    sizes of the @ref data string and size.x); see @ref size
     * -# manual horizontal scrolling via mouse clicks on the arrow icons
     * -# manual cursor movement by arrow, Home, and End keys (and their
     *    standard control-key equivalents)
     * -# character and block deletion with Del and Ctrl-G
     *
     * The view is redrawn as required and the TInputLine data members are
     * adjusted appropriately.
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Sets @ref curPos, @ref firstPos and @ref selStart data members to 0.
     *
     * If `enable' is set to True, @ref selEnd is set to the length of the
     * @ref data string, thereby selecting the whole input line; if `enable'
     * is set to False, @ref selEnd is set to 0, thereby deselecting the
     * whole line.
     *
     * Finally, the view is redrawn by calling @ref TView::drawView().
     */
    void selectAll( Boolean enable );
    /**
     * By default, copies the number of bytes (as returned by
     * @ref dataSize()) from the `rec' array to the @ref data string, and
     * then calls selectAll(True). This zeros @ref curPos, @ref firstPos and
     * @ref selStart.
     * @see TInputLine::selectAll
     *
     * Finally, @ref TView::drawView() is called to redraw the input box.
     *
     * Override setData() if you define descendants to handle non-string
     * data types. You also use setData() to convert other data types to
     * a string for editing by TInputLine.
     */
    virtual void setData( void *rec );
    /**
     * Called when the input box needs redrawing (for example, if the palette
     * is changed) following a change of state.
     *
     * Calls @ref TView::setState() to set or clear the view's @ref state with
     * the given `aState' bit(s).
     *
     * Then if `aState' is @ref sfSelected (or @ref sfActive and the input
     * box is @ref sfSelected), selectAll(enable) is called (which, in turn,
     * calls @ref TView::drawView()).
     * @see TInputLine::selectAll
     */
    virtual void setState( ushort aState, Boolean enable );

    /**
     * The string containing the edited information.
     */
    char* data;
    /**
     * Maximum length allowed for string to grow (excluding the final 0).
     */
    int maxLen;
    /**
     * Index to insertion point (that is, to the current cursor position).
     */
    int curPos;
    /**
     * Index to the first displayed character.
     */
    int firstPos;
    /**
     * Index to the beginning of the selection area (that is, to the first
     * character block marked).
     */
    int selStart;
    /**
     * Index to the end of the selection area (that is, to the last character
     * block marked).
     */
    int selEnd;

private:
    Boolean canScroll( int delta );
    int mouseDelta( TEvent& event );
    int mousePos( TEvent& event );
    void deleteSelect();
    void adjustSelectBlock();
    char *clip_get(size_t &clipsz);
    bool clip_put(void);

    static const char rightArrow;
    static const char leftArrow;
    int anchor;
#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:
    /**
     * Each streamable class needs a "builder" to allocate the correct memory
     * for its objects together with the initialized virtual table pointers.
     *
     * This is achieved by calling this constructor with an argument of type
     * @ref StreamableInit.
     */
    TInputLine( StreamableInit );
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
inline ipstream& operator >> ( ipstream& is, TInputLine& cl )
    { return is >> (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline ipstream& operator >> ( ipstream& is, TInputLine*& cl )
    { return is >> (void *&)cl; }

/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TInputLine& cl )
    { return os << (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TInputLine* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TInputLine


/* ---------------------------------------------------------------------- */
/*      TButton object                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Default text                                                */
/*        3 = Selected text                                               */
/*        4 = Disabled text                                               */
/*        5 = Normal shortcut                                             */
/*        6 = Default shortcut                                            */
/*        7 = Selected shortcut                                           */
/*        8 = Shadow                                                      */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TButton ) && !defined( __TButton )
#define __TButton

class TRect;
class TEvent;
class TDrawBuffer;

/**
 * One of the most used and easy to use views. A TButton object is a box with
 * a title and a shadow that generates a command when pressed. A button can
 * be selected by:
 *
 * -# typing the highlighted letter
 * -# tabbing to the button and pressing Spacebar
 * -# pressing Enter when the button is the default
 * -# clicking on the button with a mouse
 *
 * With color and black-and-white palettes, a button has a three-dimensional
 * look that moves when selected. On monochrome systems, a button is bordered
 * by brackets, and other ASCII characters are used to indicate whether the
 * button is default, selected, and so on.
 *
 * There can only be one default button in a window or dialog at any given
 * time. Buttons that are peers in a group grab and release the default state
 * via @ref evBroadcast messages.
 * @short The button view
 */
class TButton : public TView
{
public:
    /**
     * Constructor.
     *
     * Creates a TButton class with the given size by calling the TView
     * constructor.
     *
     * -# `bounds' is the bounding rectangle of the button
     * -# `aTitle' is a pointer to a string which will be the title of the
     *    button
     * -# `aCommand' is the command generated when the user presses the button.
     *    If the given `aCommand' is not enabled, @ref sfDisabled is set in the
     *    @ref state data member.
     * -# `aFlags' is a combination of the following values:
     *
     * <pre>
     * Constant    Value Meaning
     *
* @ref bfNormal    0x00  Button is a normal, non-default button
     *
* @ref bfDefault   0x01  Button is the default button: if this bit is set this
     *                   button will be highlighted as the default button
     *
* @ref bfLeftJust  0x02  Button label is left-justified; if this bit is clear
     *                   the title will be centered
     *
* @ref bfBroadcast 0x04  Sends a broadcast message when pressed
     *
* @ref bfNoShadows 0x08  The button has no shadows
     * </pre>
     *
     * It is the responsibility of the programmer to ensure that there is only
     * one default button in a TGroup. However the default property can be
     * passed to normal buttons by calling @ref makeDefault().
     * @see TButton::amDefault
     *
     * The @ref bfLeftJust value can be added to @ref bfNormal or
     * @ref bfDefault and affects the position of the text displayed within
     * the button: if clear, the label is centered; if set, the label is
     * left-justified.
     *
     * The @ref options data member is set to (@ref ofSelectable |
     * @ref ofFirstClick | @ref ofPreProcess | @ref ofPostProcess) so that
     * by default TButton responds to these events.
     *
     * @ref eventMask is set to @ref evBroadcast.
     */
    TButton( const TRect& bounds,
             const char *aTitle,
             ushort aCommand,
             ushort aFlags
           );
    /**
     * Destructor.
     *
     * Frees the memory assigned to the button's title, then destroys the view
     * with TView::~TView.
     * @see TView::~TView
     */
    ~TButton();
    /**
     * Draws the button by calling TButton::drawState(False).
     * @see TButton::drawState
     */
    virtual void draw();
    /**
     * Called by @ref draw().
     *
     * Draws the button in the "down" state (no shadow) if down is True;
     * otherwise, it draws the button in the "up" state if down is False.
     *
     * The appropriate palettes are used to reflect the current state (normal,
     * default, disabled). The button label is positioned according to the
     * @ref bfLeftJust bit in the @ref flags data member.
     */
    void drawState( Boolean down );
    /**
     * Returns a reference to the standard TButton palette string.
     */
    virtual TPalette& getPalette() const;
    /**
     * Handles TButton events.
     *
     * Responds to being pressed in any of three ways: mouse clicks on the
     * button, its hot key being pressed, or being the default button when a
     * cmDefault broadcast arrives.
     *
     * When the button is pressed, a command event is generated with
     * @ref putEvent(), with the @ref command data member assigned to
     * command and infoPtr set to this.
     *
     * Buttons also recognize the broadcast commands cmGrabDefault and
     * cmReleaseDefault, to become or "unbecome" the default button, as
     * appropriate, and cmCommandSetChanged, which causes them to check
     * whether their commands have been enabled or disabled.
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Changes the default property of this button. Used to make this button
     * the default with `enable' set to True, or to release the default with
     * `enable' set to False. Three notes:
     *
     * -# If `enable' is True, the button grabs the default property from
     *    the default button (if exists) with a cmGrabDefault broadcast
     *    command, so the default button losts the default property.
     * -# If `enable' is False, the button releases the default property to
     *    the default button (if exists) with a cmReleaseDefault broadcast
     *    command, so the default button gains the default property.
     *    These changes are usually the result of tabbing within a dialog box.
     *    The status is changed without actually operating the button. The
     *    default button can be subsequently "pressed" by using the Enter key.
     *    This mechanism allows a normal button (without the @ref bfDefault
     *    bit set) to behave like a default button. The button is redrawn if
     *    necessary to show the new status.
     * -# This method does nothing if the button is a default button (i.e. it
     *    has the @ref bfDefault bit set).
     *
     * @see TButton::flags
     */
    void makeDefault( Boolean enable );
    /**
     * This method is called whenever the button is pressed.
     *
     * Its task is to send a message. The message is a broadcast message to
     * the owner of the view if the button has the @ref bfBroadcast bit set,
     * otherwise the message is a command message.
     * @see TButton::flags
     *
     * Used internally by @ref handleEvent() when a mouse click "press" is
     * detected or when the default button is "pressed" with the Enter key.
     */
    virtual void press();
    /**
     * Changes the state of the button.
     *
     * Calls @ref setState(), then calls @ref drawView() to redraw the button
     * if it has been made @ref sfSelected or @ref sfActive.
     * @see TView::state
     *
     * If focus is received (that is, if `aState' is @ref sfFocused), the
     * button grabs or releases default from the default button by calling
     * @ref makeDefault().
     */
    virtual void setState( ushort aState, Boolean enable );
    /**
     * This is a pointer to the label text of the button.
     */
    const char *title;
    /**
     * A pointer to the shadow characters.
     *
     * These characters are used to draw the button shadow.
     */
    static const char * shadows;
protected:
    /**
     * This is the command word of the event generated when this button is
     * pressed.
     */
    ushort command;
    /**
     * This variabile is a bitmapped data member used to indicate whether
     * button text is left-justified or centered.
     *
     * The individual flags are the various bfXXXX constants.
     * @see TButton::TButton
     */
    uchar flags;
    /**
     * If True the button has the default property.
     *
     * The default button is automatically selected when the user presses the
     * Enter key. If this variable is False, the button is a normal button.
     */
    Boolean amDefault;
private:
    void drawTitle( TDrawBuffer&, int, int, ushort, Boolean );
    void pressButton( TEvent& );
    TRect getActiveRect();
    static const char * markers;

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:
    /**
     * Constructor.
     *
     * Used to recover the view from a stream.
     *
     * Each streamable class needs a "builder" to allocate the correct memory
     * for its objects together with the initialized virtual table pointers.
     * This is achieved by calling this constructor with an argument of type
     * @ref StreamableInit.
     */
    TButton( StreamableInit ): TView( streamableInit ) {};
    /**
     * Used to store the view in a stream.
     *
     * Writes to the output stream `os'.
     */
    virtual void write( opstream& os );
    /**
     * Used to recover the view from a stream.
     *
     * Reads from the input stream `is'.
     */
    virtual void *read( ipstream& is );
public:
    /**
     * Undocumented.
     */
    static const char * const name;
    /**
     * Creates a new TButton.
     *
     * Used to recover the view from a stream. Called to create an object in
     * certain stream-reading situations.
     */
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TButton& cl )
    { return is >> (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline ipstream& operator >> ( ipstream& is, TButton*& cl )
    { return is >> (void *&)cl; }

/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TButton& cl )
    { return os << (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TButton* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TButton


#if defined( Uses_TSItem ) && !defined( __TSItem )
#define __TSItem

/**
 * TSItem is a simple, non-view class providing a singly-linked list of
 * character strings.
 * This class is useful where the full flexibility of string collections are
 * not needed.
 * @short Non-view class providing a singly-linked list of character strings
 */
class TSItem
{
public:
    /**
     * Creates a TSItem object with the given values.
     */
    TSItem( const char *aValue, TSItem *aNext )
        { value = newStr(aValue); next = aNext; }
    /**
     * Destroys the TSItem object by calling delete value.
     */
    ~TSItem() { delete[] (char *)value; }
    /**
     * The string for this TSItem object.
     */
    const char *value;
    /**
     * Pointer to the next TSItem object in the linked list.
     */
    TSItem *next;
};

#endif  // Uses_TSItem

/* ---------------------------------------------------------------------- */
/*      class TCluster                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TCluster ) && !defined( __TCluster )
#define __TCluster

class TRect;
class TSItem;
class TEvent;
class TPoint;
class TStringCollection;

/**
 * The base class used both by @ref TCheckBoxes and @ref TRadioButtons.
 *
 * A cluster is a group of controls that all respond in the same way.
 * TCluster is an abstract class from which the useful group controls such as
 * @ref TRadioButtons, @ref TCheckBoxes, and @ref TMonoSelector are derived.
 *
 * Cluster controls are often associated with @ref TLabel objects, letting you
 * select the control by selecting on the adjacent explanatory label.
 * Clusters are used to toggle bit values in the @ref value data member, which
 * is of type uint32.
 *
 * The two standard descendants of TCluster use different algorithms when
 * changing value: @ref TCheckBoxes simply toggles a bit, while
 * @ref TRadioButtons toggles the enabled one and clears the previously
 * selected bit.
 * Both inherit most of their behavior from TCluster.
 * @short The base class of TCheckBoxes and TRadioButtons
 */
class TCluster : public TView
{
public:
    /**
     * Constructor.
     *
     * Calls TView::TView(bounds) to create a TCluster object with the given
     * `bounds', where `bounds' is the desired bounding rectangle of the view.
     * The @ref strings data member is set to `aStrings', a pointer to a
     * linked list of @ref TSItem objects, one for each cluster item.
     * Every @ref TSItem object stores the caption of the related item.
     * @see TView::TView
     *
     * TCluster handles a maximum of 32 items.
     * The constructor clears the @ref value and @ref sel data members.
     */
    TCluster( const TRect& bounds, TSItem *aStrings );
    /**
     * Deletes the cluster's string collection, then destroys the view with
     * TView::~TView().
     * @see TView::~TView
     */
    ~TCluster();
    /**
     * Returns the size of the data record of this view (composed by the
     * @ref value data member).
     * Must be overridden in derived classes that change value or add other
     * data members, in order to work with @ref getData() and @ref setData().
     *
     * It returns `sizeof(short)' for compatibility with earlier TV, even if
     * @ref value data member is now an uint32; @ref TMultiCheckBoxes
     * returns sizeof(int32).
     */
    virtual size_t dataSize();
    /**
     * Redraws the view.
     *
     * Called within the @ref draw() method of derived classes to draw the
     * box in front of the string for each item in the cluster.
     * @see TCheckBoxes::draw
     * @see TRadioButtons::draw
     * @see TView::draw
     *
     * Parameter `icon' is a five-character string that points to a string
     * which will be written at the left side of every item (" [ ] " for check
     * boxes, " () " for radio buttons).
     *
     * Parameter `marker' is the character to use to indicate the box has been
     * marked ("X" for check boxes, "." for radio buttons).
     * A space character will be used if the box is unmarked.
     * @see TCluster::drawMultiBox
     */
    void drawBox( const char *icon, char marker );
    virtual void getData( void *rec, size_t recsize );
    /**
     * Returns the help context of the selected item.
     *
     * The help context is calculated by summing view variable @ref helpCtx
     * and the number of the currently selected item (0 for the first item,
     * 1 for the second item, etc). Redefines @ref TView::getHelpCtx().
     *
     * Enables you to have separate help contexts for each item in the
     * cluster. Use it to reserve a range of help contexts equal to
     * @ref helpCtx plus the number of cluster items minus one.
     */
    ushort getHelpCtx();
    /**
     * Returns a reference to the standard TCluster palette.
     */
    virtual TPalette& getPalette() const;
    /**
     * Calls @ref TView::handleEvent(), then handles all mouse and keyboard
     * events appropriate to this cluster.
     *
     * Controls are selected by mouse click or cursor movement keys (including
     * Spacebar).
     * The cluster is redrawn to show the selected controls.
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Called by the @ref draw() method redefined both in @ref TCheckBoxes and
     * @ref TRadioButtons classes to determine which items are marked. mark()
     * should be overridden to return True if the item'th control in the
     * cluster is marked; otherwise, it should return False.
     * @see TCheckBoxes::draw
     * @see TRadioButton::draw
     *
     * The default mark() returns False. Redefined in @ref TCheckBoxes and
     * in @ref TRadioButtons.
     * @see TCheckBoxes::mark
     * @see TRadioButtons::mark
     */
    virtual Boolean mark( int item );
    virtual void setItem( int item, Boolean on );
    virtual void press( int item );
    virtual void movedTo( int item );
    virtual void setData( void *rec );
    virtual void setState( ushort aState, Boolean enable );

    bool isEnabledItem( int item ) { return (enabled & (1<<item)) != 0; }
    void setEnabledItem( int item, Boolean enable );


protected:

    ushort value;
    ushort enabled;     // bitmask of enabled bits
    int sel;
    TStringCollection *strings;

private:

    int column( int item );
    int findSel( TPoint p );
    int row( int item );
    const char *getItemText( ccIndex item );

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TCluster( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TCluster& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TCluster*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TCluster& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TCluster* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TCluster


/* ---------------------------------------------------------------------- */
/*      class TRadioButtons                                               */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/* ---------------------------------------------------------------------- */


#if defined( Uses_TRadioButtons ) && !defined( __TRadioButtons )
#define __TRadioButtons

class TRect;
class TSItem;

class TRadioButtons : public TCluster
{

public:

    TRadioButtons( const TRect& bounds, TSItem *aStrings );

    virtual void draw();
    virtual Boolean mark( int item );
    virtual void setItem( int item, Boolean on );
    virtual void movedTo( int item );
    virtual void press( int item );
    virtual void setData( void *rec );

private:

    static const char *button;
#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TRadioButtons( StreamableInit );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TRadioButtons& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TRadioButtons*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TRadioButtons& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TRadioButtons* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

inline TRadioButtons::TRadioButtons( const TRect& bounds, TSItem *aStrings ) :
    TCluster( bounds, aStrings )
{
}

#endif  // Uses_TRadioButtons


/* ---------------------------------------------------------------------- */
/*      TCheckBoxes                                                       */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TCheckBoxes ) && !defined( __TCheckBoxes )
#define __TCheckBoxes

class TRect;
class TSItem;

class TCheckBoxes : public TCluster
{

public:

    TCheckBoxes( const TRect& bounds, TSItem *aStrings);

    virtual void draw();

    virtual Boolean mark( int item );
    virtual void setItem( int item, Boolean on );
    virtual void press( int item );

private:

    static const char *button;

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TCheckBoxes( StreamableInit );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TCheckBoxes& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TCheckBoxes*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TCheckBoxes& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TCheckBoxes* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

inline TCheckBoxes::TCheckBoxes( const TRect& bounds, TSItem *aStrings) :
    TCluster( bounds, aStrings )
{
}

#endif  // Uses_TCheckBoxes


#if defined( Uses_TListBox ) && !defined( __TListBox )
#define __TListBox

class TRect;
class TScrollBar;
class TCollection;

class TListBox : public TListViewer
{

public:

    TListBox( const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar );
    ~TListBox();

    virtual size_t dataSize();
    virtual void getData( void *rec, size_t recsize );
    virtual void getText( char *dest, int item, size_t destsize );
    virtual void newList( TCollection *aList );
    virtual void setData( void *rec );

    TCollection *list();

private:

    TCollection *items;

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TListBox( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TListBox& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TListBox*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TListBox& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TListBox* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

inline TCollection *TListBox::list()
{
    return items;
}

#endif  // Uses_TListBox


/* ---------------------------------------------------------------------- */
/*      class TStaticText                                                 */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Text                                                        */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TStaticText ) && !defined( __TStaticText )
#define __TStaticText

class TRect;

class TStaticText : public TView
{

public:

    TStaticText( const TRect& bounds, const char *aText );
    ~TStaticText();

    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void getText( char *buf, size_t bufsize );
    virtual void setText( const char *buf );

protected:

    const char *text;

private:

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TStaticText( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TStaticText& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TStaticText*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TStaticText& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TStaticText* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TStaticText


/* ---------------------------------------------------------------------- */
/*      class TParamText                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Text                                                        */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TParamText ) && !defined( __TParamText )
#define __TParamText

class TRect;

class TParamText : public TStaticText
{

public:
    TParamText( const TRect& bounds, const char *aText, int aParamCount );

    virtual size_t dataSize();
    virtual void getText( char *buf, size_t bufsize );
    virtual void setData( void *rec );

protected:

    short paramCount;
    void *paramList;

private:

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
    TParamText( StreamableInit );
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
     * Creates a new TParamText.
     *
     * Called to create an object in certain stream-reading situations.
     */
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TParamText& cl )
    { return is >> (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline ipstream& operator >> ( ipstream& is, TParamText*& cl )
    { return is >> (void *&)cl; }

/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TParamText& cl )
    { return os << (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TParamText* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TParamText


/* ---------------------------------------------------------------------- */
/*      class TLabel                                                      */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TLabel ) && !defined( __TLabel )
#define __TLabel

class TRect;
class TEvent;
class TView;

/**
 * Used to attach a label to a given view.
 *
 * A TLabel object is a piece of text in a view that can be selected
 * (highlighted) by a mouse click, cursor keys, or Alt-letter hot key.
 * The label is usually "attached" via a pointer (called @ref link) to some
 * other control view such as an input line, cluster, or list viewer to guide
 * the user.
 * @see TCluster
 * @see TInputLine
 * @see TListViewer
 *
 * Useful mainly with input lines, list boxes, check boxes and radio buttons,
 * since they don't have a default caption.
 * @see TCheckBoxes
 * @see TListBox
 * @see TRadioButtons
 * @short Used to attach a label to a view
 */
class TLabel : public TStaticText
{
public:
    /**
     * Constructor.
     *
     * Creates a TLabel object of the given size and text by calling
     * TStaticText::TStaticText(bounds, aText), then setting the @ref link
     * data member to `aLink' for the associated control (make `aLink' 0 if
     * no control is needed).
     * @see TStaticText::TStaticText
     *
     * `bounds' is the bounding rectangle of the view while `aText' is the
     * caption to show.
     * @see text
     *
     * The @ref options data member is set to @ref ofPreProcess and
     * @ref ofPostProcess. The @ref eventMask is set to @ref evBroadcast.
     * `aText' can designate a hot key letter for the label by surrounding
     * the letter with tildes, like "~F~ile".
     */
    TLabel( const TRect& bounds, const char *aText, TView *aLink );
    /**
     * Draws the label with the appropriate colors from the default palette.
     */
    virtual void draw();
    /**
     * Returns a reference to the label palette.
     */
    virtual TPalette& getPalette() const;
    /**
     * Handles TLabel events.
     *
     * Handles all events by calling @ref TView::handleEvent(). If an
     * @ref evMouseDown or hot key event is received, the appropriate linked
     * control (if any) is selected with link->select().
     * @see select
     *
     * handleEvent() also handles cmReceivedFocus and cmReleasedFocus
     * broadcast events from the linked control in order to adjust the
     * value of the @ref light data member and redraw the label as necessary.
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Releases TLabel resources.
     *
     * Used internally by @ref TObject::destroy() to ensure correct
     * destruction of derived and related objects. shutDown() is overridden
     * in many classes to ensure the proper setting of related data members
     * when @ref destroy() is called.
     *
     * This method releases all the resources allocated by the TLabel. It sets
     * pointer @ref link to 0 and then calls @ref TStaticText::shutDown().
     * Since @ref TStaticText::shutDown() is not implemented,
     * @ref TView::shutDown() will be called instead.
     */
    virtual void shutDown();
protected:
    /**
     * This is a pointer to the view to focus when the user selects this
     * label.
     */
    TView *link;
    /**
     * If True, the label and its linked control has been selected and will
     * be highlighted. Otherwise, light is set to False.
     */
    Boolean light;

private:

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
    TLabel( StreamableInit );
    /**
     * Used to store the view in a stream.
     * Writes to the output stream `os'.
     */
    virtual void write( opstream& os );
    /**
     * Used to recover the view from a stream.
     * Reads from the input stream `is'.
     */
    virtual void *read( ipstream& is );
public:
    /**
     * Undocumented.
     */
    static const char * const name;
    /**
     * Creates a new TLabel.
     *
     * Called to create an object in certain stream-reading situations.
     */
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TLabel& cl )
    { return is >> (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline ipstream& operator >> ( ipstream& is, TLabel*& cl )
    { return is >> (void *&)cl; }

/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TLabel& cl )
    { return os << (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TLabel* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TLabel


/* ---------------------------------------------------------------------- */
/*      class THistoryViewer                                              */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Active                                                      */
/*        2 = Inactive                                                    */
/*        3 = Focused                                                     */
/*        4 = Selected                                                    */
/*        5 = Divider                                                     */
/* ---------------------------------------------------------------------- */

#if defined( Uses_THistoryViewer ) && !defined( __THistoryViewer )
#define __THistoryViewer

class TRect;
class TScrollBar;

/**
 * THistoryViewer is a rather straightforward descendant of @ref TListViewer.
 * It is used by the history list system, and appears inside the history
 * window set up by clicking on the history icon.
 * @short Part of the history list system
 */
class THistoryViewer : public TListViewer
{
public:
    /**
     * Initializes the viewer list by first calling the TListViewer constructor
     * to set up the boundaries, a single column, and the two scroll bar
     * pointers passed in `aHScrollBar' and `aVScrollBar'.
     *
     * The view is then linked to a history list, with the @ref historyId data
     * member set to the value passed in `aHistory'. That list is then checked
     * for length, so the range of the list is set to the number of items in
     * the list.
     *
     * The first item in the history list is given the focus, and the
     * horizontal scrolling range is set to accommodate the widest item in the
     * list.
     */
    THistoryViewer( const TRect& bounds,
                    TScrollBar *aHScrollBar,
                    TScrollBar *aVScrollBar,
                    ushort aHistoryId
                  );
    /**
     * Returns the default palette string.
     */
    virtual TPalette& getPalette() const;
    /**
     * Set `dest' to the item'th string in the associated history list.
     * getText() is called by the @ref TListViewer::draw() member function for
     * each visible item in the list.
     */
    virtual void getText( char *dest, int item, size_t destsize );
    /**
     * The history viewer handles two kinds of events itself; all others are
     * passed to @ref TListViewer::handleEvent().
     *
     * -# Double clicking or pressing the Enter key terminates the modal state
     *    of the history window with a cmOK command.
     * -# Pressing the Esc key, or any cmCancel command event, cancels the
     *    history list selection.
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Returns the length of the longest string in the history list associated
     * with @ref historyId.
     */
    int historyWidth();
protected:
    /**
     * historyId is the ID number of the history list to be displayed in the
     * view.
     */
    ushort historyId;
};

#endif  // Uses_THistoryViewer

#if defined( Uses_THistoryWindow ) && !defined( __THistoryWindow )
#define __THistoryWindow

class TListViewer;
class TRect;
class TWindow;
class TInputLine;

/**
 * @ref THistoryWindow inherits multiply from @ref TWindow and the virtual
 * base class THistInit.
 *
 * THistInit provides a constructor and
 * @ref THistoryWindow::createListViewer() member function used in creating
 * and inserting a list viewer into a history window. A similar technique is
 * used for @ref TProgram, @ref TWindow and @ref TDeskTop.
 * @short Virtual base class for THistoryWindow
 */
class THistInit
{
public:
    /**
     * This constructor takes a function address argument `cListViewer',
     * usually &THistoryWindow::initViewer.
     * @see THistoryWindow::initViewer
     *
     * This creates and inserts a list viewer into the given history window
     * with the given size `bounds' and history list `histID'.
     */
    THistInit( TListViewer *(*cListViewer)( TRect r, TWindow *w,
        ushort histID ));
protected:
    /**
     * Called by the THistInit constructor to create a list viewer for the
     * window `w' with size `r' and history list given by `histId' and return
     * a pointer to it. A 0 pointer indicates lack of success in this
     * endeavor.
     */
    TListViewer *(*createListViewer)( TRect r, TWindow *w, ushort histId );
};

/* ---------------------------------------------------------------------- */
/*      THistoryWindow                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Frame passive                                               */
/*        2 = Frame active                                                */
/*        3 = Frame icon                                                  */
/*        4 = ScrollBar page area                                         */
/*        5 = ScrollBar controls                                          */
/*        6 = HistoryViewer normal text                                   */
/*        7 = HistoryViewer selected text                                 */
/* ---------------------------------------------------------------------- */

/**
 * THistoryWindow is a specialized descendant of @ref TWindow and
 * @ref THistInit (multiple inheritance) used for holding a history list
 * viewer when the user clicks on the history icon next to an input line.
 *
 * By default, the window has no title and no number. The history window's
 * frame has only a close icon: the window can be closed, but not resized or
 * zoomed.
 * @short Holds a history list viewer
 */
class THistoryWindow : public TWindow, public virtual THistInit
{
public:
    /**
     * Calls the THistInit constructor with the argument
     * &THistoryWindow::initViewer. This creates the list viewer.
     * @see THistInit::THistInit
     * @see THistoryWindow::initViewer
     *
     * Next, the @ref TWindow constructor is called to set up a window with the
     * given bounds, a null title string, and no window number
     * (@ref wnNoNumber).
     * @see TWindow::TWindow
     *
     * Then the @ref TWindowInit constructor is called with the argument
     * &THistoryWindow::initFrame to create a frame for the history window.
     * @see THistoryWindow::initFrame
     * @see TWindowInit::TWindowInit
     *
     * Finally, the @ref flags data member is set to @ref wfClose to provide a
     * close icon, and a history viewer object is created and inserted in the
     * history window to show the items in the history list given by
     * `historyId'.
     */
    THistoryWindow( const TRect& bounds, ushort historyId );
    /**
     * Returns the default palette string.
     */
    virtual TPalette& getPalette() const;
    /**
     * Returns in `dest' the string value of the @ref THistoryViewer::focused
     * item in the associated history viewer.
     */
    virtual void getSelection( char *dest, size_t destsize );
    /**
     * Instantiates and inserts a @ref THistoryViewer object inside the
     * boundaries of the history window for the list associated with the
     * ID `aHistoryId'.
     *
     * Standard scroll bars are placed on the frame of the window to scroll
     * the list.
     */
    static TListViewer *initViewer( TRect bounds, TWindow *w, ushort
        aHistoryId );
protected:
    /**
     * Points to the list viewer to be contained in this history window.
     */
    TListViewer *viewer;
};

#endif  // Uses_THistoryWindow

#if defined( Uses_THistory ) && !defined( __THistory )
#define __THistory

class TRect;
class TInputLine;
class TEvent;
class THistoryWindow;

/**
 * A THistory object implements a pick list of previous entries, actions, or
 * choices from which the user can select a "rerun". THistory objects are
 * linked to a @ref TInputLine object and to a history list.
 * @see THistoryWindow
 *
 * History list information is stored in a block of memory on the heap. When
 * the block fills up, the oldest history items are deleted as new ones are
 * added.
 *
 * Different input lines can share the same history list by using the same ID
 * number.
 * @short Implements a pick list of previous entries, actions, or choices from
 * which the user can select a "rerun"
 */
class THistory : public TView
{
public:
    /**
     * Creates a THistory object of the given size by calling
     * TView::TView(bounds), then setting the @ref link and @ref historyId
     * data members with the given argument values `aLink' and `aHistoryId'.
     * @see TView::TView
     *
     * The @ref options member is set to @ref ofPostProcess. The
     * @ref evBroadcast bit is set in @ref eventMask in addition to the
     * @ref evMouseDown, @ref evKeyDown, and @ref evCommand bits set by
     * TView(bounds).
     */
    THistory( const TRect& bounds, TInputLine *aLink, ushort aHistoryId );
    /**
     * Draws the THistory icon in the default palette.
     */
    virtual void draw();
    /**
     * Returns a reference to the default palette.
     */
    virtual TPalette& getPalette() const;
    /**
     * Calls TView::handleEvent(event), then handles relevant mouse and key
     * events to select the linked input line and create a history window.
     * @see TView::handleEvent
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Creates a THistoryWindow object and returns a pointer to it. The new
     * object has the given bounds and the same @ref historyId as the calling
     * THistory object.
     * @see THistoryWindow
     *
     * The new object gets its @ref helpCtx from the calling object's linked
     * @ref TInputLine.
     * @see THistory::link
     */
    virtual THistoryWindow *initHistoryWindow( const TRect& bounds );
    /**
     * Used internally by @ref TObject::destroy() to ensure correct
     * destruction of derived and related objects.
     *
     * shutDown() is overridden in many classes to ensure the proper setting
     * of related data members when @ref destroy() is called.
     */
    virtual void shutDown();
    /**
     * Undocumented.
     */
    static const char * icon;
protected:
    /**
     * A pointer to the linked TInputLine object.
     */
    TInputLine *link;
    /**
     * Each history list has a unique ID number, assigned by the programmer.
     *
     * Different history objects in different windows may share a history list
     * by using the same history ID.
     */
    ushort historyId;

private:
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
    THistory( StreamableInit );
    /**
     * Writes to the output stream `os'.
     */
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, THistory& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, THistory*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, THistory& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, THistory* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_THistory

