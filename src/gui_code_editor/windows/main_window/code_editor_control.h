
#ifndef EDITOR_CTRL_H
#define EDITOR_CTRL_H

//--------------------------------------------------------------
// headers
//--------------------------------------------------------------

//! application headers
#include "scintilla_definitions.h"
#include <wx/dialog.h>
#include <fmt/ranges.h>
#include <unordered_set>

class EditorCtrlProperties;

//--------------------------------------------------------------
//! EditorCtrl
class EditorCtrl : public wxStyledTextCtrl {
    friend class EditorCtrlProperties;

  public:
    //! constructor
    EditorCtrl(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition,
               const wxSize &size = wxDefaultSize,
               long style =
#ifndef __WXMAC__
                   wxSUNKEN_BORDER |
#endif
                   wxVSCROLL);

    //! destructor
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
    void OnUseCharset(wxCommandEvent &event);
    void OnBBToggle(wxCommandEvent &event);
    //! extra
    void OnConvertEOL(wxCommandEvent &event);
    // stc
    void OnMarginClick(wxStyledTextEvent &event);
    void OnCharAdded(wxStyledTextEvent &event);
    void OnKey(wxStyledTextEvent &event);

    void OnKeyDown(wxKeyEvent &event);

    //! load/save file
    bool LoadFile();
    bool LoadFile(const wxString &filename);
    bool SaveFile();
    bool SaveFile(const wxString &filename, bool check_modified = true);
    bool Modified();
    wxString GetFilename() { return m_filename; };
    void SetFilename(const wxString &filename) { m_filename = filename; };

    //! language/lexer
    void OnStyleNeeded(wxStyledTextEvent &event);
    void DoStyling(int startPos, int endPos);
    wxString DeterminePrefs(const wxString &filename);
    bool InitializePrefs(const wxString &filename);
    bool UserSettings(const wxString &filename);
    LanguageInfo const *GetLanguageInfo() { return m_language; };

    //! educational mode
    void OnEduToggle(wxCommandEvent &event);
    void OnEduHome(wxCommandEvent &event);
    void OnEduPrev(wxCommandEvent &event);
    void OnEduNext(wxCommandEvent &event);
    void OnEduReset(wxCommandEvent &event);
    void AnnotationAdd(int line, wxString ann);
    void AnnotationRemove(int line);
    void AnnotationClear();
    void ShowEduPage(int pageNr);
    //! basic blocks
    void DrawBBs();

  private:
    // file
    wxString m_filename;

    // language properties
    const LanguageInfo *m_language;

    // margin variables
    int m_LineNrID;
    int m_LineNrMargin;
    int m_FoldingID;
    int m_FoldingMargin;
    int m_DividerID;

    // show basic blocks mode
    bool showBB;
    std::vector<int> bbLeader;

    // educational mode
    bool eduMode;
    int pageNr;

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
