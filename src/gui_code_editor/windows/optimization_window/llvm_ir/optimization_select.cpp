//
// Created by shoco on 2/1/2021.
//

#include "optimization_select.hpp"

BEGIN_EVENT_TABLE(LLVMOptimizationWindow, wxDialog)
EVT_CLOSE(LLVMOptimizationWindow::OnClose)
EVT_BUTTON(OPT_BUTTON, LLVMOptimizationWindow::OnButtonPressed)
EVT_LISTBOX(OPTIMIZATION_CHECK_BOX, LLVMOptimizationWindow::OnCheckboxClicked)
END_EVENT_TABLE()

LLVMOptimizationWindow::LLVMOptimizationWindow(wxFrame *frame, const wxString &title, wxString code)
    : wxDialog(frame, wxID_ANY, title, wxDefaultPosition, wxSize(600, 600)), m_program_code(code) {
    SetTitle("Optimization Window");

    m_optimizations = new wxCheckListBox(this, OPTIMIZATION_CHECK_BOX);
    for (auto &[name, desc] : g_optimization_descriptions) {
        int id = m_optimizations->Append(name);
    }

    m_optimization_description =
        new wxStaticText(this, OPTIMIZATION_DESCRIPTION, "", wxDefaultPosition, wxSize(250, 250));

    m_hbox = new wxBoxSizer(wxHORIZONTAL);
    m_hbox->Add(m_optimizations, 0, wxEXPAND);
    m_hbox->AddSpacer(10);
    m_hbox->Add(m_optimization_description, 0, wxALL | wxALIGN_CENTER);

    m_button = new wxButton(this, OPT_BUTTON, wxT("Применить"));
    m_vbox = new wxBoxSizer(wxVERTICAL);
    m_vbox->Add(m_hbox, 0, wxEXPAND);
    m_vbox->Add(m_button, 0, wxEXPAND, 10);

    SetSizer(m_vbox);
    m_vbox->Layout();

    Fit();
    Centre();
}

// common event handlers
void LLVMOptimizationWindow::OnClose(wxCloseEvent &event) { Destroy(); }

void LLVMOptimizationWindow::OnQuit(wxCommandEvent &event) { Destroy(); }

void LLVMOptimizationWindow::OnExit(wxCommandEvent &WXUNUSED(event)) { Close(true); }

void LLVMOptimizationWindow::OnButtonPressed(wxCommandEvent &event) { parse_and_optimize_code(); }

void LLVMOptimizationWindow::OnCheckboxClicked(wxCommandEvent &event) {
    auto &selected_description = g_optimization_descriptions[event.GetSelection()];
    m_optimization_description->SetLabel(selected_description.second);
    m_vbox->Layout();
}

