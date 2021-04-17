
#ifndef EDITOR_CTRL_H
#define EDITOR_CTRL_H

#include "scintilla_definitions.h"
#include <fmt/ranges.h>
#include <unordered_set>
#include <wx/dialog.h>

class EditorCtrlProperties;

class EditorCtrl : public wxStyledTextCtrl {
    friend class EditorCtrlProperties;

  public:
    EditorCtrl(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition,
               const wxSize &size = wxDefaultSize, long style = wxSUNKEN_BORDER | wxVSCROLL);
    ~EditorCtrl();

    // event handlers
    // common
    void OnSize(wxSizeEvent &event);
    // edit
    void OnEditRedo(wxCommandEvent &event);
    void OnEditUndo(wxCommandEvent &event);
    void OnEditClear(wxCommandEvent &event);
    void OnEditCut(wxCommandEvent &event);
    void OnEditCopy(wxCommandEvent &event);
    void OnEditPaste(wxCommandEvent &event);
    // find
    void OnFind(wxCommandEvent &event);
    void OnFindNext(wxCommandEvent &event);
    void OnReplace(wxCommandEvent &event);
    void OnReplaceNext(wxCommandEvent &event);
    void OnGoto(wxCommandEvent &event);
    void OnEditSelectAll(wxCommandEvent &event);
    void OnEditSelectLine(wxCommandEvent &event);
    //! view
    void OnDisplayEOL(wxCommandEvent &event);
    void OnIndentGuide(wxCommandEvent &event);
    void OnLineNumber(wxCommandEvent &event);
    void OnLongLineOn(wxCommandEvent &event);
    void OnWhiteSpace(wxCommandEvent &event);
    void OnSetOverType(wxCommandEvent &event);
    void OnSetReadOnly(wxCommandEvent &event);
    void OnWrapmodeOn(wxCommandEvent &event);
    void OnBBToggle(wxCommandEvent &event);
    // stc
    void OnMarginClick(wxStyledTextEvent &event);
    void OnCharAdded(wxStyledTextEvent &event);

    void OnKeyDown(wxKeyEvent& evt);

    //! load/save file
    void NewFile();
    bool LoadFile(const wxString &filename);
    bool SaveFile();
    bool SaveFile(const wxString &filename, bool check_modified = true);
    bool Modified();
    wxString GetFilename() { return m_filename; };
    void SetFilename(const wxString &filename) { m_filename = filename; };

    //! language/lexer
    void OnStyleNeeded(wxStyledTextEvent &event);
    void UpdateCodeHighlighting(int startPos, int endPos);
    wxString DeterminePrefs(const wxString &filename);
    bool InitializePreferences(const wxString &filename);
    auto GetLanguageInfo() -> const LanguageInfo * { return m_selected_language; };

    //! educational mode
    void OnEduToggle(wxCommandEvent &event);
    void AnnotationAdd(int line, wxString ann);
    void AnnotationRemove(int line);
    void AnnotationClear();
    //! basic blocks
    void UpdateLineMarkers(const std::unordered_set<int> &basic_block_leaders);

    wxString ConvertAnnotationsToCommentaries();
    auto ConvertCommentariesToAnnotations() -> std::pair<wxString, std::vector<std::pair<int, wxString>>>;

  private:
    // file
    wxString m_filename;

    // language properties
    const LanguageInfo *m_selected_language;

    // margin variables
    int m_LineNrID;
    int m_FoldingID;
    int m_DividerID;
    int m_LineNrMargin;
    int m_FoldingMargin;

    enum class MarkerType {
        BasicBlockMark,
        BrightLineBackground,
        DarkLineBackground,
    };

    // show basic blocks mode
    bool m_show_basic_block_marks;

    // educational mode
    bool m_education_mode;
    int m_page_number;

    wxDECLARE_EVENT_TABLE();
};

//--------------------------------------------------------------
//! EditorCtrlProperties
class EditorCtrlProperties : public wxDialog {
  public:
    //! constructor
    EditorCtrlProperties(EditorCtrl *editor, long style = 0);

  private:
};

#endif // EDITOR_CTRL_H
