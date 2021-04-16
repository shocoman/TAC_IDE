#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <wx/wx.h>

class FunctionChooser : public wxDialog {
  private:
  protected:
    wxStaticText *m_staticText1;
    wxChoice *m_chosen_function;
    wxButton *m_ok_btn;

  public:
    FunctionChooser(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString &title = wxT("Function Chooser"),
                    const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
                    long style = wxDEFAULT_DIALOG_STYLE);
    FunctionChooser(wxWindow *parent, wxArrayString function_names);

    wxString get_selected_function_name();
};
