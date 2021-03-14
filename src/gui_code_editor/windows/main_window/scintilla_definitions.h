
#ifndef DEFINITIONS_H_INCLUDED
#define DEFINITIONS_H_INCLUDED

//! headers
#include <vector>
#include <string>


//! wxWidgets/contrib headers
#include "wx/stc/stc.h"  // styled text control

//! application headers

//--------------------------------------------------------------
// standard IDs
//--------------------------------------------------------------
enum
{
    // menu IDs
    idMenuQuit = 1000,
    idMenuAbout,
    myID_PROPERTIES = wxID_HIGHEST,
    myID_EDIT_FIRST,
    myID_FINDNEXT,
    myID_REPLACE,
    myID_REPLACENEXT,
    myID_GOTO,
    myID_PAGEACTIVE,
    myID_DISPLAYEOL,
    myID_INDENTGUIDE,
    myID_LINENUMBER,
    myID_LONGLINEON,
    myID_WHITESPACE,
    myID_OVERTYPE,
    myID_READONLY,
    myID_WRAPMODEON,
    myID_PAGEPREV,
    myID_PAGENEXT,
    myID_BB,
    myID_SELECTLINE,
    myID_EDIT_LAST = myID_SELECTLINE,
    myID_TOOLBAR_TOGGLE,
    myID_X86,
    myID_ARM,
    myID_MIPS32,
    myID_TOY_DIALECT,
    myID_LLVMIR_DIALECT,
    myID_SYMBOL_TABLE,
    myID_TARGET_PLATFORM,
    myID_TAC_DIALECT,
    myID_SIMULATOR_RUN,
    myID_OPTIMIZATION_WINDOW,
    myID_PRINT_CFG_WINDOW,
    myID_EDU_TOGGLE,
    myID_EDU_HOME,
    myID_EDU_PREV,
    myID_EDU_NEXT,
    myID_EDU_RESET,
    myID_OPTIONS,
    myID_NOTEBOOK,
    myID_SPLITTER_MAIN,
    myID_SPLIT_CODE,
    myID_EDITOR,
    myID_ASM_WINDOW,
    myID_LOG,
    // other IDs
    myID_STATUSBAR,

    // dialog find IDs
    myID_DLG_FIND_TEXT,
};

//--------------------------------------------------------------
// declarations
//--------------------------------------------------------------

#define DEFAULT_LANGUAGE "<default>"

#define PAGE_COMMON _("Common")
#define PAGE_LANGUAGES _("Languages")
#define PAGE_STYLE_TYPES _("Style types")

#define STYLE_TYPES_COUNT 32

//! general style types
#define mySTC_TYPE_DEFAULT 0

#define mySTC_TYPE_WORD1 1
#define mySTC_TYPE_WORD2 2
#define mySTC_TYPE_WORD3 3
#define mySTC_TYPE_WORD4 4
#define mySTC_TYPE_WORD5 5
#define mySTC_TYPE_WORD6 6

#define mySTC_TYPE_COMMENT 7
#define mySTC_TYPE_COMMENT_DOC 8
#define mySTC_TYPE_COMMENT_LINE 9
#define mySTC_TYPE_COMMENT_SPECIAL 10

#define mySTC_TYPE_CHARACTER 11
#define mySTC_TYPE_CHARACTER_EOL 12
#define mySTC_TYPE_STRING 13
#define mySTC_TYPE_STRING_EOL 14

#define mySTC_TYPE_DELIMITER 15

#define mySTC_TYPE_PUNCTUATION 16

#define mySTC_TYPE_OPERATOR 17

#define mySTC_TYPE_BRACE 18

#define mySTC_TYPE_COMMAND 19
#define mySTC_TYPE_IDENTIFIER 20
#define mySTC_TYPE_LABEL 21
#define mySTC_TYPE_NUMBER 22
#define mySTC_TYPE_PARAMETER 23
#define mySTC_TYPE_REGEX 24
#define mySTC_TYPE_UUID 25
#define mySTC_TYPE_VALUE 26

#define mySTC_TYPE_PREPROCESSOR 27
#define mySTC_TYPE_SCRIPT 28

#define mySTC_TYPE_ERROR 29

//----------------------------------------------------------------------------
//! style bits types
#define mySTC_STYLE_BOLD 1
#define mySTC_STYLE_ITALIC 2
#define mySTC_STYLE_UNDERL 4
#define mySTC_STYLE_HIDDEN 8

//----------------------------------------------------------------------------
//! general folding types
#define mySTC_FOLD_COMMENT 1
#define mySTC_FOLD_BB 2

//----------------------------------------------------------------------------
//! flags
#define mySTC_FLAG_WRAPMODE 16

//----------------------------------------------------------------------------
// CommonInfo

struct CommonInfo {
    // editor functionality prefs
    bool syntaxEnable;
    bool foldEnable;
    bool indentEnable;
    // display defaults prefs
    bool readOnlyInitial;
    bool overTypeInitial;
    bool wrapModeInitial;
    bool displayEOLEnable;
    bool indentGuideEnable;
    bool lineNumberEnable;
    bool longLineOnEnable;
    bool whiteSpaceEnable;
};
extern const CommonInfo g_CommonPrefs;

//----------------------------------------------------------------------------
// LanguageInfo

struct LanguageInfo {
    const char *name;
    const char *filepattern;
    int lexer;
    struct {
        int type;
        const char *words;
    } styles [STYLE_TYPES_COUNT];
    int folds;
};

extern const LanguageInfo g_LanguagePrefs[];
extern const int g_LanguagePrefsSize;


//----------------------------------------------------------------------------
// Keywords for container-based tac language lexer
extern const std::vector<wxString> tacKeywords;

//----------------------------------------------------------------------------
// StyleInfo
struct StyleInfo {
    const wxChar *name;
    const wxChar *foreground;
    const wxChar *background;
    const wxChar *fontname;
    int fontsize;
    int fontstyle;
    int lettercase;
};

extern const StyleInfo g_StylePrefs[];
extern const int g_StylePrefsSize;

#endif // DEFINITIONS_H_INCLUDED
