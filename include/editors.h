/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   EDITORS.H                                                             */
/*                                                                         */
/*   Copyright (c) Borland International 1991                              */
/*   All Rights Reserved.                                                  */
/*                                                                         */
/*   defines the classes TIndicator, TEditor, TMemo, TFileEditor,          */
/*   and TEditWindow                                                       */
/*                                                                         */
/* ------------------------------------------------------------------------*/

#ifdef __BORLANDC__
#if !defined( __DIR_H )
#include <dir.h>
#endif  // __DIR_H
#endif

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __LIMITS_H )
#include <limits.h>
#endif  // __LIMITS_H

#include <cm_codes.h>

#if !defined( __EDIT_COMMAND_CODES )
#define __EDIT_COMMAND_CODES

const int
  ufUpdate = 0x01,
  ufLine   = 0x02,
  ufView   = 0x04;

const int
  smExtend = 0x01,
  smDouble = 0x02;

const unsigned int
  sfSearchFailed = UINT_MAX; // 0xFFFF;

  /** \var edOutOfMemory
   * @see TEditor::doSearchReplace
   */
const int
  edOutOfMemory   = 0,

  /** \var edReadError
   * @see TEditor::doSearchReplace
   */
  edReadError     = 1,

  /** \var edWriteError
   * @see TEditor::doSearchReplace
   */
  edWriteError    = 2,

  /** \var edCreateError
   * @see TEditor::doSearchReplace
   */
  edCreateError   = 3,

  /** \var edSaveModify
   * @see TEditor::doSearchReplace
   */
  edSaveModify    = 4,

  /** \var edSaveUntitled
   * @see TEditor::doSearchReplace
   */
  edSaveUntitled  = 5,

  /** \var edSaveAs
   * @see TEditor::doSearchReplace
   */
  edSaveAs        = 6,

  /** \var edFind
   * @see TEditor::doSearchReplace
   */
  edFind          = 7,

  /** \var edSearchFailed
   * @see TEditor::doSearchReplace
   */
  edSearchFailed  = 8,

  /** \var edReplace
   * @see TEditor::doSearchReplace
   */
  edReplace       = 9,

  /** \var edReplacePrompt
   * @see TEditor::doSearchReplace
   */
  edReplacePrompt = 10;

  /** \var efCaseSensitive
   * Default to case-sensitive search.
   * @see TEditor::editorFlags
   */
const int
  efCaseSensitive   = 0x0001,

  /** \var efWholeWordsOnly
   * Default to whole words only search.
   * @see TEditor::editorFlags
   */
  efWholeWordsOnly  = 0x0002,

  /** \var efPromptOnReplace
   * Prompt on replace.
   * @see TEditor::editorFlags
   */
  efPromptOnReplace = 0x0004,

  /** \var efReplaceAll
   * Replace all occurrences.
   * @see TEditor::editorFlags
   */
  efReplaceAll      = 0x0008,

  /** \var efDoReplace
   * Do replace.
   * @see TEditor::editorFlags
   */
  efDoReplace       = 0x0010,

  /** \var efBackupFiles
   * Create backup files with a trailing ~ on saves.
   * @see TEditor::editorFlags
   */
  efBackupFiles     = 0x0100;

  /** \var maxLineLength
   * Maximum allowed line length for text in a TEditor view.
   */
const
  int maxLineLength = 256;

#endif  // __EDIT_COMMAND_CODES

/** \enum TEditorDialog
 * The TEditorDialog data type is a pointer to function returning ushort
 * and taking one int argument and a variable number of additional
 * arguments.
 *
 * Since dialog boxes are very application-dependent, a @ref TEditor object
 * does not display its own dialog boxes directly. Instead it controls
 * them through this function pointer.
 *
 * The various dialog values, passed in the first int argument, are
 * self-explanatory: @ref edOutOfMemory, @ref edReadError, @ref edWriteError,
 * @ref edCreateError, @ref edSaveModify, @ref edSaveUntitled, @ref edSaveAs,
 * @ref edFind, @ref edSearchFailed, @ref edReplace and @ref edReplacePrompt.
 * @see TEditor::editorDialog
 */
typedef ushort (*TEditorDialog)( int, ... );

