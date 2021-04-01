#include <algorithm>
#include <map>
#include <vector>

#include "wx/dialog.h"
#include "wx/file.h"
#include "wx/filename.h"
#include "wx/wx.h"
#include <wx/stc/stc.h>

#include "code_editor_control.h"

// The (uniform) style used for the annotations.
const int ANNOTATION_STYLE = wxSTC_STYLE_LASTPREDEFINED + 1;

static int g_edu_page_max = 1;

// Lesson content structure
struct EducationalLesson {
    wxString lesson_header;
    std::vector<wxString> code;
    std::map<int, wxString> text;
};

const std::vector<EducationalLesson> l = {
    {wxT("Трансляция инструкции if-else"),
     {wxT("a = 3\n"), wxT("b = 0\n"), wxT("$t0 = a < 5\n"), wxT("expression = $t0\n"), wxT("$t1 = expression\n"),
      wxT("ifFalse $t1 goto $L1\n"), wxT("b = 1\n"), wxT("goto $L2\n"), wxT("$L1: b = 2\n"), wxT("$L2: halt")},
     {{1, wxT("Схема трансляции для оператора if позволяет избежать дублирования некоторых переходов."
              "\nДля компактности мы ввели действия непосредственно в продукцию"
              "\n(на практике могут потребоваться дополнительные нетерминалы и продукции)."
              "\n\tСинтаксис:"
              "\n\tif (expression) statement1 else statement2"
              "\n\tДействия в продукциях:"
              "\n\tif(expression) {C1} statement1 {C2} else statement2 {C3}"
              "\nДействиями являются:"
              "\n\t- C1 – увеличить номер метки; образовать код для перехода к метке,"
              "\nесли expression ложно; поместить номер метки в стек."
              "\n\t- C2 – увеличить номер метки; образовать код для безусловного перехода к метке;"
              "\nизвлечь из стека метку Lk; установить метку Lk в коде; поместить в стек метку,"
              "\nприменявшуюся в безусловном переходе."
              "\n\t- C3 – извлечь из стека метку Lj; установить метку Lj в коде."
              "\nПри использовании данной грамматики будет создан следующий код:"
              "\nПример:")},
      {3, wxT("Код для вычисления expression")},
      {6, wxT("код для statement1")},
      {8, wxT("код для statement2")}}},
};

//--------------------------------------------------------------
// implementation
//--------------------------------------------------------------

//--------------------------------------------------------------
// EditorCtrl
//--------------------------------------------------------------

wxBEGIN_EVENT_TABLE(EditorCtrl, wxStyledTextCtrl)
    // common
    EVT_SIZE(EditorCtrl::OnSize)
    // edit
    EVT_MENU(wxID_CLEAR, EditorCtrl::OnEditClear)
    EVT_MENU(wxID_CUT, EditorCtrl::OnEditCut)
    EVT_MENU(wxID_COPY, EditorCtrl::OnEditCopy)
    EVT_MENU(wxID_PASTE, EditorCtrl::OnEditPaste)
    EVT_MENU(wxID_SELECTALL, EditorCtrl::OnEditSelectAll)
    EVT_MENU(myID_SELECTLINE, EditorCtrl::OnEditSelectLine)
    EVT_MENU(wxID_REDO, EditorCtrl::OnEditRedo)
    EVT_MENU(wxID_UNDO, EditorCtrl::OnEditUndo)
    // find
    EVT_MENU(wxID_FIND, EditorCtrl::OnFind)
    EVT_MENU(myID_FINDNEXT, EditorCtrl::OnFindNext)
    EVT_MENU(myID_REPLACE, EditorCtrl::OnReplace)
    EVT_MENU(myID_REPLACENEXT, EditorCtrl::OnReplaceNext)
    EVT_MENU(myID_GOTO, EditorCtrl::OnGoto)
    // view
    EVT_MENU(myID_DISPLAYEOL, EditorCtrl::OnDisplayEOL)
    EVT_MENU(myID_INDENTGUIDE, EditorCtrl::OnIndentGuide)
    EVT_MENU(myID_LINENUMBER, EditorCtrl::OnLineNumber)
    EVT_MENU(myID_LONGLINEON, EditorCtrl::OnLongLineOn)
    EVT_MENU(myID_WHITESPACE, EditorCtrl::OnWhiteSpace)
    EVT_MENU(myID_OVERTYPE, EditorCtrl::OnSetOverType)
    EVT_MENU(myID_READONLY, EditorCtrl::OnSetReadOnly)
    EVT_MENU(myID_WRAPMODEON, EditorCtrl::OnWrapmodeOn)
    EVT_MENU(myID_BB, EditorCtrl::OnBBToggle)
    // stc
    EVT_STC_MARGINCLICK(wxID_ANY, EditorCtrl::OnMarginClick)
    EVT_STC_CHARADDED(wxID_ANY, EditorCtrl::OnCharAdded) EVT_STC_KEY(wxID_ANY, EditorCtrl::OnKey)
    EVT_KEY_DOWN(EditorCtrl::OnKeyDown)
    EVT_STC_STYLENEEDED(wxID_ANY, EditorCtrl::OnStyleNeeded)
    // educational mode events
    EVT_MENU(myID_EDU_TOGGLE, EditorCtrl::OnEduToggle)
    EVT_MENU(myID_EDU_HOME, EditorCtrl::OnEduHome)
    EVT_MENU(myID_EDU_PREV, EditorCtrl::OnEduPrev)
    EVT_MENU(myID_EDU_NEXT, EditorCtrl::OnEduNext)
    EVT_MENU(myID_EDU_RESET, EditorCtrl::OnEduReset)
