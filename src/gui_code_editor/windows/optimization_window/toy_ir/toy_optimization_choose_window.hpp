//
// Created by victor on 13.03.2021.
//

#ifndef TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_STANDARD_TOYOPTIMIZATIONWINDOW_HPP
#define TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_STANDARD_TOYOPTIMIZATIONWINDOW_HPP

#include <wx/mstream.h>

#include "generated_gui_classes/my_generated_gui.h"
#include "toy_optimization_description_window.hpp"
#include "windows/image_viewer/image_panel.hpp"
#include "windows/image_viewer/graph_view.h"
#include "toy_optimizer/src/structure/program.hpp"
#include "toy_optimizer/src/all_headers.hpp"
#include "utilities/image_loading.hpp"

class ToyOptimizationChooseWindow : public OptimizationChooseDialog {
    Function &m_chosen_function;
    bool m_ssa_form_is_active;

  public:

    ToyOptimizationChooseWindow(wxWindow *parent, Function &chosen_function);

    void UpdateGraphImage();

    void ConvertToSSATutorial(wxCommandEvent &event);
    void ConvertFromSSATutorial(wxCommandEvent &event);
    void SSCPTutorial(wxCommandEvent &event);
    void SCCPTutorial(wxCommandEvent &event);
    void UselessCodeEliminationTutorial(wxCommandEvent &event);
    void OperatorStrengthReductionTutorial(wxCommandEvent &event);
    void CopyPropagationTutorial(wxCommandEvent &event);
    void LazyCodeMotionTutorial(wxCommandEvent &event);

    void LiveVariableAnalysisTutorial(wxCommandEvent &event);
    void ReachingDefinitionsTutorial(wxCommandEvent &event);
    void UseDefGraphTutorial(wxCommandEvent &event);
    void DepthFirstTreeTutorial(wxCommandEvent &event);

};

#endif   // TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_STANDARD_TOYOPTIMIZATIONWINDOW_HPP
