//
// Created by victor on 13.03.2021.
//

#include "toy_optimization_choose_window.hpp"

ToyOptimizationChooseWindow::ToyOptimizationChooseWindow(wxWindow *parent, wxString code)
    : OptimizationChooseDialog(parent), m_program_code(code),
      program(Program::from_program_code(code.ToStdString())), chosen_function(program.functions[0]) {

    UpdateGraphImage();

    m_convert_to_ssa_btn->Bind(wxEVT_BUTTON, &ToyOptimizationChooseWindow::ConvertToSSA, this);
}

void ToyOptimizationChooseWindow::ConvertToSSA(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    SSAConvertationDriver convert_driver(chosen_function);
    convert_driver.convert_to_ssa();

    //        intermediate_results.dominance_frontier_graph_image = print_dominator_tree(f);
//        intermediate_results.live_variable_analysis_graph_image = print_live_variable(f);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxHtmlWinParser *m_WParser) mutable {
        wxWindow *parent = m_WParser->GetWindowInterface()->GetHTMLWindow();
        wxWindow *wnd = nullptr;

        wxImage graph_image(100, 100);
        if (tag.HasParam("LIST_OF_GLOBAL_NAMES")) {
            auto &global_names = convert_driver.intermediate_results.global_names;
            auto text = split_long_string(fmt::format("{}", global_names), 50, "\n");
            wnd = new wxStaticText(parent, wxID_ANY, text);
        } else if (tag.HasParam("CFG_BEFORE_CONVERT")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = convert_driver.intermediate_results.cfg_before_convert.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("DOMINATOR_FRONTIER")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать дерево доминаторов и фронтов"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto &f = convert_driver.intermediate_results.cfg_before_convert;
                auto graph_data = print_dominator_tree(f);
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("LIVE_ANALYSIS")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать результат анализа живых переменных"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto &f = convert_driver.intermediate_results.cfg_before_convert;
                auto graph_data = print_live_variable(f);
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("CFG_BEFORE_RENAME")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data =
                    convert_driver.intermediate_results.cfg_after_phi_placement.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("CFG_AFTER_CONVERT")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = convert_driver.intermediate_results.cfg_after_renaming.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        if (wnd != nullptr)
            m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(wnd, 0));
    });

    window->LoadHTMLFile("../_Tutorial/ToSSA/to_ssa.html");
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        chosen_function.basic_blocks = std::move(convert_driver.f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::UpdateGraphImage() {
    auto png_image_data = chosen_function.print_cfg();
    auto graph_image = LoadImageFromData(png_image_data);
    m_graph_panel->updateImage(graph_image);
}