wxEND_EVENT_TABLE()

EditorCtrl::EditorCtrl(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
    : wxStyledTextCtrl(parent, id, pos, size, style) {
    m_filename = wxEmptyString;

    m_LineNrID = 0;
    m_DividerID = 1;
    m_FoldingID = 2;

    // initialize language
    m_selected_language = nullptr;

    // Use all the bits in the style byte as styles, not indicators.
    SetStyleBits(8);

    // default font for all styles
    SetViewEOL(g_CommonPrefs.displayEOLEnable);
    SetIndentationGuides(g_CommonPrefs.indentGuideEnable);
    SetEdgeMode(g_CommonPrefs.longLineOnEnable ? wxSTC_EDGE_LINE : wxSTC_EDGE_NONE);
    SetViewWhiteSpace(g_CommonPrefs.whiteSpaceEnable ? wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE);
    SetOvertype(g_CommonPrefs.overTypeInitial);
    SetReadOnly(g_CommonPrefs.readOnlyInitial);
    SetWrapMode(g_CommonPrefs.wrapModeInitial ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
    wxFont font(10, wxMODERN, wxNORMAL, wxNORMAL);
    StyleSetFont(wxSTC_STYLE_DEFAULT, font);
    StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
    StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(wxT("DARK GREY")));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour(wxT("DARK GREY")));
    InitializePreferences(DEFAULT_LANGUAGE);

    // set visibility
    SetVisiblePolicy(wxSTC_VISIBLE_STRICT | wxSTC_VISIBLE_SLOP, 1);
    SetXCaretPolicy(wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1);
    SetYCaretPolicy(wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1);

    // markers
    MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_DOTDOTDOT, wxT("BLACK"), wxT("BLACK"));
    MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_ARROWDOWN, wxT("BLACK"), wxT("BLACK"));
    MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY, wxT("BLACK"), wxT("BLACK"));
    MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_DOTDOTDOT, wxT("BLACK"), wxT("WHITE"));
    MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN, wxT("BLACK"), wxT("WHITE"));
    MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY, wxT("BLACK"), wxT("BLACK"));
    MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY, wxT("BLACK"), wxT("BLACK"));
    MarkerDefine((int)MarkerType::BasicBlockMark, wxSTC_MARK_CHARACTER + (int)'B', wxT("BLACK"), wxNullColour);
    MarkerDefine((int)MarkerType::BrightLineBackground, wxSTC_MARK_CHARACTER, wxT("BLACK"), wxNullColour);
    MarkerDefine((int)MarkerType::DarkLineBackground, wxSTC_MARK_CHARACTER, wxT("BLACK"), wxColour(233, 233, 238));

    // annotations
    AnnotationSetVisible(wxSTC_ANNOTATION_BOXED);

    // miscellaneous
    // width of margin for line number marker
    m_LineNrMargin = TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_999"));

    m_FoldingMargin = 16;
    CmdKeyClear(wxSTC_KEY_TAB, 0); // this is done by the menu accelerator key
    SetLayoutCache(wxSTC_CACHE_PAGE);

    InitializePreferences(wxT("ThreeAC"));

    // show base blocks mode
    m_show_basic_block_marks = true;

    // educational mode
    m_education_mode = false;
    m_page_number = 0;

    // increase default font size
    SetZoom(4);
}

EditorCtrl::~EditorCtrl() {}

//----------------------------------------------------------------------------
// common event handlers
void EditorCtrl::OnSize(wxSizeEvent &event) {
    int x = GetClientSize().x + (g_CommonPrefs.lineNumberEnable ? m_LineNrMargin : 0) +
            (g_CommonPrefs.foldEnable ? m_FoldingMargin : 0);
    if (x > 0)
        SetScrollWidth(x);
    event.Skip();
}

