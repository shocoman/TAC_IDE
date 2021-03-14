//
// Created by shoco on 3/13/2021.
//

#ifndef TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_TOY_TOY_OPTIMIZATION_DESCRIPTION_WINDOW_HPP
#define TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_TOY_TOY_OPTIMIZATION_DESCRIPTION_WINDOW_HPP

#include "generated_gui_classes/my_generated_gui.h"
#include <functional>
#include <utility>

class ToyOptimizationDescriptionWindow : public OptimizationShowDialog {

  public:
    ToyOptimizationDescriptionWindow(wxWindow *parent);
    void SetHtmlTagParserCallback(std::function<void(const wxHtmlTag &, wxHtmlWinParser *)> func) {
        m_htmlWin2->myfunc = std::move(func);
    }
    void LoadHTMLFile(wxString path) {
        m_htmlWin2->LoadFile(path);
    }
};

#endif // TAC_SIMULATOR_TEST_GUI_CODE_EDITOR_WINDOWS_OPTIMIZATIONWINDOW_TOY_TOY_OPTIMIZATION_DESCRIPTION_WINDOW_HPP
