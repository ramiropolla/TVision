/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   MENUS.H                                                               */
/*                                                                         */
/*   Copyright (c) Borland International 1991                              */
/*   All Rights Reserved.                                                  */
/*                                                                         */
/*   defines the classes TMenuItem, TMenu, TMenuView, TSubMenu,            */
/*   TMenuBar, TMenuBox, TStatusItem, TStatusDef, and TStatusLine          */
/*                                                                         */
/* ------------------------------------------------------------------------*/

class TSubMenu;
class TMenuItem;
class TStatusDef;
class TStatusItem;

/**
 * Undocumented.
 */
TSubMenu& operator + ( TSubMenu& s, TMenuItem& i );
/**
 * Undocumented.
 */
TSubMenu& operator + ( TSubMenu& s1, TSubMenu& s2 );
/**
 * Undocumented.
 */
TStatusDef& operator + ( TStatusDef& s1, TStatusItem& s2 );
/**
 * Undocumented.
 */
TStatusDef& operator + ( TStatusDef& s1, TStatusDef& s2 );

#if defined( Uses_TMenuItem ) && !defined( __TMenuItem )
#define __TMenuItem

/** \file menus.h
 * menus.h
 */

class TMenu;

/**
 * Instances of TMenuItem serve as elements of a menu.
 *
 * They can be individual menu items that cause a command to be generated or
 * a @ref TSubMenu pull-down menu that contains other TMenuItem instances.
 * TMenuItem's different constructors set the data members appropriately.
 * TMenuItem also serves as a base class for @ref TSubMenu.
 * @short Instances of TMenuItem serve as elements of a menu
 */
class TMenuItem
{
public:
    /**
     * Creates an individual menu item with the given values. Data member
     * @ref disabled is set if `aCommand' is disabled.
     */
    TMenuItem( const char *aName,
               ushort aCommand,
               ushort aKeyCode,
               ushort aHelpCtx = hcNoContext,
               char *p = 0,
               TMenuItem *aNext = 0
             );
    /**
     * Creates a pull-down submenu object with the given values. Data member
     * @ref command is set to zero.
     */
    TMenuItem( const char *aName,
               ushort aKeyCode,
               TMenu *aSubMenu,
               ushort aHelpCtx = hcNoContext,
               TMenuItem *aNext = 0
             );
    /**
     * If @ref param is used, deallocates the space used to store @ref name
     * and @ref param. If @ref subMenu is used, deallocates space used to
     * store @ref name and @ref subMenu.
     */
    ~TMenuItem();
    /**
     * Appends the given TMenuItem to the list of TMenuItems by setting
     * @ref next data member to `aNext'.
     */
    void append( TMenuItem *aNext );
    /**
     * A non-zero @ref next points to the next TMenuItem object in the
     * linked list associated with a menu. If @ref next = 0, this is the
     * last item in the list.
     */
    TMenuItem *next;
    /**
     * The name of the item that appears in the menu box.
     */
    const char *name;
    /**
     * The command word of the event generated when this menu item is selected
     * if there isn't a submenu item.
     */
    ushort command;
    /**
     * True if the menu item is disabled. The menu item will be drawn using
     * the appropriate palette entry.
     */
    Boolean disabled;
    /**
     * The scan code for the associated hot key.
     */
    ushort keyCode;
    /**
     * The menu item's help context. When the menu item is selected, this
     * data member represents the help context of the application, unless the
     * context number is @ref hcNoContext, in which case there is no help
     * context.
     * @see TView::getHelpCtx
     * @see TView::helpCtx
     */
    ushort helpCtx;
    union
        {
        /**
         * @ref param is used to display the hot key associated with this menu
         * item.
         */
        const char *param;
        /**
          * @ref subMenu points to the submenu to be created when this menu
	  * item is selected, if a command is not generated.
          */
        TMenu *subMenu;
        };
};

inline void TMenuItem::append( TMenuItem *aNext )
{
    next = aNext;
}

/**
 * Undocumented.
 */
inline TMenuItem &newLine()
{
    return *new TMenuItem( 0, 0, 0, hcNoContext, 0, 0 );
}

#endif  // Uses_TMenuItem