// edit event handlers
void EditorCtrl::OnEditRedo(wxCommandEvent &WXUNUSED(event)) {
    if (CanRedo())
        Redo();
}

void EditorCtrl::OnEditUndo(wxCommandEvent &WXUNUSED(event)) {
    if (CanUndo())
        Undo();
}

void EditorCtrl::OnEditClear(wxCommandEvent &WXUNUSED(event)) {
    if (!GetReadOnly())
        Clear();
}

void EditorCtrl::OnKey(wxStyledTextEvent &WXUNUSED(event)) { wxMessageBox("OnKey"); }

void EditorCtrl::OnKeyDown(wxKeyEvent &evt) {
    if (CallTipActive())
        CallTipCancel();
    if (evt.GetKeyCode() == WXK_SPACE && evt.ControlDown() && evt.ShiftDown()) {
        int pos = GetCurrentPos();
        CallTipSetBackground(*wxYELLOW);
        CallTipShow(pos, "This is a CallTip with multiple lines.\n"
                         "It is meant to be a context sensitive popup helper for the user.");
    } else if (evt.GetKeyCode() == WXK_TAB && !evt.ShiftDown()) {
        Tab();
    } else if (evt.GetKeyCode() == WXK_TAB && evt.ShiftDown()) {
        BackTab();
    } else {
        evt.Skip();
    }
}

void EditorCtrl::OnEditCut(wxCommandEvent &WXUNUSED(event)) {
    if (!GetReadOnly() && (GetSelectionEnd() - GetSelectionStart() > 0))
        Cut();
}

void EditorCtrl::OnEditCopy(wxCommandEvent &WXUNUSED(event)) {
    if (GetSelectionEnd() - GetSelectionStart() > 0)
        Copy();
}

void EditorCtrl::OnEditPaste(wxCommandEvent &WXUNUSED(event)) {
    if (CanPaste())
        Paste();
}

void EditorCtrl::OnFind(wxCommandEvent &WXUNUSED(event)) {}

void EditorCtrl::OnFindNext(wxCommandEvent &WXUNUSED(event)) {}

void EditorCtrl::OnReplace(wxCommandEvent &WXUNUSED(event)) {}

void EditorCtrl::OnReplaceNext(wxCommandEvent &WXUNUSED(event)) {}

void EditorCtrl::OnGoto(wxCommandEvent &WXUNUSED(event)) {}

void EditorCtrl::OnEditSelectAll(wxCommandEvent &WXUNUSED(event)) { SetSelection(0, GetTextLength()); }

void EditorCtrl::OnEditSelectLine(wxCommandEvent &WXUNUSED(event)) {
    int lineStart = PositionFromLine(GetCurrentLine());
    int lineEnd = PositionFromLine(GetCurrentLine() + 1);
    SetSelection(lineStart, lineEnd);
}

void EditorCtrl::OnDisplayEOL(wxCommandEvent &WXUNUSED(event)) { SetViewEOL(!GetViewEOL()); }

void EditorCtrl::OnIndentGuide(wxCommandEvent &WXUNUSED(event)) { SetIndentationGuides(!GetIndentationGuides()); }

void EditorCtrl::OnLineNumber(wxCommandEvent &WXUNUSED(event)) {
    SetMarginWidth(m_LineNrID, GetMarginWidth(m_LineNrID) == 0 ? m_LineNrMargin : 0);
}

void EditorCtrl::OnLongLineOn(wxCommandEvent &WXUNUSED(event)) {
    SetEdgeMode(GetEdgeMode() == 0 ? wxSTC_EDGE_LINE : wxSTC_EDGE_NONE);
}

void EditorCtrl::OnWhiteSpace(wxCommandEvent &WXUNUSED(event)) {
    SetViewWhiteSpace(GetViewWhiteSpace() == 0 ? wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE);
}

void EditorCtrl::OnSetOverType(wxCommandEvent &WXUNUSED(event)) { SetOvertype(!GetOvertype()); }

void EditorCtrl::OnSetReadOnly(wxCommandEvent &WXUNUSED(event)) { SetReadOnly(!GetReadOnly()); }

