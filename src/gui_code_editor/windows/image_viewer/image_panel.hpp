//
// Created by shoco on 2/3/2021.
//

#ifndef TAC_SIMULATOR_TEST_IMAGEPANEL_HPP
#define TAC_SIMULATOR_TEST_IMAGEPANEL_HPP

#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/wx.h>

class ImagePanel : public wxPanel {

    wxImage m_original_image, m_transformed_image;
    wxBitmap m_bitmap_image;
    bool m_should_update = false;

    wxPoint m_prev_mouse_pos = {0, 0};
    bool m_lmb_pressed = false;
    wxSize mouse_pos = {0, 0};

    double m_zoom = 1.0f;
    double m_offset_x = 0.0f;
    double m_offset_y = 0.0f;

  public:
    ImagePanel(wxWindow *parent, const wxImage &image);
    ImagePanel(wxWindow *parent,
               wxWindowID winid = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxTAB_TRAVERSAL | wxNO_BORDER,
               const wxString& name = wxPanelNameStr);

    void paintEvent(wxPaintEvent &evt);
    void paintNow();
    void OnSize(wxSizeEvent &event);
    void render(wxDC &dc);

    // some useful events
    void mouseMoved(wxMouseEvent &event);
    void mouseDown(wxMouseEvent &event);
    void mouseWheelMoved(wxMouseEvent &event);
    void mouseReleased(wxMouseEvent &event);

    void updateImage(const wxImage &image);

    void OnPaint( wxPaintEvent &evt );
    void SetZoom( double zoom, const wxPoint &center );

    DECLARE_EVENT_TABLE()
};

#endif   // TAC_SIMULATOR_TEST_IMAGEPANEL_HPP
