#ifndef IDE3ACAPP_H
#define IDE3ACAPP_H

#include <wx/app.h>

//--------------------------------------------------------------
// resources
//--------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources)
//#ifndef wxHAS_IMAGES_IN_RESOURCES
//    #include "../sample.xpm"
//#endif

//--------------------------------------------------------------
// declarations
//--------------------------------------------------------------

#define APP_NAME wxT("Three Address Code IDE")
#define APP_DESCR _("See readme.txt")

#define APP_LICENCE wxT("")

#define APP_VERSION wxT("0.1.alpha")
#define APP_BUILD __DATE__

#define APP_WEBSITE wxT("http://")

#define NONAME _("Untitled")

//--------------------------------------------------------------
class MainWindowFrame;

//! application APP_VENDOR-APP_NAME.
class TheApp : public wxApp {
    friend class ide3acFrame;

  public:
    //! the main function called during application start
    virtual bool OnInit();

  private:
    //! frame window
    MainWindowFrame *m_frame;

    // wxDECLARE_EVENT_TABLE();
};

// created dynamically by wxWidgets
// DECLARE_APP (App);

#endif   // IDE3ACAPP_H
