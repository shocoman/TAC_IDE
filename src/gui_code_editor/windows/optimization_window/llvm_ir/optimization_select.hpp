//
// Created by shoco on 2/1/2021.
//

#ifndef TAC_SIMULATOR_TEST_OPTIMIZATIONSELECT_HPP
#define TAC_SIMULATOR_TEST_OPTIMIZATIONSELECT_HPP

#if LLVM_FOUND
    #include <llvm-11/llvm/Bitcode/BitcodeWriter.h>
    #include <llvm-11/llvm/ExecutionEngine/ExecutionEngine.h>
    #include <llvm-11/llvm/ExecutionEngine/GenericValue.h>
    #include <llvm-11/llvm/ExecutionEngine/MCJIT.h>
    #include <llvm-11/llvm/IR/Constants.h>
    #include <llvm-11/llvm/IR/Function.h>
    #include <llvm-11/llvm/IR/IRBuilder.h>
    #include <llvm-11/llvm/IR/InstrTypes.h>
    #include <llvm-11/llvm/IR/Instruction.h>
    #include <llvm-11/llvm/IR/LLVMContext.h>
    #include <llvm-11/llvm/IR/LegacyPassManager.h>
    #include <llvm-11/llvm/IR/Module.h>
    #include <llvm-11/llvm/IRReader/IRReader.h>
    #include <llvm-11/llvm/Support/SourceMgr.h>
    #include <llvm-11/llvm/Support/TargetRegistry.h>
    #include <llvm-11/llvm/Support/TargetSelect.h>
    #include <llvm-11/llvm/Support/raw_os_ostream.h>
    #include <llvm-11/llvm/Transforms/InstCombine/InstCombine.h>
    #include <llvm-11/llvm/Transforms/Scalar.h>
    #include <llvm-11/llvm/Transforms/Scalar/GVN.h>
    #include <llvm-11/llvm/Analysis/CFGPrinter.h>
    #include <llvm-11/llvm/Analysis/CallPrinter.h>
    #include <llvm-11/llvm/Analysis/DomPrinter.h>
    #include <llvm-11/llvm/Analysis/CallGraph.h>
    #include <llvm-11/llvm/Analysis/DOTGraphTraitsPass.h>
    #include <llvm-11/llvm/Support/GraphWriter.h>
    #include <llvm-11/llvm/AsmParser/Parser.h>
#endif

#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stc/stc.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

#include <memory>
#include <string>
#include <vector>
#include <sstream>