void EditorCtrl::OnWrapmodeOn(wxCommandEvent &WXUNUSED(event)) {
    SetWrapMode(GetWrapMode() == 0 ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
}

//! show base blocks
void EditorCtrl::OnBBToggle(wxCommandEvent &event) {
    m_show_basic_block_marks = !m_show_basic_block_marks;
    UpdateCodeHighlighting(0, GetTextLength());
}

//! educational mode
void EditorCtrl::OnEduToggle(wxCommandEvent &event) {
    if (!m_education_mode) {
        m_education_mode = true;
        g_edu_page_max = l.size() - 1;
        m_page_number = 0;
        ShowEduPage(m_page_number);
    } else
        m_education_mode = false;
}

void EditorCtrl::OnEduHome(wxCommandEvent &event) {
    m_page_number = 0;
    ShowEduPage(m_page_number);
}

void EditorCtrl::OnEduPrev(wxCommandEvent &event) {
    if (m_page_number > 0)
        ShowEduPage(--m_page_number);
}

void EditorCtrl::OnEduNext(wxCommandEvent &event) {
    if (m_page_number < g_edu_page_max)
        ShowEduPage(++m_page_number);
}

void EditorCtrl::OnEduReset(wxCommandEvent &event) { ShowEduPage(m_page_number); }

void EditorCtrl::ShowEduPage(int pageNr) {
    Clear();
    DiscardEdits();
    ClearAll();

    m_filename = l[pageNr].lesson_header;
    for (unsigned Nr = 0; Nr < l[pageNr].code.size(); Nr++)
        AddText(l[pageNr].code[Nr]);
    for (auto it = l[pageNr].text.begin(); it != l[pageNr].text.end(); it++)
        AnnotationAdd(it->first, it->second);

    SetSavePoint();
}

//! misc
void EditorCtrl::OnMarginClick(wxStyledTextEvent &event) {
    if (event.GetMargin() == 2) {
        int lineClick = LineFromPosition(event.GetPosition());
        int levelClick = GetFoldLevel(lineClick);
        if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0) {
            ToggleFold(lineClick);
        }
    }
}

void EditorCtrl::OnCharAdded(wxStyledTextEvent &event) {
    char chr = (char)event.GetKey();
    int currentLine = GetCurrentLine();
    // Change this if support for mac files with \r is needed
    if (chr == '\n') {
        int lineInd = 0;
        if (currentLine > 0) {
            lineInd = GetLineIndentation(currentLine - 1);
        }
        if (lineInd == 0)
            return;
        SetLineIndentation(currentLine, lineInd);
        GotoPos(PositionFromLine(currentLine) + lineInd);
    }
}

void EditorCtrl::OnStyleNeeded(wxStyledTextEvent &event) {
    int start_pos = std::max(0, GetEndStyled() - 1);
    int end_pos = event.GetPosition();
    UpdateCodeHighlighting(start_pos, end_pos);
}

