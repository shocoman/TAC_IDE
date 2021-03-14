
//--------------------------------------------------------------
// headers
//--------------------------------------------------------------

//! wxWidgets headers
#include "wx/string.h"

//! application headers
#include "scintilla_definitions.h" // definitions

//--------------------------------------------------------------
// implementation
//--------------------------------------------------------------

//! language types
const CommonInfo g_CommonPrefs = {
    // editor functionality prefs
    true,   // syntaxEnable
    true,   // foldEnable
    true,   // indentEnable
    // display defaults prefs
    false,   // overTypeInitial
    false,   // readOnlyInitial
    false,   // wrapModeInitial
    false,   // displayEOLEnable
    true,    // IndentGuideEnable
    false,   // lineNumberEnable
    false,   // longLineOnEnable
    false,   // whiteSpaceEnable
};

//----------------------------------------------------------------------------
// keywordlists
// ThreeAC
const char *ThreeACWordlist1 = "goto ifTrue ifFalse param call return halt uminus";

// Keywords for container-based tac language lexer
const std::vector<wxString> tacKeywords = {wxT("goto"), wxT("ifTrue"), wxT("ifFalse"), wxT("param"),
                                           wxT("call"), wxT("return"), wxT("halt"),    wxT("uminus")};

/*
     {{mySTC_TYPE_DEFAULT, NULL},
      {mySTC_TYPE_COMMENT, NULL},
      {mySTC_TYPE_NUMBER, NULL},
      {mySTC_TYPE_WORD1, ThreeACWordlist1}, // KEYWORDS
      {mySTC_TYPE_STRING, NULL},
      {mySTC_TYPE_CHARACTER, NULL},
      {mySTC_TYPE_IDENTIFIER, NULL},
      {mySTC_TYPE_STRING_EOL, NULL},
      {mySTC_TYPE_LABEL, NULL},
      {mySTC_TYPE_ERROR, NULL}, // KEYWORDS ERROR
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL}},
*/
//! languages
const LanguageInfo g_LanguagePrefs[] = {
    // ThreeAC
    {"ThreeAC",
     "*.3ac",
     wxSTC_LEX_CONTAINER,
     {{mySTC_TYPE_DEFAULT, NULL},
      {mySTC_TYPE_WORD1, ThreeACWordlist1},   // KEYWORDS
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {mySTC_TYPE_COMMENT, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {mySTC_TYPE_CHARACTER, NULL},
      {mySTC_TYPE_CHARACTER_EOL, NULL},
      {mySTC_TYPE_STRING, NULL},
      {mySTC_TYPE_STRING_EOL, NULL},
      {-1, NULL},
      {-1, NULL},
      {mySTC_TYPE_OPERATOR, NULL},
      {-1, NULL},
      {-1, NULL},
      {mySTC_TYPE_IDENTIFIER, NULL},
      {mySTC_TYPE_LABEL, NULL},
      {mySTC_TYPE_NUMBER, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {mySTC_TYPE_ERROR, NULL},   // KEYWORDS ERROR
      {-1, NULL},
      {-1, NULL}},
     mySTC_FOLD_COMMENT | mySTC_FOLD_BB},
    // * (any)
    {wxTRANSLATE(DEFAULT_LANGUAGE),
     "*.*",
     wxSTC_LEX_PROPERTIES,
     {{mySTC_TYPE_DEFAULT, NULL},
      {mySTC_TYPE_DEFAULT, NULL},
      {mySTC_TYPE_DEFAULT, NULL},
      {mySTC_TYPE_DEFAULT, NULL},
      {mySTC_TYPE_DEFAULT, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL}},
     0},
};

const int g_LanguagePrefsSize = WXSIZEOF(g_LanguagePrefs);

//----------------------------------------------------------------------------
//! style types
const StyleInfo g_StylePrefs[] = {
    // mySTC_TYPE_DEFAULT
    {wxT("Default"), wxT("ORANGE"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_WORD1
    {wxT("Keyword1"), wxT("BLUE"), wxT("WHITE"), wxT(""), 10, mySTC_STYLE_BOLD, 0},

    // mySTC_TYPE_WORD2
    {wxT("Keyword2"), wxT("MIDNIGHT BLUE"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_WORD3
    {wxT("Keyword3"), wxT("CORNFLOWER BLUE"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_WORD4
    {wxT("Keyword4"), wxT("CYAN"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_WORD5
    {wxT("Keyword5"), wxT("DARK GREY"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_WORD6
    {wxT("Keyword6"), wxT("GREY"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_COMMENT
    {wxT("Comment"), wxT("FOREST GREEN"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_COMMENT_DOC
    {wxT("Comment (Doc)"), wxT("FOREST GREEN"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_COMMENT_LINE
    {wxT("Comment line"), wxT("FOREST GREEN"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_COMMENT_SPECIAL
    {wxT("Special comment"), wxT("FOREST GREEN"), wxT("WHITE"), wxT(""), 10, mySTC_STYLE_ITALIC, 0},

    // mySTC_TYPE_CHARACTER
    {wxT("Character"), wxT("KHAKI"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_CHARACTER_EOL
    {wxT("Character (EOL)"), wxT("KHAKI"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_STRING
    {wxT("String"), wxT("BROWN"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_STRING_EOL
    {wxT("String (EOL)"), wxT("BROWN"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_DELIMITER
    {wxT("Delimiter"), wxT("ORANGE"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_PUNCTUATION
    {wxT("Punctuation"), wxT("ORANGE"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_OPERATOR
    {wxT("Operator"), wxT("BLACK"), wxT("WHITE"), wxT(""), 10, mySTC_STYLE_BOLD, 0},

    // mySTC_TYPE_BRACE
    {wxT("Label"), wxT("VIOLET"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_COMMAND
    {wxT("Command"), wxT("BLUE"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_IDENTIFIER
    {wxT("Identifier"), wxT("BLACK"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_LABEL
    {wxT("Label"), wxT("VIOLET"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_NUMBER
    {wxT("Number"), wxT("SIENNA"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_PARAMETER
    {wxT("Parameter"), wxT("VIOLET"), wxT("WHITE"), wxT(""), 10, mySTC_STYLE_ITALIC, 0},

    // mySTC_TYPE_REGEX
    {wxT("Regular expression"), wxT("ORCHID"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_UUID
    {wxT("UUID"), wxT("ORCHID"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_VALUE
    {wxT("Value"), wxT("ORCHID"), wxT("WHITE"), wxT(""), 10, mySTC_STYLE_ITALIC, 0},

    // mySTC_TYPE_PREPROCESSOR
    {wxT("Preprocessor"), wxT("GREY"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_SCRIPT
    {wxT("Script"), wxT("DARK GREY"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_ERROR
    {wxT("Error"), wxT("RED"), wxT("WHITE"), wxT(""), 10, 0, 0},

    // mySTC_TYPE_UNDEFINED
    {wxT("Undefined"), wxT("ORANGE"), wxT("WHITE"), wxT(""), 10, 0, 0}

};

const int g_StylePrefsSize = WXSIZEOF(g_StylePrefs);
