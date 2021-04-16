
#ifndef IDE3ACMAIN_H
#define IDE3ACMAIN_H

#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>

#include <unordered_map>
#include <map>

#include "code_editor_control.h"
#include "scintilla_definitions.h"
#include "the_app.h"
#include "windows/image_viewer/graph_view.h"
#include "windows/optimization_window/llvm_ir/optimization_select.hpp"
#include "windows/optimization_window/toy_ir/toy_optimization_choose_window.hpp"
#include "windows/function_selection_window/function_chooser.h"
#include "toy_optimizer/src/structure/program.hpp"

//--------------------------------------------------------------
//! frame of the application APP_VENDOR-APP_NAME.
class MainWindowFrame : public wxFrame {
  public:
    MainWindowFrame(wxFrame *frame, const wxString &title);
    ~MainWindowFrame();

    //! event handlers
    //! common
    void OnClose(wxCloseEvent &event);
    void OnQuit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
    void OnMainToolbarToggle(wxCommandEvent &event);
    //! file
    void OnFileNew(wxCommandEvent &event);
    void OnFileOpen(wxCommandEvent &event);
    void OnFileSave(wxCommandEvent &event);
    void OnFileSaveAs(wxCommandEvent &event);
    void OnFileClose(wxCommandEvent &event);
    //! properties
    void OnProperties(wxCommandEvent &event);
    //! edit events
    void OnEdit(wxCommandEvent &event);
    void OnModified(wxStyledTextEvent &event);
    //! entity
    void OnEduToggle(wxCommandEvent &event);
    void OnEduPageChange(wxCommandEvent &event);
    void OnSimulatorRun(wxCommandEvent &event);
    void OnOptimizationWindow(wxCommandEvent &event);
    void OnDisplayCFG(wxCommandEvent &event);

    void SetIRDialect(int id);

    void FileOpen(wxString fname);
  private:

    wxMenuBar *m_menuBar;
    void CreateMenu();

    wxToolBar *m_toolBar;
    wxToolBar *m_edu_toolBar;
    void CreateToolbar();


    // main sizer of the application layout
    wxBoxSizer *m_vbox;
    // notebook object
    wxNotebook *m_notebook;
    // unnamed new files counter
    int nonameFileCounter;
    // splitter windows
    wxSplitterWindow *m_splittermain;
    wxSplitterWindow *m_splitCode;
    // editor object
    EditorCtrl *m_editor;
    // asm code window
    wxTextCtrl *m_asm;
    // output window
    wxTextCtrl *m_log;

    DECLARE_EVENT_TABLE()
};

#endif   // IDE3ACMAIN_H
