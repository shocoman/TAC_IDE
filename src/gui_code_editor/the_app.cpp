
#include "the_app.h"
#include "windows/main_window/main_window.h"
#include <wx/txtstrm.h>
#include <wx/wfstream.h>

bool TheApp::OnInit() {
    wxInitAllImageHandlers();

    // set application and vendor name
    SetAppName(APP_NAME);

    // create application frame
//    m_frame = new MainWindowFrame(nullptr, APP_NAME);

    // set application icon
    // m_frame->SetIcon (wxICON(aaaa));

    // open application frame
//    m_frame->Layout();
//    m_frame->Show(true);

    wxFileInputStream input(wxT("../_TestCode/ssa_test.txt"));
    wxTextInputStream text(input, wxT("\x09"), wxConvUTF8);

    wxString code;
    while (input.IsOk() && !input.Eof()) {
        code << text.ReadLine() + "\n";
    }

    auto program = Program::from_program_code(code.ToStdString());
    auto &f = program.functions[0];
    auto graph = f.print_cfg();

    auto *win = new ToyOptimizationChooseWindow(nullptr, code);
//    auto *win = GraphView::fromImageData(nullptr, graph);
    win->ShowModal();

    return true;
}

// IMPLEMENT_APP(ide3acApp);
