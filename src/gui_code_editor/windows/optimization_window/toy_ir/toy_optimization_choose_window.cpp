//
// Created by victor on 13.03.2021.
//

#include "toy_optimization_choose_window.hpp"

ToyOptimizationChooseWindow::ToyOptimizationChooseWindow(wxWindow *parent, Function &chosen_function)
    : OptimizationChooseDialog(parent), m_chosen_function(chosen_function), m_ssa_form_is_active(false) {

    UpdateGraphImage();

    m_back_to_code_btn->Bind(wxEVT_BUTTON, [&](auto &evt) {
        if (m_ssa_form_is_active) {
            wxString msg = wxT("Для возврата обратно к коду, программа должна быть выведена из SSA-формы. "
                               "Вы уверены, что хотите продолжить?");
            auto dialog = new wxMessageDialog(this, msg, wxMessageBoxCaptionStr, wxYES_NO);
            if (dialog->ShowModal() != wxID_YES)
                return;
        }
        EndModal(wxID_OK);
    });

    auto AddButton = [&](bool is_optim, wxString btn_name, void (ToyOptimizationChooseWindow::*f)(wxCommandEvent &)) {
        auto *sz = is_optim ? m_optimization_sizer : m_analysis_sizer;
        auto *btn = new wxButton(sz->GetContainingWindow(), wxID_ANY, btn_name);
        btn->Bind(wxEVT_BUTTON, f, this);
        sz->Add(btn, 0, wxALL | wxEXPAND, 1);
    };

    using This = ToyOptimizationChooseWindow;
    AddButton(true, wxT("Конвертировать в SSA"), &This::ConvertToSSATutorial);
    AddButton(true, wxT("Конвертировать из SSA"), &This::ConvertFromSSATutorial);
    AddButton(true, wxT("Sparse Simple Constant Propagation"), &This::SSCPTutorial);
    AddButton(true, wxT("Sparse Conditional Constant Propagation"), &This::SCCPTutorial);
    AddButton(true, wxT("Lazy Code Motion"), &This::LazyCodeMotionTutorial);
    AddButton(true, wxT("Useless Code Elimination"), &This::UselessCodeEliminationTutorial);
    AddButton(true, wxT("Operator Strength Reduction"), &This::OperatorStrengthReductionTutorial);
    AddButton(true, wxT("Copy Propagation"), &This::CopyPropagationTutorial);
    AddButton(true, wxT("Global Value Numbering"), &This::GlobalValueNumberingTutorial);
    AddButton(true, wxT("Constant Folding"), &This::ConstantFoldingTutorial);

    AddButton(false, wxT("Depth First Search Tree"), &This::DepthFirstTreeTutorial);
    AddButton(false, wxT("Live Variable Analyses"), &This::LiveVariableAnalysisTutorial);
    AddButton(false, wxT("Reaching Definitions"), &This::ReachingDefinitionsTutorial);
    AddButton(false, wxT("Use Def Graph"), &This::UseDefGraphTutorial);
    AddButton(false, wxT("Dominator Tree"), &This::DominatorsTutorial);
    AddButton(false, wxT("Critical Edges"), &This::CriticalEdgesTutorial);
}