/** \fn defEditorDialog( int dialog, ... )
 * Since dialog boxes are very application-dependent, a @ref TEditor object
 * does not display its own dialog boxes directly. Instead it controls
 * them through the @ref TEditorDialog function pointer.
 * @see TEditor::editorDialog
 *
 * This is the default dialog; it simply returns cmCancel.
 */
ushort defEditorDialog( int dialog, ... );

#if defined( Uses_TIndicator ) && !defined( __TIndicator )
#define __TIndicator

class TRect;

/**
 * TIndicator is the line and column counter in the lower left corner of the
 * edit window.
 * It is initialized by the @ref TEditWindow constructor and passed as the
 * fourth argument to the @ref TEditor constructor.
 * @short The line and column counter in the lower left corner of the edit
 * window
 */
class TIndicator : public TView
{
public:
    /**
     * Creates a TIndicator object.
     */
    TIndicator( const TRect& );
    /**
     * Draws the indicator. If variable @ref modified is True, a special
     * character (ASCII value 15, a star on PC graphic adapters) is displayed.
     */
    virtual void draw();
    /**
     * Returns the TIndicator default palette.
     */
    virtual TPalette& getPalette() const;
    /**
     * Draws the indicator in the frame-dragging color if the view is being
     * dragged.
     */
    virtual void setState( ushort, Boolean );
    /**
     * Method called by @ref TEditor to update and display the values of the
     * data members of the associated TIndicator object.
     * @see TIndicator::location
     * @see TIndicator::modified
     */
    void setValue( const TPoint& aLocation, Boolean aModified );
    /**
     * Undocumented.
     */
    static char dragFrame;
    /**
     * Undocumented.
     */
    static char normalFrame;
protected:
    /**
     * Stores the location to display. Updated by a TEditor.
     */
    TPoint location;
    /**
     * True if the associated TEditor has been modified.
     */
    Boolean modified;

private:
#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TIndicator( StreamableInit );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TIndicator& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TIndicator*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TIndicator& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TIndicator* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TIndicator


#if defined( Uses_TEditor ) && !defined( __TEditor )
#define __TEditor

class TRect;
class TScrollBar;
class TIndicator;
class TEvent;

class TEditor : public TView
{

public:

    friend void genRefs();

    TEditor( const TRect&, TScrollBar *, TScrollBar *, TIndicator *, size_t );
    virtual ~TEditor();

    virtual void shutDown();

    char TV_CDECL bufChar( size_t );
    size_t TV_CDECL bufPtr( size_t );
    virtual void changeBounds( const TRect& );
    virtual void convertEvent( TEvent& );
    Boolean cursorVisible();
    void deleteSelect();
    virtual void doneBuffer();
    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& );
    virtual void initBuffer();
    Boolean insertBuffer( char *, size_t, size_t, Boolean, Boolean );
    virtual Boolean insertFrom( TEditor * );
    Boolean insertText( const void *, size_t, Boolean );
    void scrollTo( int, int );
    Boolean search( const char *, ushort );
    virtual Boolean setBufSize( size_t );
    void setCmdState( ushort, Boolean );
    void setSelect( size_t, size_t, Boolean);
    virtual void setState( ushort, Boolean );
    void trackCursor( Boolean );
    void undo();
    virtual void updateCommands();
    virtual Boolean valid( ushort );

    int charPos( size_t, size_t );
    size_t charPtr( size_t, int );
    Boolean clipCopy();
    void clipCut();
    void clipPaste();
    void deleteRange( size_t, size_t, Boolean );
    void doUpdate();
    void doSearchReplace();
    void drawLines( int, int, size_t );
    void TV_CDECL formatLine(void *, size_t, int, ushort );
    void find();
    size_t getMousePtr( TPoint );
    Boolean hasSelection();
    void hideSelect();
    Boolean isClipboard();
    size_t TV_CDECL lineEnd( size_t );
    size_t lineMove( size_t, int );
    size_t TV_CDECL lineStart( size_t );
    void lock();
    void newLine();
    size_t TV_CDECL nextChar( size_t );
    size_t nextLine( size_t );
    size_t nextWord( size_t );
    size_t TV_CDECL prevChar( size_t );
    size_t prevLine( size_t );
    size_t prevWord( size_t );
    void replace();
    void setBufLen( size_t );
    void setCurPtr( size_t, uchar );
    void startSelect();
    void toggleInsMode();
    void unlock();
    void update( uchar );
    void checkScrollBar( const TEvent&, TScrollBar *, int& );