// region Optimization descriptions
static std::vector<std::pair<std::string, std::string>> g_optimization_descriptions = {
    {"ConstantPropagation", "ConstantPropagation - A worklist driven constant propagation pass"},
    {"AlignmentFromAssumptions",
     "AlignmentFromAssumptions - Use assume intrinsics to set load/store alignments."},
    {"SCCP", "SCCP - Sparse conditional constant propagation."},
    {"DeadInstElimination", "DeadInstElimination - This pass quickly removes trivially dead instructions "
                            "without modifying the CFG of the function.  It is a FunctionPass."},
    {"RedundantDbgInstElimination",
     "RedundantDbgInstElimination - This pass removes redundant dbg intrinsics without modifying the CFG of "
     "the function.  It is a FunctionPass."},
    {"DeadCodeElimination",
     "DeadCodeElimination - This pass is more powerful than DeadInstElimination, because it is worklist "
     "driven that can potentially revisit instructions when their other instructions become dead, to "
     "eliminate chains of dead computations."},
    {"DeadStoreElimination", "DeadStoreElimination - This pass deletes stores that are post-dominated by "
                             "must-aliased stores and are not loaded used between the stores."},
    {"CallSiteSplitting",
     "CallSiteSplitting - This pass split call-site based on its known argument values."},
    {"AggressiveDCE", "AggressiveDCE - This pass uses the SSA based Aggressive DCE algorithm.  This "
                      "algorithm assumes instructions are dead until proven otherwise, which makes it more "
                      "successful are removing non-obviously dead instructions."},
    {"GuardWidening", "GuardWidening - An optimization over the @llvm.experimental.guard intrinsic that "
                      "(optimistically) combines multiple guards into one to have fewer checks at runtime."},
//    {"LoopGuardWidening",
//     "LoopGuardWidening - Analogous to the GuardWidening pass, but restricted to a single loop at a time for "
//     "use within a LoopPassManager.  Desired effect is to widen guards into preheader or a single guard "
//     "within loop if that's not possible."},
    {"BitTrackingDCE", "BitTrackingDCE - This pass uses a bit-tracking DCE algorithm in order to remove "
                       "computations of dead bits."},
    {"SROA", "SROA - Replace aggregates or pieces of aggregates with scalar SSA values."},
    {"InductiveRangeCheckElimination", "InductiveRangeCheckElimination - Transform loops to elide range "
                                       "checks on linear functions of the induction variable."},
    {"IndVarSimplify", "InductionVariableSimplify - Transform induction variables in a program to all use a "
                       "single canonical induction variable per loop."},
    {"LICM", "LICM - This pass is a loop invariant code motion and memory promotion pass."},
    {"LoopSink", "LoopSink - This pass sinks invariants from preheader to loop body where frequency is lower "
                 "than loop preheader."},
    {"LoopPredication", "LoopPredication - This pass does loop predication on guards."},
    {"LoopInterchange", "LoopInterchange - This pass interchanges loops to provide a more cache-friendly "
                        "memory access patterns."},
    {"LoopStrengthReduce", "LoopStrengthReduce - This pass is strength reduces GEP instructions that use a "
                           "loop's canonical induction variable as one of their indices."},
    {"LoopUnswitch", "LoopUnswitch - This pass is a simple loop unswitching pass."},
    {"LoopInstSimplify", "LoopInstSimplify - This pass simplifies instructions in a loop's body."},
    {"LoopUnroll", "LoopUnroll - This pass is a simple loop unrolling pass."},
    {"SimpleLoopUnroll", "Create an unrolling pass for full unrolling that uses exact trip count only."},
    {"LoopUnrollAndJam", "LoopUnrollAndJam - This pass is a simple loop unroll and jam pass."},
    {"LoopReroll", "LoopReroll - This pass is a simple loop rerolling pass."},
    {"LoopRotate", "LoopRotate - This pass is a simple loop rotating pass."},
    {"LoopIdiom", "LoopIdiom - This pass recognizes and replaces idioms in loops."},
    {"LoopVersioningLICM", "LoopVersioningLICM - This pass is a loop versioning pass for LICM."},
    {"DemoteRegisterToMemory",
     "DemoteRegisterToMemoryPass - This pass is used to demote registers to memory references. In basically "
     "undoes the PromoteMemoryToRegister pass to make cfg hacking easier."},
    {"Reassociate",
     "Reassociate - This pass reassociates commutative expressions in an order that is designed to promote "
     "better constant propagation, GCSE, LICM, PRE... For example:  4 + (x + 5)  ->  x + (4 + 5)"},
    {"JumpThreading",
     "JumpThreading - Thread control through mult-pred/multi-succ blocks where some preds always go to some "
     "succ. Thresholds other than minus one override the internal BB duplication default threshold."},
    {"CFGSimplification", "CFGSimplification - Merge basic blocks, eliminate unreachable blocks, simplify "
                          "terminator instructions, convert switches to lookup tables, etc."},
    {"FlattenCFG", "FlattenCFG - flatten CFG, reduce number of conditional branches by using parallel-and "
                   "and parallel-or mode, etc..."},
    {"StructurizeCFG",
     "CFG Structurization - Remove irreducible control flow When SkipUniformRegions is true the "
     "structizer will not structurize/ regions that only contain uniform branches."},
    {"TailCallElimination", "TailCallElimination - This pass eliminates call instructions to the current "
                            "function which occur immediately before return instructions."},
    {"EarlyCSE", "EarlyCSE - This pass performs a simple and fast CSE pass over the dominator tree."},
    {"GVNHoist", "GVNHoist - This pass performs a simple and fast GVN pass over the dominator tree to hoist "
                 "common expressions from sibling branches."},
    {"GVNSink", "GVNSink - This pass uses an 'inverted' value numbering to decide the similarity of "
                "expressions and sinks similar expressions into successors."},
    {"MergedLoadStoreMotion", "MergedLoadStoreMotion - This pass merges loads and stores in diamonds. Loads "
                              "are hoisted into the lesson_header, while stores sink into the footer."},
    {"NewGVN",
     "GVN - This pass performs global value numbering and redundant load elimination cotemporaneously."},
    {"DivRemPairs", "DivRemPairs - Hoist/decompose integer division and remainder instructions."},
    {"MemCpyOpt", "MemCpyOpt - This pass performs optimizations related to eliminating memcpy calls and/or "
                  "combining multiple stores into memset's."},
    {"LoopDeletion",
     "LoopDeletion - This pass performs DCE of non-infinite loops that it can prove are dead."},
    {"ConstantHoisting", "ConstantHoisting - This pass prepares a function for expensive constants."},
    {"Sinking", "Sink - Code Sinking"},
    {"LowerAtomic", "LowerAtomic - Lower atomic intrinsics to non-atomic form"},
    {"LowerGuardIntrinsic", "LowerGuardIntrinsic - Lower guard intrinsics to normal control flow."},
    {"LowerMatrixIntrinsics", "LowerMatrixIntrinsics - Lower matrix intrinsics to vector operations."},
    {"LowerWidenableCondition", "LowerWidenableCondition - Lower widenable condition to i1 true."},
    {"MergeICmpsLegacy", "MergeICmps - Merge integer comparison chains into a memcmp"},
    {"CorrelatedValuePropagation", "ValuePropagation - Propagate CFG-derived value information"},
    {"InferAddressSpaces",
     "InferAddressSpaces - Modify users of addrspacecast instructions with values in the source address "
     "space if using the destination address space is slower on the target. If AddressSpace is left to its "
     "default value, it will be obtained from the TargetTransformInfo."},
    {"LowerExpectIntrinsic",
     "LowerExpectIntrinsics - Removes llvm.expect intrinsics and creates 'block_weights' metadata."},
    {"LowerConstantIntrinsics", "LowerConstantIntrinsicss - Expand any remaining llvm.objectsize and "
                                "llvm.is.constant intrinsic calls, even for the unknown cases."},
    {"PartiallyInlineLibCalls",
     "PartiallyInlineLibCalls - Tries to inline the fast path of library calls such as sqrt."},
    {"SeparateConstOffsetFromGEP", "SeparateConstOffsetFromGEP - Split GEPs for better CSE"},
//    {"SpeculativeExecution", "SpeculativeExecution - Aggressively hoist instructions to enable speculative "
//                             "execution on targets where branches are expensive."},
//    {"SpeculativeExecutionIfHasBranchDivergence",
//     "Same as createSpeculativeExecutionPass, but does nothing unless "
//     "TargetTransformInfo::hasBranchDivergence() is true."},
    {"StraightLineStrengthReduce", "StraightLineStrengthReduce - This pass strength-reduces some certain "
                                   "instruction patterns in straight-line code."},
    {"PlaceSafepoints", "PlaceSafepoints - Rewrite any IR calls to gc.statepoints and insert any safepoint "
                        "polls (method entry, backedge) that might be required.  This pass does not generate "
                        "explicit relocation sequences - that's handled by RewriteStatepointsForGC which can "
                        "be run at an arbitrary point in the pass order following this pass."},
//    {"RewriteStatepointsForGCLegacy", "RewriteStatepointsForGC - Rewrite any gc.statepoints which do not yet "
//                                      "have explicit relocations to include explicit relocations."},
    {"Float2Int", "Float2Int - Demote floats to ints where possible."},
    {"NaryReassociate", "NaryReassociate - Simplify n-ary operations by reassociation."},
    {"LoopDistribute", "LoopDistribute - Distribute loops."},
    {"LoopFuse", "LoopFuse - Fuse loops."},
    {"LoopLoadElimination", "LoopLoadElimination - Perform loop-aware load elimination."},
    {"LoopVersioning", "LoopVersioning - Perform loop multi-versioning."},
    {"LoopDataPrefetch", "LoopDataPrefetch - Perform data prefetching in loops."},
    {"LibCallsShrinkWrap", "LibCallsShrinkWrap - Shrink-wraps a call to function if the result is not used."},
    {"LoopSimplifyCFG", "LoopSimplifyCFG - This pass performs basic CFG simplification on loops, primarily "
                        "to help other loop passes."},
    {"WarnMissedTransformations",
     "WarnMissedTransformations - This pass emits warnings for leftover forced transformations."},
};
// endregion

enum {
    // menu IDs
    justOffset = 10000,
    OPTIMIZATION_CHECK_BOX,
    OPTIMIZATION_DESCRIPTION,
    OPT_BUTTON,
};

class LLVMOptimizationWindow : public wxDialog {
  public:
    LLVMOptimizationWindow(wxFrame *frame, const wxString &title, wxString code);
    ~LLVMOptimizationWindow() = default;

    void OnClose(wxCloseEvent &event);
    void OnQuit(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
    void OnButtonPressed(wxCommandEvent &event);
    void OnCheckboxClicked(wxCommandEvent &event);

    void parse_and_optimize_code();

    wxString output_code;

  private:
    wxCheckListBox *m_optimizations;
    wxStaticText  *m_optimization_description;
    wxButton *m_button;
    wxString m_program_code;

    wxBoxSizer *m_vbox;
    wxBoxSizer *m_hbox;

    DECLARE_EVENT_TABLE()
};

#endif   // TAC_SIMULATOR_TEST_OPTIMIZATIONSELECT_HPP
