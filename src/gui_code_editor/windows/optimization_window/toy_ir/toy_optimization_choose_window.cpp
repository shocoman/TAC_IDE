//
// Created by victor on 13.03.2021.
//

#include "toy_optimization_choose_window.hpp"

ToyOptimizationChooseWindow::ToyOptimizationChooseWindow(wxWindow *parent, wxString code)
    : OptimizationChooseDialog(parent), m_program_code(code) {

    auto program = Program::from_program_code(code.ToStdString());
    auto &f = program.functions[0];
    auto png_image_data = f.print_cfg();

    wxMemoryInputStream image_data_stream(png_image_data.data(), png_image_data.size());
    wxImage image;
    if (!image.LoadFile(image_data_stream, wxBitmapType::wxBITMAP_TYPE_PNG))
        std::cout << "Graph image loading error!" << std::endl;

    m_graph_panel->updateImage(image);

//    m_convert_to_ssa_btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent &event) {
//        auto *window = new ToyOptimizationDescriptionWindow(this);
//
//        int i = 0;
//        window->SetHtmlTagParserCallback([=](const wxHtmlTag &tag, wxHtmlWinParser *m_WParser) mutable {
//            i += 1;
//            wxWindow *wnd;
//            if (tag.GetName() == "MYIMAGE") {
//                auto img_src = "Label: " + tag.GetParam("src");
//                img_src += std::to_string(i);
//                wnd = new wxStaticText(m_WParser->GetWindowInterface()->GetHTMLWindow(),
//                                       wxID_ANY, img_src);
//                wnd->Show(true);
//                m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(wnd, 0));
//            } else if (tag.GetName() == "MYBIND") {
//                auto img_src = tag.GetParam("src");
//                wxImage image;
//                image.LoadFile(img_src);
//                wxBitmap mybitmap(image);
//                wnd = new wxStaticBitmap(m_WParser->GetWindowInterface()->GetHTMLWindow(),
//                                         wxID_ANY, mybitmap);
//                wnd->Show(true);
//                m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(wnd, 0));
//            } else if (tag.GetName() == "HIDE") {
//                auto *parent = m_WParser->GetWindowInterface()->GetHTMLWindow();
//
//                wnd = new wxButton(parent, wxID_ANY, "Show something");
//                wnd->Bind(wxEVT_BUTTON, [=](auto &e) {
//                    auto *f = new wxDialog(parent, wxID_ANY, "WINDOW");
//
//                    auto *sz = new wxBoxSizer(wxVERTICAL);
//                    sz->Add(new wxStaticText(f, wxID_ANY, "Just a text"));
//                    f->SetSizer(sz);
//                    f->ShowModal();
//                });
//
//                wnd->Show(true);
//                m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(wnd, 0));
//            }
//        });
//        window->LoadHTMLFile("htmls/start.html");
//
////        window->Layout();
////        window->Fit();
//        window->ShowModal();
//    });

}
