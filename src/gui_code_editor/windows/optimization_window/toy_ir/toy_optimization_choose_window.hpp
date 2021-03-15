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
#include "toy_optimizer/src/optimizations/ssa.hpp"
#include "utilities/image_loading.hpp"

class ToyOptimizationChooseWindow : public OptimizationChooseDialog {
    wxString m_program_code;
    Program program;
    Function &chosen_function;

  public:
    wxString output_code;

    ToyOptimizationChooseWindow(wxWindow *parent, wxString code);

    void UpdateGraphImage();
    void ConvertToSSA(wxCommandEvent &event);
};

#endif   // TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_STANDARD_TOYOPTIMIZATIONWINDOW_HPP
