#ifndef DEFINITIONS_H_INCLUDED
#define DEFINITIONS_H_INCLUDED

#include <string>
#include <vector>
#include <wx/stc/stc.h>

enum {
    // menu IDs
    idMenuQuit = 1000,
    myID_PROPERTIES = wxID_HIGHEST,
    myID_EDIT_FIRST,
    myID_FINDNEXT,
    myID_REPLACE,
    myID_REPLACENEXT,
    myID_GOTO,
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
    myID_EDITOR,
};

#define DEFAULT_LANGUAGE "ThreeAC"

enum GeneralStyleTypes {
    mySTC_TYPE_DEFAULT = 0,
    mySTC_TYPE_WORD1 = 1,
    mySTC_TYPE_WORD2 = 2,
    mySTC_TYPE_WORD3 = 3,
    mySTC_TYPE_WORD4 = 4,
    mySTC_TYPE_WORD5 = 5,
    mySTC_TYPE_WORD6 = 6,
    mySTC_TYPE_COMMENT = 7,
    mySTC_TYPE_COMMENT_DOC = 8,
    mySTC_TYPE_COMMENT_LINE = 9,
    mySTC_TYPE_COMMENT_SPECIAL = 10,
    mySTC_TYPE_CHARACTER = 11,
    mySTC_TYPE_CHARACTER_EOL = 12,
    mySTC_TYPE_STRING = 13,
    mySTC_TYPE_STRING_EOL = 14,
    mySTC_TYPE_DELIMITER = 15,
    mySTC_TYPE_PUNCTUATION = 16,
    mySTC_TYPE_OPERATOR = 17,
    mySTC_TYPE_BRACE = 18,
    mySTC_TYPE_COMMAND = 19,
    mySTC_TYPE_IDENTIFIER = 20,
    mySTC_TYPE_LABEL = 21,
    mySTC_TYPE_NUMBER = 22,
    mySTC_TYPE_PARAMETER = 23,
    mySTC_TYPE_REGEX = 24,
    mySTC_TYPE_UUID = 25,
    mySTC_TYPE_VALUE = 26,
    mySTC_TYPE_PREPROCESSOR = 27,
    mySTC_TYPE_SCRIPT = 28,
    mySTC_TYPE_ERROR = 29,
    mySTC_TYPE_UNDEFINED = 30,
    STYLE_TYPES_COUNT = 32,
};

//----------------------------------------------------------------------------
//! style bits types
enum StyleBitsTypes {
    mySTC_STYLE_BOLD = 1,
    mySTC_STYLE_ITALIC = 2,
    mySTC_STYLE_UNDERLINE = 4,
    mySTC_STYLE_HIDDEN = 8,
};

//----------------------------------------------------------------------------
//! general folding types
enum FoldingType { mySTC_FOLD_COMMENT = 1, mySTC_FOLD_BB = 2 };

//----------------------------------------------------------------------------

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

const CommonInfo g_CommonPrefs = {.syntaxEnable = true,
                                  .foldEnable = true,
                                  .indentEnable = true,
                                  .readOnlyInitial = false,
                                  .overTypeInitial = false,
                                  .wrapModeInitial = false,
                                  .displayEOLEnable = false,
                                  .indentGuideEnable = true,
                                  .lineNumberEnable = false,
                                  .longLineOnEnable = false,
                                  .whiteSpaceEnable = false};

//----------------------------------------------------------------------------

struct LanguageInfo {
    const char *name;
    const char *filepattern;
    int lexer;
    struct {
        int type;
        const char *words;
    } styles[STYLE_TYPES_COUNT];
    int folds;
};

