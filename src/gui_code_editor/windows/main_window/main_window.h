
#ifndef IDE3ACMAIN_H
#define IDE3ACMAIN_H

#include <wx/wx.h>
#include <wx/stdpaths.h>

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
    friend class ide3acApp;

  public:
    //! constructor
    MainWindowFrame(wxFrame *frame, const wxString &title);
    //! destructor
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

  private:
    void FileOpen(wxString fname);

#if wxUSE_MENUS
    //! creates the application menu bar
    wxMenuBar *m_menuBar;
    void CreateMenu();
#endif   // wxUSE_MENUS

#if wxUSE_TOOLBAR
    //! creates a tool bar with some frequently used buttons
    wxToolBar *m_toolBar;
    //! creates a tool bar with educational mode control buttons
    wxToolBar *m_edu_toolBar;
    void CreateToolbar();
#endif   // wxUSE_TOOLBAR

    // important variables

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
    LLVMOptimizationWindow *m_optimization_window;

    DECLARE_EVENT_TABLE()
};

#endif   // IDE3ACMAIN_H
