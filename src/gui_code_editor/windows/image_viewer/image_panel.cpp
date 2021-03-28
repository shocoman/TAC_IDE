//
// Created by shoco on 2/3/2021.
//

#include "image_panel.hpp"

BEGIN_EVENT_TABLE(ImagePanel, wxPanel)
EVT_LEFT_DOWN(ImagePanel::mouseDown)
EVT_LEFT_UP(ImagePanel::mouseReleased)

EVT_MOUSEWHEEL(ImagePanel::mouseWheelMoved)
EVT_MOTION(ImagePanel::mouseMoved)

EVT_PAINT(ImagePanel::OnPaint)
EVT_SIZE(ImagePanel::OnSize)
END_EVENT_TABLE()

ImagePanel::ImagePanel(wxWindow *parent, const wxImage &image) : wxPanel(parent) { updateImage(image); }

ImagePanel::ImagePanel(wxWindow *parent, wxWindowID winid, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name)
    : wxPanel(parent, winid, pos, size, style, name) {
    updateImage(wxImage(200, 200));
}

void ImagePanel::updateImage(const wxImage &image) {
    m_original_image = image;
    m_transformed_image = m_original_image.Copy();
    m_bitmap_image = wxBitmap(m_original_image);
    m_offset_x = GetParent()->GetSize().x / 2 - m_bitmap_image.GetWidth() / 2;
    m_offset_y = GetParent()->GetSize().y / 2 - m_bitmap_image.GetHeight() / 2;
    Refresh();

    m_zoom = 1.0f;
    m_offset_x = 0.0f;
    m_offset_y = 0.0f;
}

void ImagePanel::OnSize(wxSizeEvent &event) {
    Refresh();
    event.Skip();
}

void ImagePanel::mouseMoved(wxMouseEvent &event) {
    auto [x, y] = event.GetPosition();

    auto [prev_x, prev_y] = m_prev_mouse_pos;
    int delta_x = x - prev_x, delta_y = y - prev_y;

    if (m_lmb_pressed) {
        m_offset_x += int((double)delta_x * m_zoom);
        m_offset_y += int((double)delta_y * m_zoom);
        Refresh();
    }

    m_prev_mouse_pos = event.GetPosition();
}

void ImagePanel::mouseDown(wxMouseEvent &event) {
    m_lmb_pressed = true;
    m_prev_mouse_pos = event.GetPosition();
}

void ImagePanel::mouseReleased(wxMouseEvent &event) { m_lmb_pressed = false; }

void ImagePanel::mouseWheelMoved(wxMouseEvent &event) {
    m_should_update = true;
    auto delta = event.m_wheelRotation;
    SetZoom(m_zoom * (delta > 0 ? 2.0 : 0.5), event.GetPosition());
}

void ImagePanel::OnPaint(wxPaintEvent &evt) {
    wxPaintDC dc(this);

    dc.SetBrush(*wxWHITE_BRUSH);
    dc.SetPen(*wxWHITE_PEN);
    dc.DrawRectangle(GetClientSize());

    if (m_bitmap_image.IsOk()) {
        dc.SetUserScale(m_zoom, m_zoom);

        double x = m_offset_x / m_zoom, y = m_offset_y / m_zoom;
        dc.DrawBitmap(m_bitmap_image, x, y, true);

        dc.SetPen(*wxBLACK_PEN);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle({(int)x, (int)y}, m_bitmap_image.GetSize());

        dc.SetUserScale(1., 1.);
        dc.DrawRoundedRectangle({0, 0}, dc.GetSize(), 5);
    } else {
        dc.DrawText("Bitmap is not OK", 10, 10);
    }
}

void ImagePanel::SetZoom(double zoom, const wxPoint &center) {
    double image_x = ((double)center.x - m_offset_x) / m_zoom;
    double image_y = ((double)center.y - m_offset_y) / m_zoom;

    m_offset_x = center.x - image_x * zoom;
    m_offset_y = center.y - image_y * zoom;

    m_zoom = zoom;
    Refresh();
}
