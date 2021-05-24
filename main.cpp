#include <fmt/ranges.h>
#include <iostream>
#include <vector>

#include "src/gui_code_editor/the_app.h"
//#include "tac_project.h"
//#include <wx/wx.h>

int main(int argc, char **argv) {
    if (getenv("DISPLAY") == nullptr)
        setenv("DISPLAY", "192.168.80.145:0", true);

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