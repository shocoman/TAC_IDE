//
// Created by victor on 13.03.2021.
//

#include "toy_optimization_choose_window.hpp"

ToyOptimizationChooseWindow::ToyOptimizationChooseWindow(wxWindow *parent, wxString code)
    : OptimizationChooseDialog(parent), program(Program::from_program_code(code.ToStdString())),
      chosen_function(program.functions[0]) {

    UpdateGraphImage();


    m_back_to_code_btn->Bind(wxEVT_BUTTON, [&](auto &evt) {
        output_code = program.get_as_code();
        EndModal(wxID_OK);
    });

    auto AddButton = [&](wxString btn_name, std::function<void(wxCommandEvent &)> f, bool is_optim) {
        auto *sz = is_optim ? m_optimization_sizer : m_analysis_sizer;
        auto *btn = new wxButton(sz->GetContainingWindow(), wxID_ANY, btn_name);
        btn->Bind(wxEVT_BUTTON, f);
        sz->Add(btn, 0, wxALL | wxEXPAND, 2);
    };

    AddButton(
        wxT("Конвертировать в SSA"), [&](auto &evt) { ConvertToSSATutorial(evt); }, true);
    AddButton(
        wxT("Конвертировать из SSA"), [&](auto &evt) { ConvertFromSSATutorial(evt); }, true);
    AddButton(
        wxT("SCCP"), [&](auto &evt) { SCCPTutorial(evt); }, true);
    AddButton(
        wxT("SSCP"), [&](auto &evt) { SSCPTutorial(evt); }, true);
    AddButton(
        wxT("Lazy Code Motion"), [&](auto &evt) { LazyCodeMotionTutorial(evt); }, true);
    AddButton(
        wxT("Useless Code Elimination"), [&](auto &evt) { UselessCodeEliminationTutorial(evt); }, true);
    AddButton(
        wxT("Operator Strength Reduction"), [&](auto &evt) { OperatorStrengthReductionTutorial(evt); },
        true);
    AddButton(
        wxT("Copy Propagation"), [&](auto &evt) { CopyPropagationTutorial(evt); }, true);

    AddButton(
        wxT("DFS Tree"), [&](auto &evt) { DepthFirstTreeTutorial(evt); }, false);
    AddButton(
        wxT("Live Variable Analyses"), [&](auto &evt) { LiveVariableAnalysisTutorial(evt); }, false);
    AddButton(
        wxT("Reaching Definitions"), [&](auto &evt) { ReachingDefinitionsTutorial(evt); }, false);
    AddButton(
        wxT("Use Def Graph"), [&](auto &evt) { UseDefGraphTutorial(evt); }, false);
}

