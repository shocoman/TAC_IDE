#include <iostream>

#include "src/gui_code_editor/the_app.h"
//#include "tac_project.h"
//#include <wx/wx.h>

int main(int argc, char **argv) {
    //    simulator_main(argc, argv);

    setenv("DISPLAY", "192.168.185.81:0", true);

    wxApp *pApp = new TheApp();
    wxApp::SetInstance(pApp);
    wxEntryStart(argc, argv);
    wxTheApp->OnInit();

    // wxWidgets event loop
    wxTheApp->OnRun();

    // cleaning up...
    wxTheApp->OnExit();
    wxEntryCleanup();

    return 0;
}