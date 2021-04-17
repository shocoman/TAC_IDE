
#ifndef IDE3ACMAIN_H
#define IDE3ACMAIN_H

#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/wx.h>
#include <wx/aui/aui.h>

#include <map>
#include <unordered_map>

#include "code_editor_control.h"
#include "scintilla_definitions.h"
#include "the_app.h"
#include "toy_optimizer/src/structure/program.hpp"
#include "windows/function_selection_window/function_chooser.h"
#include "windows/image_viewer/graph_view.h"
#include "windows/optimization_window/llvm_ir/optimization_select.hpp"
#include "windows/optimization_window/toy_ir/toy_optimization_choose_window.hpp"

//--------------------------------------------------------------
//! frame of the application APP_VENDOR-APP_NAME.
class MainWindowFrame : public wxFrame {
  public:
    MainWindowFrame(wxFrame *frame, const wxString &title, const wxPoint &pos = wxDefaultPosition,
                    const wxSize &size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);

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
    void OnSimulatorRun(wxCommandEvent &event);
    void OnOptimizationWindow(wxCommandEvent &event);
    void OnDisplayCFG(wxCommandEvent &event);

    void SetIRDialect(int id);

    void FileOpen(wxString fname);

    EditorCtrl* GetSelectedEditor();
  private:
    wxMenuBar *m_menuBar;
    void CreateMenu();

    wxToolBar *m_toolBar;
    void CreateToolbar();

    // main sizer of the application layout
    wxBoxSizer *m_vbox;
    // notebook object
    wxAuiNotebook *m_notebook;
    // unnamed new files counter
    int nonameFileCounter;

    DECLARE_EVENT_TABLE()
};

#endif // IDE3ACMAIN_H