void ToyOptimizationChooseWindow::ConvertToSSATutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    ConvertToSSADriver convert_driver(chosen_function);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) mutable {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("LIST_OF_GLOBAL_NAMES")) {
            auto &global_names = convert_driver.ir.global_names;
            auto text = split_long_string(fmt::format("{}", global_names), 50, "\n");
            wnd = new wxStaticText(parent, wxID_ANY, text);
        } else if (tag.HasParam("CFG_BEFORE_CONVERT")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = convert_driver.ir.f_before_convert.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("DOMINATOR_FRONTIER")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать дерево доминаторов и фронтов"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto &f = convert_driver.ir.f_before_convert;
                auto graph_data = print_dominator_tree(f);
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("LIVE_ANALYSIS")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать результат анализа живых переменных"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto &f = convert_driver.ir.f_before_convert;
                auto graph_data = print_live_variable(f);
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("CFG_BEFORE_RENAME")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = convert_driver.ir.f_after_phi_placement.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("CFG_AFTER_CONVERT")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = convert_driver.ir.f_after_renaming.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/ToSSA/text.html");
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

void ToyOptimizationChooseWindow::UseDefGraphTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    UseDefGraph use_def_graph(chosen_function);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) mutable {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("USE_DEF_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф использований-определений"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = use_def_graph.print_use_def_chains_graph();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/UseDefGraph/text.html");
    window->m_accept_optimization->Hide();
    window->ShowModal();
}

void ToyOptimizationChooseWindow::ReachingDefinitionsTutorial(wxCommandEvent &event) {

    auto *window = new ToyOptimizationDescriptionWindow(this);
    ReachingDefinitionsDriver reaching_definitions(chosen_function);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) mutable {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("REACHDEF_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY,
                               wxT("Показать достигающие определения для каждого блока"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto image_data = reaching_definitions.print_reaching_definitions();
                auto graph_viewer = new GraphView(this, LoadImageFromData(image_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/ReachingDefinition/text.html");
    window->m_accept_optimization->Hide();
    window->ShowModal();
}

void ToyOptimizationChooseWindow::LiveVariableAnalysisTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    LiveVariableAnalysisDriver live_variable_analysis(chosen_function);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) mutable {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("UNINIT_VARS")) {
            auto uninit_vars = live_variable_analysis.get_uninitialized_variables(false);
            wxString vars = fmt::format("{}", uninit_vars);
            if (uninit_vars.empty())
                vars = wxT("'-'");
            wnd = new wxStaticText(parent, wxID_ANY, vars);
        } else if (tag.HasParam("LIVENESS_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать активные переменные для каждого блока"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto image_data = live_variable_analysis.print_live_variable_analysis();
                auto graph_viewer = new GraphView(this, LoadImageFromData(image_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/LivenessAnalysis/text.html");
    window->m_accept_optimization->Hide();
    window->ShowModal();
}

void ToyOptimizationChooseWindow::LazyCodeMotionTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    LazyCodeMotionDriver lazy_code_motion(chosen_function);
    lazy_code_motion.run();

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("RESULT")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = lazy_code_motion.f.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }

        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/LCM/text.html");
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        chosen_function.basic_blocks = std::move(lazy_code_motion.f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::CopyPropagationTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    CopyPropagationDriver copy_propagation(chosen_function);
    copy_propagation.run_on_non_ssa();

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("COPY_PROP")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = copy_propagation.print_copy_propagation_for_block();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("RESULT")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = copy_propagation.f.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }

        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/CopyPropagation/text.html");
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        chosen_function.basic_blocks = std::move(copy_propagation.f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::OperatorStrengthReductionTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    OperatorStrengthReductionDriver operator_strength_reduction(chosen_function);
    operator_strength_reduction.run();

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("SSA_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = operator_strength_reduction.PrintSSAGraph();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("RESULT")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = operator_strength_reduction.f.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }

        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/OSR/text.html");
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        chosen_function.basic_blocks = std::move(operator_strength_reduction.f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::UselessCodeEliminationTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    UselessCodeEliminationDriver useless_code_elimination(chosen_function);
    useless_code_elimination.run();

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("RESULT_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = useless_code_elimination.f.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/UCE/text.html");
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        chosen_function.basic_blocks = std::move(useless_code_elimination.f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::SSCPTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    SparseSimpleConstantPropagationDriver sparse_simple_constant_propagation(chosen_function);
    sparse_simple_constant_propagation.run();

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("RESULT_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = sparse_simple_constant_propagation.f.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/SSCP/text.html");
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        chosen_function.basic_blocks = std::move(sparse_simple_constant_propagation.f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::ConvertFromSSATutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    ConvertFromSSADriver convert_from_ssa(chosen_function);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("DESSA_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф после удаление Ф-узлов"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = convert_from_ssa.f.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/FromSSA/text.html");
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        chosen_function.basic_blocks = std::move(convert_from_ssa.f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::SCCPTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    SparseConditionalConstantPropagation sparse_conditional_constant(chosen_function);
    sparse_conditional_constant.run();

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("SCCP_CHANGES_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф с изменениями"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = sparse_conditional_constant.print_sccp_result_graph();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("RESULT_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать граф"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = sparse_conditional_constant.f.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/SCCP/text.html");
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        chosen_function.basic_blocks = std::move(sparse_conditional_constant.f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::DepthFirstTreeTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) mutable {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("DFS_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать остовное дерево"));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto dfs_tree_data = print_depth_first_search_tree(chosen_function);
                auto graph_viewer = new GraphView(this, LoadImageFromData(dfs_tree_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/DFSTree/text.html");
    window->m_accept_optimization->Hide();
    window->ShowModal();
}
