/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   STDDLG.H                                                              */
/*                                                                         */
/*   Copyright (c) Borland International 1991                              */
/*   All Rights Reserved.                                                  */
/*                                                                         */
/*   defines the classes TFileInputLine, TFileCollection, TSortedListBox,  */
/*   TFileList, TFileInfoPane, TFileDialog, TDirCollection, TDirListBox,   */
/*   and TChDirDialog                                                      */
/*                                                                         */
/* ------------------------------------------------------------------------*/

#ifdef __BORLANDC__
#pragma warn -hid
#endif
#if defined(_MSC_VER)
#pragma warning(disable:4263 4264)  // hidden virtual functions
#endif


#include <cm_codes.h>

#if defined( Uses_TSearchRec ) && !defined( __TSearchRec )
#define __TSearchRec

#ifdef __IDA__
#include <prodir.h>
#endif

struct dos_ftime
{
    unsigned    ft_tsec  : 5;   /* Two second interval */
    unsigned    ft_min   : 6;   /* Minutes */
    unsigned    ft_hour  : 5;   /* Hours */
    unsigned    ft_day   : 5;   /* Days */
    unsigned    ft_month : 4;   /* Months */
    unsigned    ft_year  : 7;   /* Year */
};

struct TSearchRec
{
    uchar attr;
    int32 time;
    int32 size;
    char name[MAXFILE+MAXEXT-1];
};

#endif  // Uses_TSearchRec

#if defined( Uses_TFileInputLine ) && !defined( __TFileInputLine )
#define __TFileInputLine

class TRect;
class TEvent;

class TFileInputLine : public TInputLine
{

public:

    TFileInputLine( const TRect& bounds, short aMaxLen );

    virtual void handleEvent( TEvent& event );

private:

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TFileInputLine( StreamableInit );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TFileInputLine& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TFileInputLine*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TFileInputLine& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TFileInputLine* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TFileInputLine

#if defined( Uses_TFileCollection ) && !defined( __TFileCollection )
#define __TFileCollection

struct TSearchRec;

class TFileCollection: public TSortedCollection
{

public:

    TFileCollection( ccIndex aLimit, ccIndex aDelta) :
        TSortedCollection( aLimit, aDelta ) {}

    TSearchRec *at( ccIndex index )
        { return (TSearchRec *)TSortedCollection::at( index ); }
    virtual ccIndex indexOf( TSearchRec *item )
        { return TSortedCollection::indexOf( item ); }

    void remove( TSearchRec *item )
        { TSortedCollection::remove( item ); }
    void free( TSearchRec *item )
        { TSortedCollection::free( item ); }
    void atInsert( ccIndex index, TSearchRec *item )
        { TSortedCollection::atInsert( index, item ); }
    void atPut( ccIndex index, TSearchRec *item )
        { TSortedCollection::atPut( index, item ); }
    virtual ccIndex insert( TSearchRec *item )
        { return TSortedCollection::insert( item ); }

    TSearchRec *firstThat( ccTestFunc Test, void *arg );
    TSearchRec *lastThat( ccTestFunc Test, void *arg );

private:

    virtual void freeItem( void *item )
        { delete (TSearchRec *)item; }

    virtual int compare( void *key1, void *key2 );

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

    virtual void *readItem( ipstream& );
    virtual void writeItem( void *, opstream& );

protected:

    TFileCollection( StreamableInit ) : TSortedCollection ( streamableInit ) {}

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TFileCollection& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TFileCollection*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TFileCollection& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TFileCollection* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

inline TSearchRec *TFileCollection::firstThat( ccTestFunc func, void *arg )
{
    return (TSearchRec *)TSortedCollection::firstThat( ccTestFunc(func), arg );
}

inline TSearchRec *TFileCollection::lastThat( ccTestFunc func, void *arg )
{
    return (TSearchRec *)TSortedCollection::lastThat( ccTestFunc(func), arg );
}

#endif  // Uses_TFileCollection


#if defined( Uses_TSortedListBox ) && !defined( __TSortedListBox )
#define __TSortedListBox

class TRect;
class TScrollBar;
class TEvent;
class TSortedCollection;

class TSortedListBox: public TListBox
{

public:

    TSortedListBox( const TRect& bounds,
                    ushort aNumCols,
                    TScrollBar *aScrollBar
                  );

    virtual void handleEvent( TEvent& event );
    virtual void newList( TSortedCollection *aList );

    TSortedCollection *list();

protected:

    uchar shiftState;

private:

    virtual void *getKey( const char *s );

    int searchPos;

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TSortedListBox( StreamableInit ) : TListBox ( streamableInit ) {}

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TSortedListBox& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TSortedListBox*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TSortedListBox& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TSortedListBox* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

inline TSortedCollection *TSortedListBox::list()
{
    return (TSortedCollection *)TListBox::list();
}

#endif  // Uses_TSortedListBox

#if defined( Uses_TFileList ) && !defined( __TFileList )
#define __TFileList

class TRect;
class TScrollBar;
class TEvent;

class TFileList : public TSortedListBox
{

public:

    TFileList( const TRect& bounds,
               TScrollBar *aScrollBar
             );
    ~TFileList();

    virtual void focusItem( int item );
    virtual void getText( char *dest, int item, size_t destsize );
    virtual void handleEvent( TEvent& event );
    virtual void newList( TFileCollection *aList );
    void readDirectory( const char *dir, const char *wildCard );
    void readDirectory( const char *wildCard );

    virtual size_t dataSize();
    virtual void getData( void *rec, size_t recsize );
    virtual void setData( void *rec );

    TFileCollection *list();

private:

    virtual void *getKey( const char *s );

    static const char *tooManyFiles;

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TFileList( StreamableInit ) : TSortedListBox ( streamableInit ) {}

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TFileList& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TFileList*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TFileList& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TFileList* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

inline void TFileList::newList( TFileCollection *f )
{
    TSortedListBox::newList( f );
}

inline TFileCollection *TFileList::list()
{
    return (TFileCollection *)TSortedListBox::list();
}

#endif  // Uses_TFileList


#if defined( Uses_TFileInfoPane ) && !defined( __TFileInfoPane )
#define __TFileInfoPane

class TRect;
class TEvent;

class TFileInfoPane : public TView
{

public:

    TFileInfoPane( const TRect& bounds );

    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );

private:

    TSearchRec file_block;

    static const char * const months[13];
    static const char *pmText;
    static const char *amText;

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TFileInfoPane( StreamableInit ) : TView ( streamableInit ) {}

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TFileInfoPane& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TFileInfoPane*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TFileInfoPane& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TFileInfoPane* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TFileInfoPane

#if defined( Uses_TFileDialog ) && !defined( __TFileDialog )
#define __TFileDialog

const int
    fdOKButton      = 0x0001,      // Put an OK button in the dialog
    fdOpenButton    = 0x0002,      // Put an Open button in the dialog
    fdReplaceButton = 0x0004,      // Put a Replace button in the dialog
    fdClearButton   = 0x0008,      // Put a Clear button in the dialog
    fdHelpButton    = 0x0010,      // Put a Help button in the dialog
    fdNoLoadDir     = 0x0100;      // Do not load the current directory
                                   // contents into the dialog at Init.
                                   // This means you intend to change the
                                   // WildCard by using SetData or store
                                   // the dialog on a stream.

#ifdef __IDA__
#include <prodir.h>
#endif

class TEvent;
class TFileInputLine;
class TFileList;

class TFileDialog : public TDialog
{

public:

    TFileDialog( const char *aWildCard, const char *aTitle,
                 const char *inputName, ushort aOptions, uchar histId );
    ~TFileDialog();

    virtual void getData( void *rec, size_t recsize );
    void getFileName( char *buf, size_t bufsize );
    virtual void handleEvent( TEvent& event );
    virtual void setData( void *rec );
    virtual Boolean valid( ushort command );
    virtual void shutDown();

    TFileInputLine *fileName;
    TFileList *fileList;
    char wildCard[MAXPATH];
    char *directory;

private:

    void readDirectory();

    Boolean checkDirectory( const char * );

    static const char *filesText;
    static const char *openText;
    static const char *okText;
    static const char *replaceText;
    static const char *clearText;
    static const char *cancelText;
    static const char *helpText;
    static const char *invalidDriveText;
    static const char *invalidFileText;

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TFileDialog( StreamableInit ) : TDialog ( streamableInit ),
        TWindowInit( TFileDialog::initFrame ) {}
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TFileDialog& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TFileDialog*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TFileDialog& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TFileDialog* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TFileDialog


#if defined( Uses_TDirEntry ) && !defined( __TDirEntry )
#define __TDirEntry

class TDirEntry
{

public:

    TDirEntry( const char *, const char * );
    ~TDirEntry();
    char *dir() { return directory; }
    char *text() { return displayText; }

private:

    char *displayText;
    char *directory;

};

inline TDirEntry::TDirEntry( const char *txt, const char *dir ) :
    displayText( newStr( txt ) ), directory( newStr( dir ) )
{
}

inline TDirEntry::~TDirEntry()
{
    delete displayText;
    delete directory;
}

#endif  // Uses_TDirEntry

#if defined( Uses_TDirCollection ) && !defined( __TDirCollection )
#define __TDirCollection

class TDirEntry;

class TDirCollection : public TCollection
{

public:

    TDirCollection( ccIndex aLimit, ccIndex aDelta) :
        TCollection( aLimit, aDelta ) {}

    TDirEntry *at( ccIndex index )
        { return (TDirEntry *)TCollection::at( index );}
    virtual ccIndex indexOf( TDirEntry *item )
        { return TCollection::indexOf( item ); }

    void remove( TDirEntry *item )
        { TCollection::remove( item ); }
    void free( TDirEntry *item )
        { TCollection::free( item ); }
    void atInsert( ccIndex index, TDirEntry *item )
        { TCollection::atInsert( index, item ); }
    void atPut( ccIndex index, TDirEntry *item )
        { TCollection::atPut( index, item ); }
    virtual ccIndex insert( TDirEntry *item )
        { return TCollection::insert( item ); }