void EditorCtrl::UpdateCodeHighlighting(int startPos, int endPos) {
    enum class TokenType { Other, Space, Commentary, Number, Identifier, Goto, Label, String, Char, Eof };
    auto GetIdentifier = [&](int start_pos, int length) { return GetTextRange(start_pos, start_pos + length); };
    auto ReadNextToken = [&](int pos) -> std::pair<TokenType, int> {
        auto IsValidIdentifierChar = [](auto &c, bool first) -> bool {
            return wxIsalpha(c) || c == wxT('_') || c == wxT('$') || (wxIsdigit(c) || c == wxT('.')) && not first;
        };
        if (pos == GetTextLength())
            return {TokenType::Eof, 0};

        wxChar c = GetCharAt(pos);
        if (c == wxT(' ') || c == wxT('\t') || c == wxT('\r') || c == wxT('\n')) {
            // Whitespaces
            return {TokenType::Space, 1};
        } else if (c == wxT('"') || c == wxT('\'')) {
            // String or Char
            wxChar end_quotes = c;
            int end_pos = pos + 1;
            while (end_pos < GetTextLength() && GetCharAt(end_pos) != end_quotes && GetCharAt(end_pos) != wxT('\n'))
                ++end_pos;
            if (end_pos < GetTextLength() && GetCharAt(end_pos) != wxT('\n'))
                ++end_pos;
            return {end_quotes == wxT('"') ? TokenType::String : TokenType::Char, end_pos - pos};
        } else if (pos + 1 < GetTextLength() && c == wxT('/') && GetCharAt(pos + 1) == wxT('/')) {
            // Commentary
            int end_pos = pos;
            while (GetCharAt(end_pos) != wxT('\n') && end_pos < GetTextLength())
                ++end_pos;
            if (end_pos != GetTextLength())
                ++end_pos;
            return {TokenType::Commentary, end_pos - pos};
        } else if (wxIsdigit(c)) {
            // Number
            int end_pos = pos;
            bool has_dot = false;

            do {
                if (c == wxT('.'))
                    has_dot = true;
                ++end_pos;
                c = GetCharAt(end_pos);
            } while ((wxIsdigit(c) || c == wxT('.') && not has_dot) && end_pos < GetTextLength());

            return {TokenType::Number, end_pos - pos};
        } else if (IsValidIdentifierChar(c, true)) {
            // Identifier, goto or label
            int end_pos = pos;
            do {
                if (end_pos + 1 == GetTextLength()) {
                    end_pos += 1;
                    break;
                }
                ++end_pos;
                c = GetCharAt(end_pos);
            } while (IsValidIdentifierChar(c, false));

            int len = end_pos - pos;
            if (GetIdentifier(pos, len).CmpNoCase(wxT("goto")) == 0)
                return {TokenType::Goto, len};
            else if (c == wxT(':'))
                return {TokenType::Label, len + 1};
            else
                return {TokenType::Identifier, len};
        } else {
            return {TokenType::Other, 1};
        }
    };

    StartStyling(0, 31);
    std::unordered_set<int> basic_block_leaders;

    int curr_pos = 0;
    while (curr_pos < GetTextLength()) {
        int chosen_style = mySTC_TYPE_DEFAULT;

        auto [token_type, len] = ReadNextToken(curr_pos);

        wxString output = fmt::format("{:2}; ", len);
        switch (token_type) {
        case TokenType::Other: {
            output += fmt::format("Other: '{}'\n", GetIdentifier(curr_pos, len));
            break;
        }
        case TokenType::Space: {
            output += fmt::format("Space\n");
            break;
        }
        case TokenType::Commentary: {
            output += fmt::format("Commentary\n");
            chosen_style = mySTC_TYPE_COMMENT;
            break;
        }
        case TokenType::Number: {
            output += fmt::format("Number: '{}'\n", GetIdentifier(curr_pos, len));
            chosen_style = mySTC_TYPE_NUMBER;
            break;
        }
        case TokenType::Identifier: {
            output += fmt::format("Identifier: '{}'\n", GetIdentifier(curr_pos, len));
            if (std::find(g_tac_keywords.begin(), g_tac_keywords.end(), GetIdentifier(curr_pos, len)) !=
                g_tac_keywords.end()) {
                chosen_style = mySTC_TYPE_WORD1;
            }
            break;
        }
        case TokenType::Goto: {
            output += fmt::format("Goto\n");
            chosen_style = mySTC_TYPE_WORD1;

            // mark next non-empty line
            int last_line = LineFromPosition(GetTextLength());
            int line = LineFromPosition(curr_pos);
            while (true) {
                ++line;
                if (line > last_line)
                    break;
                if (GetLineLength(line) != 0) {
                    basic_block_leaders.insert(line);
                    break;
                }
            }
            break;
        }
        case TokenType::Label: {
            int j = curr_pos;
            int length = len;
            do {
                j += length;
                std::tie(token_type, length) = ReadNextToken(j);
            } while (token_type == TokenType::Space);

            if (token_type == TokenType::Eof || GetIdentifier(j, length) != wxT('.')) {
                // label
                chosen_style = mySTC_TYPE_WORD1;
                output += fmt::format("Label: '{}'\n", GetIdentifier(curr_pos, length));
                basic_block_leaders.insert(LineFromPosition(curr_pos));
            } else {
                // var declaration
            }
            break;
        }
        case TokenType::String: {
            output += fmt::format("String: '{}'\n", GetIdentifier(curr_pos, len));
            chosen_style = mySTC_TYPE_STRING;
            break;
        }
        case TokenType::Char: {
            output += fmt::format("Char: '{}'\n", GetIdentifier(curr_pos, len));
            chosen_style = mySTC_TYPE_CHARACTER;
            break;
        }
        default:
            break;
        }
        // fmt::print("{}", output);

        curr_pos += len;
        SetStyling(len, chosen_style);
    }

    UpdateLineMarkers(basic_block_leaders);
}

//----------------------------------------------------------------------------
// private functions
void EditorCtrl::UpdateLineMarkers(const std::unordered_set<int> &basic_block_leaders) {
    MarkerDeleteAll((int)MarkerType::BasicBlockMark);
    MarkerDeleteAll((int)MarkerType::BrightLineBackground);
    MarkerDeleteAll((int)MarkerType::DarkLineBackground);

    if (m_show_basic_block_marks && not basic_block_leaders.empty()) {
        bool bbStyleSwitch = false;
        for (int n_line = 0; n_line < GetLineCount(); n_line++) {
            if (basic_block_leaders.count(n_line) > 0) {
                MarkerAdd(n_line, (int)MarkerType::BasicBlockMark);
                bbStyleSwitch = !bbStyleSwitch;
            }
            MarkerAdd(n_line,
                      bbStyleSwitch ? (int)MarkerType::BrightLineBackground : (int)MarkerType::DarkLineBackground);
        }
    }
}

