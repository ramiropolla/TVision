#ifndef __cm_codes_h__
#define __cm_codes_h__


enum cmCodes_lowTv {
/* views.h */
//  Standard command codes
    cmValid = 0,
    cmQuit,                     // 1
    cmError,                    // 2
    cmMenu,                     // 3
    cmClose,                    // 4
    cmZoom,                     // 5
    cmResize,                   // 6
    cmNext,                     // 7
    cmPrev,                     // 8
    cmHelp,                     // 9
//  TDialog standard commands
    cmOK,                       // 10
    cmCancel,                   // 11
    cmYes,                      // 12
    cmNo,                       // 13
    cmDefault,                  // 14
//  Application command codes
    cmCut,                      // 15 (was 20)
    cmCopy,                     // 16 (was 21)
    cmPaste,                    // 17 (was 22)
    cmUndo,                     // 18 (was 23)
    cmClear,                    // 19 (was 24)
    cmTile,                     // 20 (was 25)
    cmCascade,                  // 21 (was 26)
// SS: some new internal commands.
    cmSysRepaint,               // 22 (was 38)
    cmSysResize,                // 23 (was 39)
    cmSysWakeup,                // 24 (was 40)
// Standard messages
    cmReceivedFocus,            // 25 (was 50)
    cmReleasedFocus,            // 26 (was 51)
    cmCommandSetChanged,        // 27 (was 52)
// TScrollBar messages
    cmScrollBarChanged,         // 28 (was 53)
    cmScrollBarClicked,         // 29 (was 54)
// TWindow select messages
    cmSelectWindowNum,          // 30 (was 55)
//  TListViewer messages
    cmListItemSelected,         // 31 (was 56)
/* dialogs.h */
    cmRecordHistory,            // 32 (was 60)
/* views.h */
    cmGrabDefault,              // 33 (was 62)
    cmReleaseDefault,           // 34 (was 63)
/* colorsel.h */
    cmColorForegroundChanged,   // 35 (was 71)
    cmColorBackgroundChanged,   // 36 (was 72)
    cmColorSet,                 // 37 (was 73)
    cmNewColorItem,             // 38 (was 74)
    cmNewColorIndex,            // 39 (was 75)
/* editors.h */
    cmSave,                     // 40 (was 80)
    cmSaveAs,                   // 41 (was 81)
    cmFind,                     // 42 (was 82)
    cmReplace,                  // 43 (was 83)
    cmSearchAgain,              // 44 (was 84)
/* stddlg.h */
//  Messages
    cmFileFocused,              // 45 (was 102) // A new file was focused in the TFileList
    cmFileDoubleClicked,        // 46 (was 103) // A file was selected in the TFileList
//---
// enum's can't be prompted
    cmLow_lastTv
};

enum cmCodes_hiTv {
/* editors.h */
    cmHigh_FirstTv = 500,		
//---
    cmCharLeft = cmHigh_FirstTv,
    cmCharRight,      // 501
    cmWordLeft,       // 502
    cmWordRight,      // 503
    cmLineStart,      // 504
    cmLineEnd,        // 505
    cmLineUp,         // 506
    cmLineDown,       // 507
    cmPageUp,         // 508
    cmPageDown,       // 509
    cmTextStart,      // 510
    cmTextEnd,        // 511
    cmNewLine,        // 512
    cmBackSpace,      // 513
    cmDelChar,        // 514
    cmDelWord,        // 515
    cmDelStart,       // 516
    cmDelEnd,         // 517
    cmDelLine,        // 518
    cmInsMode,        // 519
    cmStartSelect,    // 520
    cmHideSelect,     // 521
    cmIndentMode,     // 522
    cmUpdateTitle,    // 523
/* stddlg.h */
//  Commands
    cmFileOpen,       // 504 (was 1001) // Returned from TFileDialog when Open pressed
    cmFileReplace,    // 505 (was 1002) // Returned from TFileDialog when Replace pressed
    cmFileClear,      // 506 (was 1003) // Returned from TFileDialog when Clear pressed
    cmFileInit,       // 507 (was 1004) // Used by TFileDialog internally
    cmChangeDir,      // 508 (was 1005) // Used by TChDirDialog internally
    cmRevert,         // 509 (was 1006) // Used by TChDirDialog internally
//---
    cmHigh_LastTv,
};

#endif  // __cm_codes_h__