static const LanguageInfo g_language_preferences[] = {
    // Language: ThreeAC
    {.name = "ThreeAC",
     .filepattern = "*.3ac",
     .lexer = wxSTC_LEX_CONTAINER,
     .styles = {{mySTC_TYPE_DEFAULT, nullptr},         {mySTC_TYPE_WORD1, nullptr}, // KEYWORDS
                {mySTC_TYPE_WORD2, nullptr},           {mySTC_TYPE_WORD3, nullptr},
                {mySTC_TYPE_WORD4, nullptr},           {mySTC_TYPE_WORD5, nullptr},
                {mySTC_TYPE_WORD6, nullptr},           {mySTC_TYPE_COMMENT, nullptr},
                {mySTC_TYPE_COMMENT_DOC, nullptr},     {mySTC_TYPE_COMMENT_LINE, nullptr},
                {mySTC_TYPE_COMMENT_SPECIAL, nullptr}, {mySTC_TYPE_CHARACTER, nullptr},
                {mySTC_TYPE_CHARACTER_EOL, nullptr},   {mySTC_TYPE_STRING, nullptr},
                {mySTC_TYPE_STRING_EOL, nullptr},      {mySTC_TYPE_DELIMITER, nullptr},
                {mySTC_TYPE_PUNCTUATION, nullptr},     {mySTC_TYPE_OPERATOR, nullptr},
                {mySTC_TYPE_BRACE, nullptr},           {mySTC_TYPE_COMMAND, nullptr},
                {mySTC_TYPE_IDENTIFIER, nullptr},      {mySTC_TYPE_LABEL, nullptr},
                {mySTC_TYPE_NUMBER, nullptr},          {mySTC_TYPE_PARAMETER, nullptr},
                {mySTC_TYPE_REGEX, nullptr},           {mySTC_TYPE_UUID, nullptr},
                {mySTC_TYPE_VALUE, nullptr},           {mySTC_TYPE_PREPROCESSOR, nullptr},
                {mySTC_TYPE_SCRIPT, nullptr},          {mySTC_TYPE_ERROR, nullptr}, // KEYWORDS ERROR
                {mySTC_TYPE_UNDEFINED, nullptr},       {-1, nullptr}},
     .folds = mySTC_FOLD_COMMENT | mySTC_FOLD_BB}};

static const std::vector<wxString> g_tac_keywords = {wxT("goto"),  wxT("ifTrue"), wxT("ifFalse"),
                                                     wxT("param"), wxT("call"),   wxT("return"),
                                                     wxT("halt"),  wxT("if"),     wxT("uminus")};

//----------------------------------------------------------------------------
// StyleInfo
struct StyleInfo {
    const wxChar *name, *foreground, *background, *fontname;
    int fontsize, fontstyle, lettercase;
};