void EditorCtrl::AnnotationAdd(int line, wxString ann) {
    ann = AnnotationGetText(line) + ann;
    if (ann.empty())
        return;

    AnnotationSetText(line, ann);
    AnnotationSetStyle(line, ANNOTATION_STYLE);

    // Scintilla doesn't update the scroll width for annotations, even with
    // scroll width tracking on, so do it manually.
    const int width = GetScrollWidth();

    // NB: The following adjustments are only needed when using
    //     wxSTC_ANNOTATION_BOXED annotations style, but we apply them always
    //     in order to make things simpler and not have to redo the width
    //     calculations when the annotations visibility changes. In a real
    //     program you'd either just stick to a fixed annotations visibility or
    //     update the width when it changes.

    // Take into account the fact that the annotation is shown indented, with
    // the same indent as the line it's attached to.
    int indent = GetLineIndentation(line);

    // This is just a hack to account for the width of the box, there doesn't
    // seem to be any way to get it directly from Scintilla.
    indent += 3;

    const int widthAnn = TextWidth(ANNOTATION_STYLE, ann + wxString(indent, ' '));

    if (widthAnn > width)
        SetScrollWidth(widthAnn);
}

void EditorCtrl::AnnotationRemove(int line) { AnnotationSetText(line, wxString()); }

void EditorCtrl::AnnotationClear() { AnnotationClearAll(); }

wxString EditorCtrl::DeterminePrefs(const wxString &filename) {
    const LanguageInfo *curInfo = nullptr;

    // determine language from file patterns
    int languageNr;
    for (languageNr = 0; languageNr < WXSIZEOF(g_language_preferences); languageNr++) {
        curInfo = &g_language_preferences[languageNr];
        wxString filepattern = curInfo->filepattern;
        filepattern.Lower();
        while (!filepattern.empty()) {
            wxString cur = filepattern.BeforeFirst(';');
            if ((cur == filename) || (cur == (filename.BeforeLast('.') + wxT(".*"))) ||
                (cur == (wxT("*.") + filename.AfterLast('.')))) {
                return curInfo->name;
            }
            filepattern = filepattern.AfterFirst(';');
        }
    }
    // take last by default
    if (curInfo != nullptr) {
        return curInfo->name;
    } else {
        return wxEmptyString;
    }
}

