#include <fmt/ranges.h>
#include <iostream>
#include <vector>

#include "src/gui_code_editor/the_app.h"
//#include "tac_project.h"
//#include <wx/wx.h>

int main(int argc, char **argv) {
    if (getenv("DISPLAY") == nullptr)
        setenv("DISPLAY", "172.18.67.241:0", true);

    wxApp *pApp = new TheApp();
    wxApp::SetInstance(pApp);
    wxEntryStart(argc, argv);
    wxTheApp->OnInit();

    // wxWidgets event loop
    wxTheApp->OnRun();

    // cleaning up...
    wxTheApp->OnExit();
    wxEntryCleanup();

    std::vector v =  {1,2,3};


    return 0;
}