const StyleInfo g_style_preferences[] = {
    {wxT("Default"), wxT("BLACK"), wxT("WHITE"), wxT(""), 10, 0, 0},                     // mySTC_TYPE_DEFAULT
    {wxT("Keyword1"), wxT("BLUE"), wxT("WHITE"), wxT(""), 10, mySTC_STYLE_BOLD, 0},      // mySTC_TYPE_WORD1
    {wxT("Keyword2"), wxT("MIDNIGHT BLUE"), wxT("WHITE"), wxT(""), 10, 0, 0},            // mySTC_TYPE_WORD2
    {wxT("Keyword3"), wxT("CORNFLOWER BLUE"), wxT("WHITE"), wxT(""), 10, 0, 0},          // mySTC_TYPE_WORD3
    {wxT("Keyword4"), wxT("CYAN"), wxT("WHITE"), wxT(""), 10, 0, 0},                     // mySTC_TYPE_WORD4
    {wxT("Keyword5"), wxT("DARK GREY"), wxT("WHITE"), wxT(""), 10, 0, 0},                // mySTC_TYPE_WORD5
    {wxT("Keyword6"), wxT("GREY"), wxT("WHITE"), wxT(""), 10, 0, 0},                     // mySTC_TYPE_WORD6
    {wxT("Comment"), wxT("FOREST GREEN"), wxT("WHITE"), wxT(""), 10, 0, 0},              // mySTC_TYPE_COMMENT
    {wxT("Comment (Doc)"), wxT("FOREST GREEN"), wxT("WHITE"), wxT(""), 10, 0, 0},        // mySTC_TYPE_COMMENT_DOC
    {wxT("Comment line"), wxT("FOREST GREEN"), wxT("WHITE"), wxT(""), 10, 0, 0},         // mySTC_TYPE_COMMENT_LINE
    {wxT("Special comment"), wxT("GREEN"), wxT("WHITE"), wxT(""), 10, 0, 0},             // mySTC_TYPE_COMMENT_SPECIAL
    {wxT("Character"), wxT("KHAKI"), wxT("WHITE"), wxT(""), 10, 0, 0},                   // mySTC_TYPE_CHARACTER
    {wxT("Character (EOL)"), wxT("KHAKI"), wxT("WHITE"), wxT(""), 10, 0, 0},             // mySTC_TYPE_CHARACTER_EOL
    {wxT("String"), wxT("BROWN"), wxT("WHITE"), wxT(""), 10, 0, 0},                      // mySTC_TYPE_STRING
    {wxT("String (EOL)"), wxT("BROWN"), wxT("WHITE"), wxT(""), 10, 0, 0},                // mySTC_TYPE_STRING_EOL
    {wxT("Delimiter"), wxT("ORANGE"), wxT("WHITE"), wxT(""), 10, 0, 0},                  // mySTC_TYPE_DELIMITER
    {wxT("Punctuation"), wxT("ORANGE"), wxT("WHITE"), wxT(""), 10, 0, 0},                // mySTC_TYPE_PUNCTUATION
    {wxT("Operator"), wxT("BLACK"), wxT("WHITE"), wxT(""), 10, mySTC_STYLE_BOLD, 0},     // mySTC_TYPE_OPERATOR
    {wxT("Brace"), wxT("VIOLET"), wxT("WHITE"), wxT(""), 10, 0, 0},                      // mySTC_TYPE_BRACE
    {wxT("Command"), wxT("BLUE"), wxT("WHITE"), wxT(""), 10, 0, 0},                      // mySTC_TYPE_COMMAND
    {wxT("Identifier"), wxT("BLACK"), wxT("WHITE"), wxT(""), 10, 0, 0},                  // mySTC_TYPE_IDENTIFIER
    {wxT("Label"), wxT("VIOLET"), wxT("WHITE"), wxT(""), 10, mySTC_STYLE_BOLD, 0},       // mySTC_TYPE_LABEL
    {wxT("Number"), wxT("SIENNA"), wxT("WHITE"), wxT(""), 10, 0, 0},                     // mySTC_TYPE_NUMBER
    {wxT("Parameter"), wxT("VIOLET"), wxT("WHITE"), wxT(""), 10, mySTC_STYLE_ITALIC, 0}, // mySTC_TYPE_PARAMETER
    {wxT("Regular expression"), wxT("ORCHID"), wxT("WHITE"), wxT(""), 10, 0, 0},         // mySTC_TYPE_REGEX
    {wxT("UUID"), wxT("ORCHID"), wxT("WHITE"), wxT(""), 10, 0, 0},                       // mySTC_TYPE_UUID
    {wxT("Value"), wxT("ORCHID"), wxT("WHITE"), wxT(""), 10, mySTC_STYLE_ITALIC, 0},     // mySTC_TYPE_VALUE
    {wxT("Preprocessor"), wxT("GREY"), wxT("WHITE"), wxT(""), 10, 0, 0},                 // mySTC_TYPE_PREPROCESSOR
    {wxT("Script"), wxT("DARK GREY"), wxT("WHITE"), wxT(""), 10, 0, 0},                  // mySTC_TYPE_SCRIPT
    {wxT("Error"), wxT("RED"), wxT("WHITE"), wxT(""), 10, 0, 0},                         // mySTC_TYPE_ERROR
    {wxT("Undefined"), wxT("ORANGE"), wxT("WHITE"), wxT(""), 10, 0, 0}                   // mySTC_TYPE_UNDEFINED
};

#endif // DEFINITIONS_H_INCLUDED