bool EditorCtrl::InitializePreferences(const wxString &name) {
    StyleClearAll();

    // determine language
    const LanguageInfo *curr_lang_info = nullptr;
    for (int n_language = 0; n_language < WXSIZEOF(g_language_preferences); ++n_language) {
        if (g_language_preferences[n_language].name == name) {
            curr_lang_info = &g_language_preferences[n_language];
            break;
        }
    }
    if (curr_lang_info == nullptr)
        return false;

    // set lexer and language
    SetLexer(curr_lang_info->lexer);
    m_selected_language = curr_lang_info;

    // set margin for line numbers
    SetMarginType(m_LineNrID, wxSTC_MARGIN_NUMBER);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(wxT("DARK GREY")));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
    SetMarginWidth(m_LineNrID, m_LineNrMargin);

    // annotations style
    StyleSetBackground(ANNOTATION_STYLE, wxColour(220, 244, 220));
    StyleSetForeground(ANNOTATION_STYLE, *wxBLACK);
    StyleSetSizeFractional(ANNOTATION_STYLE, (StyleGetSizeFractional(wxSTC_STYLE_DEFAULT) * 4) / 5);

    // default fonts for all styles!
    for (int Nr = 0; Nr < wxSTC_STYLE_LASTPREDEFINED; Nr++) {
        wxFont font(10, wxMODERN, wxNORMAL, wxNORMAL);
        StyleSetFont(Nr, font);
    }

    // set common styles
    StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour(wxT("DARK GREY")));
    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour(wxT("DARK GREY")));

    // initialize settings
    if (g_CommonPrefs.syntaxEnable) {
        int keywordnr = 0;
        for (int i = 0; i < STYLE_TYPES_COUNT; i++) {
            if (curr_lang_info->styles[i].type == -1)
                continue;
            const StyleInfo &current_style = g_style_preferences[curr_lang_info->styles[i].type];
            wxFont font(current_style.fontsize, wxMODERN, wxNORMAL, wxNORMAL, false, current_style.fontname);
            StyleSetFont(i, font);
            if (current_style.foreground)
                StyleSetForeground(i, wxColour(current_style.foreground));
            if (current_style.background)
                StyleSetBackground(i, wxColour(current_style.background));
            StyleSetBold(i, (current_style.fontstyle & mySTC_STYLE_BOLD) > 0);
            StyleSetItalic(i, (current_style.fontstyle & mySTC_STYLE_ITALIC) > 0);
            StyleSetUnderline(i, (current_style.fontstyle & mySTC_STYLE_UNDERLINE) > 0);
            StyleSetVisible(i, (current_style.fontstyle & mySTC_STYLE_HIDDEN) == 0);
            StyleSetCase(i, current_style.lettercase);
            const char *pwords = curr_lang_info->styles[i].words;
            if (pwords) {
                SetKeyWords(keywordnr, pwords);
                keywordnr += 1;
            }
        }
    }

    // set margin as unused
    SetMarginType(m_DividerID, wxSTC_MARGIN_SYMBOL);
    SetMarginWidth(m_DividerID, 0);
    SetMarginSensitive(m_DividerID, false);

    // bb highlighting
    SetMarginType(m_FoldingID, wxSTC_MARGIN_TEXT);
    SetMarginMask(m_FoldingID, wxSTC_MASK_FOLDERS | 0x3);
    StyleSetBackground(m_FoldingID, wxColour(238, 238, 238));
    SetMarginWidth(m_FoldingID, 16);
    SetMarginSensitive(m_FoldingID, false);

    //    if (g_CommonPrefs.foldEnable) {
    //        SetMarginWidth(m_FoldingID, curInfo->folds != 0 ? m_FoldingMargin : 0);
    //        SetMarginSensitive(m_FoldingID, curInfo->folds != 0);
    //        SetProperty(wxT("fold"), curInfo->folds != 0 ? wxT("1") : wxT("0"));
    //        SetProperty(wxT("fold.comment"), (curInfo->folds & mySTC_FOLD_COMMENT) > 0 ? wxT("1") : wxT("0"));
    //        SetProperty(wxT("fold.bb"), (curInfo->folds & mySTC_FOLD_BB) > 0 ? wxT("1") : wxT("0"));
    //    }
    //    SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

    // set spaces and indention
    SetTabWidth(4);
    SetUseTabs(false);
    SetTabIndents(true);
    SetBackSpaceUnIndents(true);
    SetIndent(g_CommonPrefs.indentEnable ? 4 : 0);

    // others
    SetViewEOL(g_CommonPrefs.displayEOLEnable);
    SetIndentationGuides(g_CommonPrefs.indentGuideEnable);
    SetEdgeColumn(80);
    SetEdgeMode(g_CommonPrefs.longLineOnEnable ? wxSTC_EDGE_LINE : wxSTC_EDGE_NONE);
    SetViewWhiteSpace(g_CommonPrefs.whiteSpaceEnable ? wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE);
    SetOvertype(g_CommonPrefs.overTypeInitial);
    SetReadOnly(g_CommonPrefs.readOnlyInitial);
    SetWrapMode(g_CommonPrefs.wrapModeInitial ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);


    return true;
}

