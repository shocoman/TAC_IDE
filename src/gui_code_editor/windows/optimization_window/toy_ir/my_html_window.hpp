//
// Created by victor on 13.03.2021.
//

#ifndef TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_TOY_MY_HTML_WINDOW_HPP
#define TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_TOY_MY_HTML_WINDOW_HPP

#include <wx/wxhtml.h>
#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include <functional>

class MyHTMLWindow : public wxHtmlWindow {
  public:

    std::function<wxWindow *(const wxHtmlTag &, wxWindow *)> html_callback;
    MyHTMLWindow(wxWindow *parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize,
                 long style = wxHW_SCROLLBAR_AUTO,
                 const wxString &name = wxT("HtmlWindow")) : wxHtmlWindow(parent, id, pos, size, style, name) {}

};



#endif // TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_TOY_MY_HTML_WINDOW_HPP
