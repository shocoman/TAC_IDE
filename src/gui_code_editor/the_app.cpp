
#include "the_app.h"
#include "windows/main_window/main_window.h"
#include <wx/dir.h>
#include <wx/listctrl.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/wx.h>

bool TheApp::OnInit() {
    wxInitAllImageHandlers();

    // set application and vendor name
    SetAppName(APP_NAME);

    // create application frame
    m_frame = new MainWindowFrame(nullptr, APP_NAME, wxDefaultPosition, wxSize(960, 640),
                                  wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE);
    //    m_frame->FileOpen("../_Examples/Toy/new_sccp_test.txt");
    m_frame->FileOpen("../_Examples/edu_test.txt");

    // open application frame
    m_frame->Layout();
    m_frame->Show(true);

    //    wxFileInputStream input(wxT("../_Examples/Toy/bob_maxcol.txt"));
    //    wxTextInputStream text(input, wxT("\x09"), wxConvUTF8);
    //    wxString code;
    //    while (input.IsOk() && !input.Eof())
    //        code << text.ReadLine() + "\n";
    //
    //    auto program = Program::from_program_code(code.ToStdString());
    //    auto &f = program.functions[0];
    ////    auto graph = f.print_cfg();
    //
    //    auto *win = new ToyOptimizationChooseWindow(nullptr, f);
    ////    auto *win = GraphView::fromImageData(nullptr, graph);
    //    win->ShowModal();

    return true;
}

// IMPLEMENT_APP(ide3acApp);