bool EditorCtrl::LoadFile() {
    // get filename
    if (!m_filename) {
        wxFileDialog dlg(this, wxT("Open file"), wxEmptyString, wxEmptyString, wxT("Any file (*)|*"),
                         wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
        if (dlg.ShowModal() != wxID_OK)
            return false;
        m_filename = dlg.GetPath();
    }

    // load file
    return LoadFile(m_filename);
}

bool EditorCtrl::LoadFile(const wxString &filename) {

    // load file in EditorCtrl and clear undo
    if (!filename.empty())
        m_filename = filename;

    wxStyledTextCtrl::LoadFile(m_filename);
    EmptyUndoBuffer();

    // determine lexer language
    wxFileName fname(m_filename);
    InitializePreferences(DeterminePrefs(fname.GetFullName()));

    ConvertEOLs(wxSTC_EOL_LF);
    SetEOLMode(wxSTC_EOL_LF);

    return true;
}

bool EditorCtrl::SaveFile() {
    // return if no change
    if (!Modified())
        return true;

    // get filname
    if (!m_filename) {
        wxFileDialog dlg(this, wxT("Save file"), wxEmptyString, wxEmptyString, wxT("Any file (*)|*"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dlg.ShowModal() != wxID_OK)
            return false;
        m_filename = dlg.GetPath();
    }

    // save file
    return SaveFile(m_filename, true);
}

bool EditorCtrl::SaveFile(const wxString &filename, bool check_modified) {
    // return if no change
    if (!Modified() && check_modified)
        return true;

    //     // save EditorCtrl in file and clear undo
    //     if (!filename.empty()) m_filename = filename;
    //     wxFile file (m_filename, wxFile::write);
    //     if (!file.IsOpened()) return false;
    //     wxString buf = GetText();
    //     bool okay = file.Write (buf);
    //     file.Close();
    //     if (!okay) return false;
    //     EmptyUndoBuffer();
    //     SetSavePoint();

    //     return true;

    return wxStyledTextCtrl::SaveFile(filename);
}

bool EditorCtrl::Modified() {
    // return modified state
    return (GetModify() && !GetReadOnly());
}

//----------------------------------------------------------------------------
// EditorCtrlProperties
//----------------------------------------------------------------------------

EditorCtrlProperties::EditorCtrlProperties(EditorCtrl *editor, long style)
    : wxDialog(editor, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
               style | wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

    // sets the application title
    SetTitle(_("Properties"));
    wxString text;

    // fullname
    wxBoxSizer *fullname = new wxBoxSizer(wxHORIZONTAL);
    fullname->Add(10, 0);
    fullname->Add(new wxStaticText(this, wxID_ANY, _("Full filename"), wxDefaultPosition, wxSize(80, wxDefaultCoord)),
                  0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    fullname->Add(new wxStaticText(this, wxID_ANY, editor->GetFilename()), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    // text info
    wxGridSizer *textinfo = new wxGridSizer(4, 0, 2);
    textinfo->Add(new wxStaticText(this, wxID_ANY, _("Language"), wxDefaultPosition, wxSize(80, wxDefaultCoord)), 0,
                  wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    textinfo->Add(new wxStaticText(this, wxID_ANY, editor->m_selected_language->name), 0,
                  wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    textinfo->Add(new wxStaticText(this, wxID_ANY, _("Lexer-ID: "), wxDefaultPosition, wxSize(80, wxDefaultCoord)), 0,
                  wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    text = wxString::Format(wxT("%d"), editor->GetLexer());
    textinfo->Add(new wxStaticText(this, wxID_ANY, text), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    wxString EOLtype = wxEmptyString;
    switch (editor->GetEOLMode()) {
    case wxSTC_EOL_LF:
        EOLtype = wxT("LF (Unix)");
        break;

    case wxSTC_EOL_CRLF:
        EOLtype = wxT("CRLF (Windows)");
        break;

    case wxSTC_EOL_CR:
        EOLtype = wxT("CR (Macintosh)");
        break;
    }
    textinfo->Add(new wxStaticText(this, wxID_ANY, _("Line endings"), wxDefaultPosition, wxSize(80, wxDefaultCoord)), 0,
                  wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    textinfo->Add(new wxStaticText(this, wxID_ANY, EOLtype), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    // text info box
    wxStaticBoxSizer *textinfos = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Informations")), wxVERTICAL);
    textinfos->Add(textinfo, 0, wxEXPAND);
    textinfos->Add(0, 6);

    // statistic
    wxGridSizer *statistic = new wxGridSizer(4, 0, 2);
    statistic->Add(new wxStaticText(this, wxID_ANY, _("Total lines"), wxDefaultPosition, wxSize(80, wxDefaultCoord)), 0,
                   wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    text = wxString::Format(wxT("%d"), editor->GetLineCount());
    statistic->Add(new wxStaticText(this, wxID_ANY, text), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    statistic->Add(new wxStaticText(this, wxID_ANY, _("Total chars"), wxDefaultPosition, wxSize(80, wxDefaultCoord)), 0,
                   wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    text = wxString::Format(wxT("%d"), editor->GetTextLength());
    statistic->Add(new wxStaticText(this, wxID_ANY, text), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    statistic->Add(new wxStaticText(this, wxID_ANY, _("Current line"), wxDefaultPosition, wxSize(80, wxDefaultCoord)),
                   0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    text = wxString::Format(wxT("%d"), editor->GetCurrentLine());
    statistic->Add(new wxStaticText(this, wxID_ANY, text), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    statistic->Add(new wxStaticText(this, wxID_ANY, _("Current pos"), wxDefaultPosition, wxSize(80, wxDefaultCoord)), 0,
                   wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    text = wxString::Format(wxT("%d"), editor->GetCurrentPos());
    statistic->Add(new wxStaticText(this, wxID_ANY, text), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    // char/line statistics
    wxStaticBoxSizer *statistics = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Statistics")), wxVERTICAL);
    statistics->Add(statistic, 0, wxEXPAND);
    statistics->Add(0, 6);

    // total pane
    wxBoxSizer *totalpane = new wxBoxSizer(wxVERTICAL);
    totalpane->Add(fullname, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    totalpane->Add(0, 6);
    totalpane->Add(textinfos, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    totalpane->Add(0, 10);
    totalpane->Add(statistics, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    totalpane->Add(0, 6);
    wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
    okButton->SetDefault();
    totalpane->Add(okButton, 0, wxALIGN_CENTER | wxALL, 10);

    SetSizerAndFit(totalpane);

    ShowModal();
}
