//
// Created by shoco on 2/3/2021.
//

#ifndef TAC_SIMULATOR_TEST_IMAGEPANEL_HPP
#define TAC_SIMULATOR_TEST_IMAGEPANEL_HPP

#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/wx.h>

class ImagePanel : public wxPanel {

    wxImage m_original_image;
    wxBitmap m_bitmap_image;

    wxPoint m_prev_mouse_pos = {0, 0};
    int m_x = 0, m_y = 0;
    bool m_lmb_pressed = false;
    double m_scale = 1.0;

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
    void rightClick(wxMouseEvent &event);
    void mouseLeftWindow(wxMouseEvent &event);
    void keyPressed(wxKeyEvent &event);
    void keyReleased(wxKeyEvent &event);

    void updateImage(const wxImage &image);

    DECLARE_EVENT_TABLE()
};

#endif   // TAC_SIMULATOR_TEST_IMAGEPANEL_HPP