#if defined( Uses_TSubMenu ) && !defined( __TSubMenu )
#define __TSubMenu

/**
 * TSubMenu is a class used to differentiate between different types of
 * @ref TMenuItem: individual menu items and submenus.
 *
 * TVision supplies the overloaded operator + so you can easily construct
 * complete menus without dozens of nested parentheses. When you use
 * TSubMenu, the compiler can distinguish between attempts to use operator +
 * on individual menu items and their submenus.
 * @short Used to differentiate between different types of TMenuItem:
 * individual menu items and submenus
 */
class TSubMenu : public TMenuItem
{

public:
    /**
     * Calls constructor TMenuItem(nm, 0, key, helpCtx).
     * @see TMenuItem::TMenuItem
     */
    TSubMenu( const char *nm, ushort key, ushort helpCtx = hcNoContext );
};

#endif  // Uses_TSubMenu

#if defined( Uses_TMenu ) && !defined( __TMenu )
#define __TMenu

/**
 * TMenu serves as a "wrapper" for the various other menu classes, such as
 * @ref TMenuItem, @ref TSubMenu and @ref TMenuView.
 * @short A "wrapper" for the various other menu classes, such as TMenuItem,
 * TSubMenu and TMenuView
 */
class TMenu
{
public:
    /**
     * Creates a TMenu object and sets both @ref deflt and @ref items data
     * members to 0.
     */
    TMenu() : items(0), deflt(0) {};
    /**
     * Creates a TMenu object consisting of `itemList'; sets @ref items and
     * @ref deflt data members to that item.
     */
    TMenu( TMenuItem& itemList )
        { items = &itemList; deflt = &itemList; }
    /**
     * Creates a TMenu object; sets data members @ref items and @ref deflt
     * to `itemList' and `TheDefault', respectively.
     */
    TMenu( TMenuItem& itemList, TMenuItem& TheDefault )
        { items = &itemList; deflt = &TheDefault; }
    /**
     * Deletes the menu item list in @ref items.
     */
    ~TMenu();
    /**
     * Points to the list of menu items. Used by various draw member
     * functions when parts of the menu structure need to be redrawn.
     */
    TMenuItem *items;
    /**
     * Points to the default (highlighted) menu item. Determines how to react
     * when the user presses Enter.
     */
    TMenuItem *deflt;
};

#endif  // Uses_TMenu

/* ---------------------------------------------------------------------- */
/*      class TMenuView                                                   */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Disabled text                                               */
/*        3 = Shortcut text                                               */
/*        4 = Normal selection                                            */
/*        5 = Disabled selection                                          */
/*        6 = Shortcut selection                                          */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TMenuView ) && !defined( __TMenuView )
#define __TMenuView

class TRect;
class TMenu;
class TEvent;

/**
 * TMenuView provides an abstract base from which menu bar and menu box
 * classes (either pull down or pop up) are derived. You cannot instantiate a
 * TMenuView itself.
 * @short An abstract base from which menu bar and menu box classes (either
 * pull down or pop up) are derived
 */
