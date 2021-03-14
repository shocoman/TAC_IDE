#include "graph_view.h"

GraphView *GraphView::fromDotFile(wxWindow *parent, wxString dot_file_content) {
    auto png_image_data = renderGraphFromString(dot_file_content.ToStdString());
    return fromImageData(parent, png_image_data);
}

GraphView *GraphView::fromImageData(wxWindow *parent, std::vector<char> png_image_data) {
    wxMemoryInputStream image_data_stream(png_image_data.data(), png_image_data.size());
    wxImage image;
    if (!image.LoadFile(image_data_stream, wxBitmapType::wxBITMAP_TYPE_ANY))
        std::cout << "Graph image loading error!" << std::endl;
    return new GraphView(parent, image);
}

GraphView::GraphView(wxWindow *parent, wxImage image)
    : GraphView(parent, wxID_ANY, "GraphView", wxDefaultPosition, wxSize(900, 900)) {

    m_image_panel = new ImagePanel(this, image);
    m_main_sizer->Add(m_image_panel, 4, wxEXPAND);

    m_main_sizer->Layout();
    m_main_sizer->Fit(this);
    this->Fit();
    this->Layout();
}

GraphView::GraphView(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos,
                     const wxSize &size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    this->SetSizeHints(size);

    m_main_sizer = new wxBoxSizer(wxVERTICAL);

    this->SetSizer(m_main_sizer);
    this->Centre();
}
