//
// Created by victor on 13.03.2021.
//

#ifndef TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_STANDARD_TOYOPTIMIZATIONWINDOW_HPP
#define TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_STANDARD_TOYOPTIMIZATIONWINDOW_HPP

#include <wx/mstream.h>

#include "generated_gui_classes/my_generated_gui.h"
#include "toy_optimization_description_window.hpp"
#include "windows/image_viewer/image_panel.hpp"
#include "toy_optimizer/src/structure/program.hpp"

class ToyOptimizationChooseWindow : public OptimizationChooseDialog {
    wxString m_program_code;

  public:
    ToyOptimizationChooseWindow(wxWindow *parent, wxString code);

    wxString output_code;
};

#endif   // TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_STANDARD_TOYOPTIMIZATIONWINDOW_HPP