class TMenuView : public TView
{
public:
    /**
     * Undocumented.
     */
    TMenuView( const TRect& bounds, TMenu *aMenu, TMenuView *aParent = 0 );
    /**
     * Calls TView constructor to create a TMenuView object of size `bounds'.
     * @see TView::TView
     *
     * The current @ref parentMenu and @ref menu pointers are set to 0.
     * The default @ref eventMask is set to @ref evBroadcast.
     */
    TMenuView( const TRect& bounds );
    /**
     * Undocumented.
     */
    void setBounds( const TRect& bounds );
    /**
     * Executes a menu view until the user selects a menu item or cancels the
     * process. Returns the command assigned to the selected menu item, or
     * zero if the menu was canceled.
     *
     * Should never be called except by @ref TGroup::execView().
     */
    virtual ushort execute();
    /**
     * Returns a pointer to the menu item that has toupper(ch) as its hot key
     * (the highlighted character). Returns 0 if no such menu item is found or
     * if the menu item is disabled. Note that findItem() is case insensitive.
     */
    TMenuItem *findItem( char ch );
    /**
     * Classes derived from TMenuView must override this member function in
     * order to respond to mouse events. Your overriding functions in derived
     * classes must return the rectangle occupied by the given menu item.
     */
    virtual TRect getItemRect( TMenuItem *item );
    /**
     * By default, this member function returns the help context of the
     * current menu selection. If this is @ref hcNoContext, the parent menu's
     * current context is checked. If there is no parent menu, getHelpCtx()
     * returns @ref hcNoContext.
     * @see helpCtx
     */
    virtual ushort getHelpCtx();
    /**
     * Returns the default palette string.
     */
    virtual TPalette& getPalette() const;
    /**
     * Called whenever a menu event needs to be handled. Determines which menu
     * item has been mouse or keyboard selected (including hot keys) and
     * generates the appropriate command event with @ref putEvent().
     */
    virtual void handleEvent( TEvent& event );
    /**
     * Returns a pointer to the menu item associated with the hot key given
     * by `keyCode'. Returns 0 if no such menu item exists, or if the item is
     * disabled. hotKey() is used by @ref handleEvent() to determine whether a
     * keystroke event selects an item in the menu.
     */
    TMenuItem *hotKey( ushort keyCode );
    /**
     * Undocumented.
     */
    TMenuView *newSubView( const TRect& bounds,
                           TMenu *aMenu,
                           TMenuView *aParentMenu
                         );
protected:
    /**
     * A pointer to the TMenuView object (or any class derived from TMenuView)
     * that owns this menu.
     */
    TMenuView *parentMenu;
    /**
     * A pointer to the @ref TMenu object for this menu, which provides a
     * linked list of menu items. The menu pointer allows access to all the
     * data members of the menu items in this menu view.
     */
    TMenu *menu;
    /**
     * A pointer to the currently selected menu item.
     */
    TMenuItem *current;

private:

    void nextItem();
    void prevItem();
    void trackKey( Boolean findNext );
    Boolean mouseInOwner( TEvent& e );
    Boolean mouseInMenus( TEvent& e );
    void trackMouse( TEvent& e );
    TMenuView *topMenu();
    uchar updateMenu( TMenu *menu );
    void do_a_select( TEvent& );
    TMenuItem *findHotKey( TMenuItem *p, ushort keyCode );

private:

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }
    static void writeMenu( opstream&, TMenu * );
    static TMenu *readMenu( ipstream& );