    TScrollBar *hScrollBar;
    TScrollBar *vScrollBar;
    TIndicator *indicator;
    char *buffer;
    size_t bufSize;
    size_t bufLen;
    size_t gapLen;
    size_t selStart;
    size_t selEnd;
    size_t curPtr;
    TPoint curPos;
    TPoint delta;
    TPoint limit;
    int drawLine;
    size_t drawPtr;
    size_t delCount;
    size_t insCount;
    Boolean isValid;
    Boolean canUndo;
    Boolean modified;
    Boolean selecting;
    Boolean overwrite;
    Boolean autoIndent;

    static TEditorDialog editorDialog;
    static ushort editorFlags;
    static char findStr[maxFindStrLen];
    static char replaceStr[maxReplaceStrLen];
    static TEditor *clipboard;
    uchar lockCount;
    uchar updateFlags;
    int keyState;

private:

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TEditor( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TEditor& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TEditor*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TEditor& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TEditor* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TEditor

#if defined( Uses_TMemo ) && !defined( __TMemo )
#define __TMemo

class TEvent;

struct TMemoData
{
    size_t length;
    char buffer[];
};

class TMemo : public TEditor
{

public:

    TMemo( const TRect&, TScrollBar *, TScrollBar *, TIndicator *, size_t );
    virtual void getData( void *rec, size_t recsize );
    virtual void setData( void *rec );
    virtual size_t dataSize();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& );

private:

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TMemo( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TMemo& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TMemo*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TMemo& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TMemo* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TMemo


#if defined( Uses_TFileEditor ) && !defined( __TFileEditor )
#define __TFileEditor

#ifdef __IDA__
#include <prodir.h>
#endif

class TRect;
class TScrollBar;
class TIndicator;
class TEvent;

class TFileEditor : public TEditor
{

public:

    char fileName[MAXPATH];
    TFileEditor( const TRect&,
                 TScrollBar *,
                 TScrollBar *,
                 TIndicator *,
                 const char *
               );
    virtual void doneBuffer();
    virtual void handleEvent( TEvent& );
    virtual void initBuffer();
    Boolean loadFile();
    Boolean save();
    Boolean saveAs();
    Boolean saveFile();
    virtual Boolean setBufSize( size_t );
    virtual void shutDown();
    virtual void updateCommands();
    virtual Boolean valid( ushort );

private:

    static const char *backupExt;

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TFileEditor( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TFileEditor& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TFileEditor*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TFileEditor& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TFileEditor* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TFileEditor


#if defined( Uses_TEditWindow ) && !defined( __TEditWindow )
#define __TEditWindow

class TFileEditor;

class TEditWindow : public TWindow
{

public:

    TEditWindow( const TRect&, const char *, int );
    virtual void close();
    virtual const char *getTitle( short );
    virtual void handleEvent( TEvent& );
    virtual void sizeLimits( TPoint& min, TPoint& max );

    TFileEditor *editor;

private:

    static const char *clipboardTitle;
    static const char *untitled;

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TEditWindow( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TEditWindow& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TEditWindow*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TEditWindow& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TEditWindow* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TEditWindow


#if defined( Uses_TFindDialogRec ) && !defined( __TFindDialogRec )
#define __TFindDialogRec

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

struct TFindDialogRec
{
    TFindDialogRec( const char *str, ushort flgs )
        {
        qstrncpy( find, str, sizeof(find) );
        options = flgs;
        }
    char find[maxFindStrLen];
    ushort options;
};

#endif  // Uses_TFindDialogRec

#if defined( Uses_TReplaceDialogRec ) && !defined( __TReplaceDialogRec )
#define __TReplaceDialogRec

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

struct TReplaceDialogRec
{
    TReplaceDialogRec( const char *str, const char *rep, ushort flgs )
        {
        qstrncpy( find, str, sizeof(find) );
        qstrncpy( replace, rep, sizeof(replace) );
        options = flgs;
        }
    char find[maxFindStrLen];
    char replace[maxReplaceStrLen];
    ushort options;
};

#endif  // Uses_TReplaceDialogRec

