//
// Created by shoco on 2/3/2021.
//

#include "image_panel.hpp"

BEGIN_EVENT_TABLE(ImagePanel, wxPanel)
EVT_LEFT_DOWN(ImagePanel::mouseDown)
EVT_LEFT_UP(ImagePanel::mouseReleased)

EVT_MOUSEWHEEL(ImagePanel::mouseWheelMoved)
EVT_MOTION(ImagePanel::mouseMoved)

EVT_PAINT(ImagePanel::paintEvent)
EVT_SIZE(ImagePanel::OnSize)
END_EVENT_TABLE()

ImagePanel::ImagePanel(wxWindow *parent, const wxImage &image) : wxPanel(parent) {
    m_original_image = image;
    m_bitmap_image = wxBitmap(m_original_image);
    m_x = GetParent()->GetSize().x/2 - m_bitmap_image.GetWidth()/2;
    m_y = GetParent()->GetSize().y/2 - m_bitmap_image.GetHeight()/2;
}

ImagePanel::ImagePanel(wxWindow *parent, wxWindowID winid, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name)
    : wxPanel(parent, winid, pos, size, style, name) {
    m_original_image = wxImage(200, 200);
    m_bitmap_image = wxBitmap(m_original_image);
    m_x = GetParent()->GetSize().x/2 - m_bitmap_image.GetWidth()/2;
    m_y = GetParent()->GetSize().y/2 - m_bitmap_image.GetHeight()/2;
}

void ImagePanel::updateImage(const wxImage &image) {
    m_original_image = image;
    m_bitmap_image = wxBitmap(m_original_image);
    m_x = GetParent()->GetSize().x/2 - m_bitmap_image.GetWidth()/2;
    m_y = GetParent()->GetSize().y/2 - m_bitmap_image.GetHeight()/2;
    m_scale = 0.5;
    Refresh();
}

void ImagePanel::paintEvent(wxPaintEvent &evt) {
    wxPaintDC dc(this);
    render(dc);
}

void ImagePanel::paintNow() {
    wxClientDC dc(this);
    render(dc);
}

void ImagePanel::render(wxDC &dc) {
    dc.SetLogicalScale(m_scale, m_scale);
    dc.DrawBitmap(m_original_image, m_x, m_y, false);
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
        m_x += int((double)delta_x / m_scale);
        m_y += int((double)delta_y / m_scale);
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
    auto delta = event.m_wheelRotation;
    int sign = (delta > 0 ? 1 : -1);
    m_scale += 0.1 * sign;
    Refresh();
}