protected:
    /**
     * Each streamable class needs a "builder" to allocate the correct memory
     * for its objects together with the initialized virtual table pointers.
     * This is achieved by calling this constructor with an argument of type
     * @ref StreamableInit.
     */
    TMenuView( StreamableInit );
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
inline ipstream& operator >> ( ipstream& is, TMenuView& cl )
    { return is >> (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline ipstream& operator >> ( ipstream& is, TMenuView*& cl )
    { return is >> (void *&)cl; }

/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TMenuView& cl )
    { return os << (TStreamable&)cl; }
/**
 * Undocumented.
 */
inline opstream& operator << ( opstream& os, TMenuView* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

inline TMenuView::TMenuView( const TRect& bounds,
                             TMenu *aMenu,
                             TMenuView *aParent
                           ) :
    TView(bounds),
    parentMenu( aParent ),
    menu( aMenu ),
    current( 0 )
{
     eventMask |= evBroadcast;
}

inline TMenuView::TMenuView( const TRect& bounds ) :
    TView(bounds), parentMenu(0), menu(0), current(0)
{
     eventMask |= evBroadcast;
}

#endif  // Uses_TMenuView

/* ---------------------------------------------------------------------- */
/*      class TMenuBar                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Disabled text                                               */
/*        3 = Shortcut text                                               */
/*        4 = Normal selection                                            */
/*        5 = Disabled selection                                          */
/*        6 = Shortcut selection                                          */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TMenuBar ) && !defined( __TMenuBar )
#define __TMenuBar

class TRect;
class TMenu;

/**
 * TMenuBar objects represent the horizontal menu bars from which menu
 * selections can be made by:
 *
 * -# direct clicking
 * -# F10 selection and hot keys
 * -# selection (highlighting) and pressing Enter
 * -# hot keys
 *
 * The main menu selections are displayed in the top menu bar. This is
 * represented by an object of type TMenuBar, usually owned by your
 * @ref TApplication object.
 *
 * Submenus are displayed in objects of type @ref TMenuBox. Both TMenuBar
 * and @ref TMenuBox are derived from @ref TMenuView (which is in turn derived
 * from @ref TView).
 *
 * For most TVision applications, you will not be involved directly with menu
 * objects. By overriding @ref TApplication::initMenuBar() with a suitable
 * set of nested new @ref TMenuItem and new @ref TMenu calls, TVision takes
 * care of all the standard menu mechanisms.
 * @short The horizontal menu bar from which you make menu selections
 */
class TMenuBar : public TMenuView
{
public:
    /**
     * Creates a menu bar by calling TMenuView::TMenuView(bounds). The
     * @ref growMode data member is set to @ref gfGrowHiX. The @ref options
     * data member is set to @ref ofPreProcess to allow hot keys to operate.
     * @see TMenuView::TMenuView
     *
     * The @ref menu data member is set to `aMenu', providing the menu
     * selections.
     */
    TMenuBar( const TRect& bounds, TMenu *aMenu );
    /**
     * Undocumented.
     */
    TMenuBar( const TRect& bounds, TSubMenu &aMenu );
    /**
     * Undocumented.
     */
    ~TMenuBar();
    /**
     * Draws the menu bar with the default palette. The @ref TMenuItem::name
     * and @ref TMenuItem::disabled data members of each @ref TMenuItem
     * object in the menu linked list are read to give the menu legends in
     * the correct colors.
     * The current (selected) item is highlighted.
     */
    virtual void draw();
    /**
     * Returns the rectangle occupied by the given menu item. It can be used
     * with @ref mouseInView() to determine if a mouse click has occurred on
     * a given menu selection.
     */
    virtual TRect getItemRect( TMenuItem *item );

private:

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TMenuBar( StreamableInit );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TMenuBar& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TMenuBar*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TMenuBar& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TMenuBar* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TMenuBar

/* ---------------------------------------------------------------------- */
/*      class TMenuBox                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Disabled text                                               */
/*        3 = Shortcut text                                               */
/*        4 = Normal selection                                            */
/*        5 = Disabled selection                                          */
/*        6 = Shortcut selection                                          */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TMenuBox ) && !defined( __TMenuBox )
#define __TMenuBox

class TRect;
class TMenu;
class TMenuView;
class TDrawBuffer;

class TMenuBox : public TMenuView
{

public:

    TMenuBox( const TRect& bounds, TMenu *aMenu, TMenuView *aParentMenu);

    virtual void draw();
    virtual TRect getItemRect( TMenuItem *item );

    static const char * frameChars;
private:

    void frameLine( TDrawBuffer&, short n );
    void drawLine( TDrawBuffer& );

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TMenuBox( StreamableInit );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TMenuBox& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TMenuBox*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TMenuBox& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TMenuBox* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TMenuBox


#if defined( Uses_TStatusItem ) && !defined( __TStatusItem )
#define __TStatusItem

class TStatusItem
{

public:

    TStatusItem( const char *aText,
                 ushort key,
                 ushort cmd,
                 TStatusItem *aNext = 0
                );

    TStatusItem *next;
    const char *text;
    ushort keyCode;
    ushort command;

};

inline TStatusItem::TStatusItem( const char *aText,
                                 ushort key,
                                 ushort cmd,
                                 TStatusItem *aNext
                                ) :
    next( aNext ), text( aText ), keyCode( key ), command( cmd )
{
}

#endif  // Uses_TStatusItem

#if defined( Uses_TStatusDef ) && !defined( __TStatusDef )
#define __TStatusDef

class TStatusDef
{

public:

    TStatusDef( ushort aMin,
                ushort aMax,
                TStatusItem *someItems = 0,
                TStatusDef *aNext = 0
              );

    TStatusDef *next;
    ushort lmin;
    ushort lmax;
    TStatusItem *items;
};

inline TStatusDef::TStatusDef( ushort aMin,
                               ushort aMax,
                               TStatusItem *someItems,
                               TStatusDef *aNext
                             ) :
    next( aNext ), lmin( aMin ), lmax( aMax ), items( someItems )
{
}

#endif  // Uses_TStatusDef

/* ---------------------------------------------------------------------- */
/*      class TStatusLine                                                 */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Disabled text                                               */
/*        3 = Shortcut text                                               */
/*        4 = Normal selection                                            */
/*        5 = Disabled selection                                          */
/*        6 = Shortcut selection                                          */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TStatusLine ) && !defined( __TStatusLine )
#define __TStatusLine

class TRect;
class TEvent;
class TPoint;

/**
 * The TStatusLine object is a specialized view, usually displayed at the
 * bottom of the screen. Typical status line displays are lists of available
 * hot keys, displays of available memory, time of day, current edit modes,
 * and hints for users.
 *
 * Status line items are @ref TStatusItem objects which contain data members
 * for a text string to be displayed on the status line, a key code to bind
 * a hot key, and a command to be generated if the displayed text is clicked
 * on with the mouse or the hot key is pressed.
 * @short A specialized view, usually displayed at the bottom of the screen
 */
class TStatusLine : public TView
{
public:
    /**
     * Creates a TStatusLine object with the given bounds by calling
     * TView::TView(bounds).
     * @see TView::TView
     *
     * The @ref ofPreProcess bit in @ref options is set, @ref eventMask is
     * set to include @ref evBroadcast, and @ref growMode is set to
     * @ref gfGrowLoY | @ref gfGrowHiX | @ref gfGrowHiY.
     *
     * The @ref defs data member is set to `aDefs'. If `aDefs' is 0,
     * @ref items is set to 0; otherwise, @ref items is set to aDefs->items.
     */
    TStatusLine( const TRect& bounds, TStatusDef& aDefs );
    /**
     * Disposes of all the @ref items and @ref defs in the TStatusLine
     * object, then calls @ref TView destructor.
     * @see TView::~TView
     */
    ~TStatusLine();
    /**
     * Draws the status line by writing the text string for each status item
     * that has one, then any hints defined for the current help context,
     * following a divider bar. Uses the appropriate palettes depending on
     * each item's status.
     */
    virtual void draw();
    /**
     * Returns the default palette string.
     */
    virtual TPalette& getPalette() const;
    /**
     * Handles events sent to the status line by calling
     * @ref TView::handleEvent(), then checking for three kinds of special
     * events.
     *
     * -# Mouse clicks that fall within the rectangle occupied by any status
     *    item generate a command event, with event.what set to the command in
     *    that status item.
     * -# Key events are checked against the keyCode data member in each
     *    item; a match causes a command event with that item's command.
     * -# Broadcast events with the command cmCommandSetChanged cause the
     *    status line to redraw itself to reflect any hot keys that might have
     *    been enabled or disabled.
     */
    virtual void handleEvent( TEvent& event );
    /**
     * By default, hint() returns a 0 string. Override it to provide a
     * context-sensitive hint string for the `aHelpCtx' argument. A nonzero
     * string will be drawn on the status line after a divider bar.
     * @see getHelpCtx
     * @see helpCtx
     */
    virtual const char* hint( ushort aHelpCtx );
    /**
     * Updates the status line by selecting the correct items from the lists
     * in @ref defs data member, depending on the current help context.
     *
     * Then calls @ref drawView() to redraw the status line if the items have
     * changed.
     */
    void update();
    /**
     * Undocumented.
     */
    static const char * hintSeparator;
protected:
    /**
     * A pointer to the current linked list of @ref TStatusItem records.
     */
    TStatusItem *items;
    /**
     * A pointer to the current linked list of @ref TStatusDef objects. The
     * list to use is determined by the current help context.
     */
    TStatusDef *defs;

private:

    void drawSelect( TStatusItem *selected );
    void findItems();
    TStatusItem *itemMouseIsIn( TPoint );
    void disposeItems( TStatusItem *item );
#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

    static void writeItems( opstream&, TStatusItem * );
    static void writeDefs( opstream&, TStatusDef * );
    static TStatusItem *readItems( ipstream& );
    static TStatusDef *readDefs( ipstream& );


protected:

    TStatusLine( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TStatusLine& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TStatusLine*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TStatusLine& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TStatusLine* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TStatusLine