void ToyOptimizationChooseWindow::ConvertToSSATutorial(wxCommandEvent &event) {
    if (m_ssa_form_is_active) {
        wxString msg = wxT("Код уже находится в SSA-форме. Вы уверены, что хотите продолжить?");
        auto dialog = new wxMessageDialog(this, msg, wxMessageBoxCaptionStr, wxYES_NO);
        if (dialog->ShowModal() != wxID_YES)
            return;
    }

    auto *window = new ToyOptimizationDescriptionWindow(this);
    auto uninitialized_vars = LiveVariableAnalysisDriver(m_chosen_function).get_uninitialized_variables();
    ConvertToSSADriver convert_driver(m_chosen_function);

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
        if (not uninitialized_vars.empty()) {
            wxString msg = wxString::Format(wxT("Не были инициализированы следующие переменные: '%s'. "
                                                "Перевод в SSA-форму может привести к неработоспособной программе. "
                                                "Вы уверены, что хотите продолжить?"),
                                            fmt::format("{}", uninitialized_vars));
            auto dialog = new wxMessageDialog(this, msg, wxMessageBoxCaptionStr, wxYES_NO);
            if (dialog->ShowModal() != wxID_YES)
                return;
        }

        m_ssa_form_is_active = true;
        m_cfg_sizer->GetStaticBox()->SetLabel(wxT("Граф потока управления (SSA-форма)"));

        m_chosen_function = convert_driver.f;
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::UpdateGraphImage() {
    auto png_image_data = m_chosen_function.print_cfg();
    auto graph_image = LoadImageFromData(png_image_data);
    m_graph_panel->updateImage(graph_image);
}

void ToyOptimizationChooseWindow::UseDefGraphTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    UseDefGraph use_def_graph(m_chosen_function);

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

void ToyOptimizationChooseWindow::DominatorsTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;
        if (tag.HasParam("DOM_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, window->GetTagContent(tag));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = print_dominator_tree(m_chosen_function);
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/Dominators/text.html");
    window->m_accept_optimization->Hide();
    window->ShowModal();
}

void ToyOptimizationChooseWindow::ReachingDefinitionsTutorial(wxCommandEvent &event) {

    auto *window = new ToyOptimizationDescriptionWindow(this);
    ReachingDefinitionsDriver reaching_definitions(m_chosen_function);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) mutable {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("REACHDEF_GRAPH")) {
            wnd = new wxButton(parent, wxID_ANY, wxT("Показать достигающие определения для каждого блока"));
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
    LiveVariableAnalysisDriver live_variable_analysis(m_chosen_function);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) mutable {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("UNINIT_VARS")) {
            auto uninit_vars = live_variable_analysis.get_uninitialized_variables(false);
            wxString vars = fmt::format("{}", uninit_vars);
            if (uninit_vars.empty())
                vars = wxT("'Неинициализированные переменные отсутствуют'");
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
    LazyCodeMotionDriver lazy_code_motion(m_chosen_function);
    lazy_code_motion.run();

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("RESULT")) {
            wnd = new wxButton(parent, wxID_ANY, window->GetTagContent(tag));
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
        m_chosen_function = lazy_code_motion.f;
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::CopyPropagationTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    CopyPropagationDriver copy_propagation(m_chosen_function);
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
        m_chosen_function = copy_propagation.f;
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::OperatorStrengthReductionTutorial(wxCommandEvent &event) {
    if (not m_ssa_form_is_active) {
        wxString msg = wxT("Для работы данного алгоритма оптимизации код должен быть в SSA-форме. "
                           "Вы уверены, что хотите продолжить?");
        auto dialog = new wxMessageDialog(this, msg, wxMessageBoxCaptionStr, wxYES_NO);
        if (dialog->ShowModal() != wxID_YES)
            return;
    }

    auto *window = new ToyOptimizationDescriptionWindow(this);
    OperatorStrengthReductionDriver operator_strength_reduction(m_chosen_function);
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
        m_chosen_function = operator_strength_reduction.f;
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::UselessCodeEliminationTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    UselessCodeEliminationDriver useless_code_elimination(m_chosen_function);
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
        m_chosen_function = useless_code_elimination.f;
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::SSCPTutorial(wxCommandEvent &event) {
    if (not m_ssa_form_is_active) {
        wxString msg = wxT("Для работы данного алгоритма оптимизации код должен быть в SSA-форме. "
                           "Вы уверены, что хотите продолжить?");
        auto dialog = new wxMessageDialog(this, msg, wxMessageBoxCaptionStr, wxYES_NO);
        if (dialog->ShowModal() != wxID_YES)
            return;
    }

    auto *window = new ToyOptimizationDescriptionWindow(this);
    SparseSimpleConstantPropagationDriver sparse_simple_constant_propagation(m_chosen_function);
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
        m_chosen_function = sparse_simple_constant_propagation.f;
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::ConvertFromSSATutorial(wxCommandEvent &event) {
    if (not m_ssa_form_is_active) {
        wxString msg = wxT("Код не находится в SSA-форме. Вы уверены, что хотите продолжить?");
        auto dialog = new wxMessageDialog(this, msg, wxMessageBoxCaptionStr, wxYES_NO);
        if (dialog->ShowModal() != wxID_YES)
            return;
    }

    auto *window = new ToyOptimizationDescriptionWindow(this);
    ConvertFromSSADriver convert_from_ssa(m_chosen_function);

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
        m_ssa_form_is_active = false;
        m_cfg_sizer->GetStaticBox()->SetLabel(wxT("Граф потока управления"));

        m_chosen_function = convert_from_ssa.f;
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::SCCPTutorial(wxCommandEvent &event) {
    if (not m_ssa_form_is_active) {
        wxString msg = wxT("Для работы данного алгоритма оптимизации код должен быть в SSA-форме. "
                           "Вы уверены, что хотите продолжить?");
        auto dialog = new wxMessageDialog(this, msg, wxMessageBoxCaptionStr, wxYES_NO);
        if (dialog->ShowModal() != wxID_YES)
            return;
    }

    auto *window = new ToyOptimizationDescriptionWindow(this);
    SparseConditionalConstantPropagation sparse_conditional_constant(m_chosen_function);
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
        m_chosen_function.basic_blocks = std::move(sparse_conditional_constant.f.basic_blocks);
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
                auto dfs_tree_data = print_depth_first_search_tree(m_chosen_function);
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

void ToyOptimizationChooseWindow::GlobalValueNumberingTutorial(wxCommandEvent &event) {
    if (not m_ssa_form_is_active) {
        wxString msg = wxT("Для работы данного алгоритма оптимизации код должен быть в SSA-форме. "
                           "Вы уверены, что хотите продолжить?");
        auto dialog = new wxMessageDialog(this, msg, wxMessageBoxCaptionStr, wxYES_NO);
        if (dialog->ShowModal() != wxID_YES)
            return;
    }

    auto *window = new ToyOptimizationDescriptionWindow(this);
    GlobalValueNumberingDriver global_value_numbering(m_chosen_function);
    global_value_numbering.run();

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("REMOVED_QUADS")) {
            wnd = new wxButton(parent, wxID_ANY, window->GetTagContent(tag));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                wxDialog *dlg = new wxDialog(nullptr, wxID_ANY, "Удалённые выражения");
                auto *l = new wxListCtrl(dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

                wxListItem col0, col1;
                col0.SetId(0), col0.SetText(wxT("Блок")), col0.SetWidth(150);
                l->InsertColumn(0, col0);
                col1.SetId(1), col1.SetText(wxT("Инструкция")), col1.SetWidth(150);
                l->InsertColumn(1, col1);

                for (auto &[n1, n2] : global_value_numbering.ir.removed_quads) {
                    long itemIndex = l->InsertItem(l->GetItemCount(), n1);
                    l->SetItem(itemIndex, 1, n2);
                }
                dlg->ShowModal();
            });
        } else if (tag.HasParam("DOM_TREE")) {
            wnd = new wxButton(parent, wxID_ANY, window->GetTagContent(tag));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = print_dominator_tree(m_chosen_function);
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("RESULT")) {
            wnd = new wxButton(parent, wxID_ANY, window->GetTagContent(tag));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = global_value_numbering.f.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/GVN/text.html");
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        m_chosen_function.basic_blocks = std::move(global_value_numbering.f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::ConstantFoldingTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    Function f = m_chosen_function;
    run_constant_folding_on_every_quad(f);

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;
        if (tag.HasParam("RESULT")) {
            wnd = new wxButton(parent, wxID_ANY, window->GetTagContent(tag));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = f.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/ConstantFolding/text.html");
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        m_chosen_function.basic_blocks = std::move(f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}

void ToyOptimizationChooseWindow::CriticalEdgesTutorial(wxCommandEvent &event) {
    auto *window = new ToyOptimizationDescriptionWindow(this);
    CriticalEdgesDriver critical_edges_driver(m_chosen_function);
    critical_edges_driver.split_critical_edges();

    window->SetHtmlTagParserCallback([&](const wxHtmlTag &tag, wxWindow *parent) {
        wxWindow *wnd = nullptr;

        if (tag.HasParam("SHOW_CRITICAL")) {
            wnd = new wxButton(parent, wxID_ANY, window->GetTagContent(tag));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = critical_edges_driver.print_critical_edges();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        } else if (tag.HasParam("SPLIT_EDGES")) {
            wnd = new wxButton(parent, wxID_ANY, window->GetTagContent(tag));
            wnd->Bind(wxEVT_BUTTON, [&](auto &evt) {
                auto graph_data = critical_edges_driver.f.print_cfg();
                auto graph_viewer = new GraphView(this, LoadImageFromData(graph_data));
                graph_viewer->ShowModal();
            });
        }
        return wnd;
    });

    window->LoadHTMLFile("../_Tutorial/CriticalEdges/text.html");
    window->m_accept_optimization->SetLabel(wxT("Расщепить рёбра"));
    window->m_accept_optimization->Bind(wxEVT_BUTTON, [&](auto &evt) {
        m_chosen_function.basic_blocks = std::move(critical_edges_driver.f.basic_blocks);
        UpdateGraphImage();
        window->Close();
    });
    window->ShowModal();
}