void LLVMOptimizationWindow::parse_and_optimize_code() {

#if LLVM_FOUND
    using namespace llvm;
    LLVMContext context;
    SMDiagnostic err;
    auto module = parseAssemblyString(m_program_code.ToStdString(), err, context);
    if (!module) {
        err.print("Program", errs());
        std::string error_text;
        llvm::raw_string_ostream error_stream(error_text);
        err.print("Program", error_stream);
        wxMessageBox(error_text, wxT("Произошла ошибка!"), wxCENTER | wxOK | wxICON_ERROR);
        return;
    }
    auto fpm = std::make_unique<legacy::FunctionPassManager>(module.get());

    wxArrayInt selected_checkboxes;
    m_optimizations->GetCheckedItems(selected_checkboxes);
    for (auto &s : selected_checkboxes) {
        auto &[n, desc] = g_optimization_descriptions[s];

        if (n == "ConstantPropagation")
            fpm->add(createConstantPropagationPass());
        if (n == "AlignmentFromAssumptions")
            fpm->add(createAlignmentFromAssumptionsPass());
        if (n == "SCCP")
            fpm->add(createSCCPPass());
        if (n == "DeadInstElimination")
            fpm->add(createDeadInstEliminationPass());
        if (n == "RedundantDbgInstElimination")
            fpm->add(createRedundantDbgInstEliminationPass());
        if (n == "DeadCodeElimination")
            fpm->add(createDeadCodeEliminationPass());
        if (n == "DeadStoreElimination")
            fpm->add(createDeadStoreEliminationPass());
        if (n == "CallSiteSplitting")
            fpm->add(createCallSiteSplittingPass());
        if (n == "AggressiveDCE")
            fpm->add(createAggressiveDCEPass());
        if (n == "GuardWidening")
            fpm->add(createGuardWideningPass());
        if (n == "LoopGuardWidening")
            fpm->add(createLoopGuardWideningPass());
        if (n == "BitTrackingDCE")
            fpm->add(createBitTrackingDCEPass());
        if (n == "SROA")
            fpm->add(createSROAPass());
        if (n == "InductiveRangeCheckElimination")
            fpm->add(createInductiveRangeCheckEliminationPass());
        if (n == "IndVarSimplify")
            fpm->add(createIndVarSimplifyPass());
        if (n == "LICM")
            fpm->add(createLICMPass());
        if (n == "LoopSink")
            fpm->add(createLoopSinkPass());
        if (n == "LoopPredication")
            fpm->add(createLoopPredicationPass());
        if (n == "LoopInterchange")
            fpm->add(createLoopInterchangePass());
        if (n == "LoopStrengthReduce")
            fpm->add(createLoopStrengthReducePass());
        if (n == "LoopUnswitch")
            fpm->add(createLoopUnswitchPass());
        if (n == "LoopInstSimplify")
            fpm->add(createLoopInstSimplifyPass());
        if (n == "LoopUnroll")
            fpm->add(createLoopUnrollPass());
        if (n == "SimpleLoopUnroll")
            fpm->add(createSimpleLoopUnrollPass());
        if (n == "LoopUnrollAndJam")
            fpm->add(createLoopUnrollAndJamPass());
        if (n == "LoopReroll")
            fpm->add(createLoopRerollPass());
        if (n == "LoopRotate")
            fpm->add(createLoopRotatePass());
        if (n == "LoopIdiom")
            fpm->add(createLoopIdiomPass());
        if (n == "LoopVersioningLICM")
            fpm->add(createLoopVersioningLICMPass());
        if (n == "DemoteRegisterToMemory")
            fpm->add(createDemoteRegisterToMemoryPass());
        if (n == "Reassociate")
            fpm->add(createReassociatePass());
        if (n == "JumpThreading")
            fpm->add(createJumpThreadingPass());
        if (n == "CFGSimplification")
            fpm->add(createCFGSimplificationPass());
        if (n == "FlattenCFG")
            fpm->add(createFlattenCFGPass());
        if (n == "StructurizeCFG")
            fpm->add(createStructurizeCFGPass());
        if (n == "TailCallElimination")
            fpm->add(createTailCallEliminationPass());
        if (n == "EarlyCSE")
            fpm->add(createEarlyCSEPass());
        if (n == "GVNHoist")
            fpm->add(createGVNHoistPass());
        if (n == "GVNSink")
            fpm->add(createGVNSinkPass());
        if (n == "MergedLoadStoreMotion")
            fpm->add(createMergedLoadStoreMotionPass());
        if (n == "NewGVN")
            fpm->add(createNewGVNPass());
        if (n == "DivRemPairs")
            fpm->add(createDivRemPairsPass());
        if (n == "MemCpyOpt")
            fpm->add(createMemCpyOptPass());
        if (n == "LoopDeletion")
            fpm->add(createLoopDeletionPass());
        if (n == "ConstantHoisting")
            fpm->add(createConstantHoistingPass());
        if (n == "Sinking")
            fpm->add(createSinkingPass());
        if (n == "LowerAtomic")
            fpm->add(createLowerAtomicPass());
        if (n == "LowerGuardIntrinsic")
            fpm->add(createLowerGuardIntrinsicPass());
        if (n == "LowerMatrixIntrinsics")
            fpm->add(createLowerMatrixIntrinsicsPass());
        if (n == "LowerWidenableCondition")
            fpm->add(createLowerWidenableConditionPass());
        if (n == "MergeICmpsLegacy")
            fpm->add(createMergeICmpsLegacyPass());
        if (n == "CorrelatedValuePropagation")
            fpm->add(createCorrelatedValuePropagationPass());
        if (n == "InferAddressSpaces")
            fpm->add(createInferAddressSpacesPass());
        if (n == "LowerExpectIntrinsic")
            fpm->add(createLowerExpectIntrinsicPass());
        if (n == "LowerConstantIntrinsics")
            fpm->add(createLowerConstantIntrinsicsPass());
        if (n == "PartiallyInlineLibCalls")
            fpm->add(createPartiallyInlineLibCallsPass());
        if (n == "SeparateConstOffsetFromGEP")
            fpm->add(createSeparateConstOffsetFromGEPPass());
        if (n == "SpeculativeExecution")
            fpm->add(createSpeculativeExecutionPass());
        if (n == "SpeculativeExecutionIfHasBranchDivergence")
            fpm->add(createSpeculativeExecutionIfHasBranchDivergencePass());
        if (n == "StraightLineStrengthReduce")
            fpm->add(createStraightLineStrengthReducePass());
        if (n == "PlaceSafepoints")
            fpm->add(createPlaceSafepointsPass());
        if (n == "RewriteStatepointsForGCLegacy")
            fpm->add(createRewriteStatepointsForGCLegacyPass());
        if (n == "Float2Int")
            fpm->add(createFloat2IntPass());
        if (n == "NaryReassociate")
            fpm->add(createNaryReassociatePass());
        if (n == "LoopDistribute")
            fpm->add(createLoopDistributePass());
        if (n == "LoopFuse")
            fpm->add(createLoopFusePass());
        if (n == "LoopLoadElimination")
            fpm->add(createLoopLoadEliminationPass());
        if (n == "LoopVersioning")
            fpm->add(createLoopVersioningPass());
        if (n == "LoopDataPrefetch")
            fpm->add(createLoopDataPrefetchPass());
        if (n == "LibCallsShrinkWrap")
            fpm->add(createLibCallsShrinkWrapPass());
        if (n == "LoopSimplifyCFG")
            fpm->add(createLoopSimplifyCFGPass());
        if (n == "WarnMissedTransformations")
            fpm->add(createWarnMissedTransformationsPass());
    }

    fpm->doInitialization();
    for (auto &func : module->functions()) {
        fpm->run(func);
    }

    std::string string_stream;
    llvm::raw_string_ostream ostream(string_stream);
    module->print(ostream, nullptr);
    output_code = ostream.str();

    EndModal(wxID_OK);
#endif
}
