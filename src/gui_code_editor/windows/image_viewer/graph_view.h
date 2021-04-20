#pragma once

#include <wx/artprov.h>
#include <wx/colour.h>
#include <wx/dialog.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/mstream.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/xrc/xmlres.h>

#include "convert_dot_to_image.hpp"
#include "image_panel.hpp"

class GraphView : public wxFrame {
    ImagePanel *m_image_panel;
    wxBoxSizer *m_main_sizer;

  public:
    GraphView(wxWindow *parent, wxImage image);
    GraphView(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString &title = wxEmptyString,
              const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
              long style = wxDEFAULT_DIALOG_STYLE);
    ~GraphView() = default;

    static GraphView *fromDotFile(wxWindow *parent, wxString dot_file_content);
    static GraphView *fromImageData(wxWindow *parent, std::vector<char> png_image_data);

    bool ShowModal(bool show = true) {
        return Show(show);
    }
};