    TDirEntry *firstThat( ccTestFunc Test, void *arg );
    TDirEntry *lastThat( ccTestFunc Test, void *arg );

private:

    virtual void freeItem( void *item )
        { delete (TDirEntry *)item; }

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }
    virtual void *readItem( ipstream& );
    virtual void writeItem( void *, opstream& );

protected:

    TDirCollection( StreamableInit ) : TCollection ( streamableInit ) {}

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TDirCollection& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDirCollection*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDirCollection& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDirCollection* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

inline TDirEntry *TDirCollection::firstThat( ccTestFunc func, void *arg )
{
    return (TDirEntry *)TCollection::firstThat( ccTestFunc(func), arg );
}

inline TDirEntry *TDirCollection::lastThat( ccTestFunc func, void *arg )
{
    return (TDirEntry *)TCollection::lastThat( ccTestFunc(func), arg );
}

#endif  // Uses_TDirCollection


#if defined( Uses_TDirListBox ) && !defined( __TDirListBox )
#define __TDirListBox

#ifdef __IDA__
#include <prodir.h>
#endif

class TRect;
class TScrollBar;
class TEvent;
class TDirCollection;

class TDirListBox : public TListBox
{

public:

    TDirListBox( const TRect& bounds, TScrollBar *aScrollBar );
    ~TDirListBox();

    virtual void getText( char *dest, int item, size_t destsize );
    virtual void handleEvent( TEvent& );
    virtual Boolean isSelected( int );
    /**
     * Deletes the existing @ref TDirEntry object associated with this list
     * box and replaces it with the file collection given by `aList'.
     *
     * The first item of the new collection will receive the focus.
     */
    void newDirectory( const char *aList );
    /**
     * By default, calls the ancestral @ref TListBox::setState().
     */
    virtual void setState( ushort aState, Boolean enable );
    /**
     * Returns a pointer to the @ref TDirCollection object currently
     * associated with this directory list box.
     */
    TDirCollection *list();
    /**
     * Undocumented.
     */
    static const char * pathDir;
    /**
     * Undocumented.
     */
    static const char * firstDir;
    /**
     * Undocumented.
     */
    static const char * middleDir;
    /**
     * Undocumented.
     */
    static const char * lastDir;
    /**
     * Undocumented.
     */
    static const char *graphics;
private:
    void showDrives( TDirCollection * );
    void showDirs( TDirCollection * );
    char dir[MAXPATH];
    ushort cur;
    static const char * drives;
#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TDirListBox( StreamableInit ): TListBox( streamableInit ) {}

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TDirListBox& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDirListBox*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDirListBox& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDirListBox* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

inline TDirCollection *TDirListBox::list()
{
    return (TDirCollection *)TListBox::list();
}

#endif  // Uses_TDirListBox

#if defined( Uses_TChDirDialog ) && !defined( __TChDirDialog )
#define __TChDirDialog

const int
    cdNormal     = 0x0000, // Option to use dialog immediately
    cdNoLoadDir  = 0x0001, // Option to init the dialog to store on a stream
    cdHelpButton = 0x0002, // Put a help button in the dialog
    cdCancelButton = 0x0004; // Replace Help button with Cancel button

class TEvent;
class TInputLine;
class TDirListBox;
class TButton;

class TChDirDialog : public TDialog
{

public:

    friend class TDirListBox;

    TChDirDialog( ushort aOptions, ushort histId, const char *title=NULL, const char *_inidir=NULL);
    virtual size_t dataSize();
    virtual void getData( void *rec, size_t recsize );
    virtual void handleEvent( TEvent& );
    virtual void setData( void *rec );
    virtual Boolean valid( ushort );
    virtual void shutDown();
    void getDirName(char *buf, size_t bufsize);

private:

    void setUpDialog();

    TInputLine *dirInput;
    TDirListBox *dirList;
    TButton *okButton;
    TButton *chDirButton;
    const char *inidir;

    static const char *changeDirTitle;
    static const char *dirNameText;
    static const char *dirTreeText;
    static const char *okText;
    static const char *chdirText;
    static const char *revertText;
    static const char *helpText;
    static const char *drivesText;
    static const char *invalidText;

#ifndef NO_TV_STREAMS
    virtual const char *streamableName() const
        { return name; }

protected:

    TChDirDialog( StreamableInit ) : TDialog( streamableInit ),
        TWindowInit( TChDirDialog::initFrame ) {}
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif  // ifndef NO_TV_STREAMS

};

#ifndef NO_TV_STREAMS
inline ipstream& operator >> ( ipstream& is, TChDirDialog& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TChDirDialog*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TChDirDialog& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TChDirDialog* cl )
    { return os << (TStreamable *)cl; }
#endif  // ifndef NO_TV_STREAMS

#endif  // Uses_TChDirDialog

