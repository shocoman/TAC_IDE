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
#include <wx/chartype.h>

#include <memory>
#include <string>
#include <vector>
#include <sstream>

// region Optimization descriptions
static std::vector<std::pair<wxString, wxString>> g_optimization_descriptions = {
    {wxT("ConstantPropagation"), wxT("ConstantPropagation - это оптимизационный проход, отвечающий за распространение констант (замену переменных на их значения).")},
    {wxT("AlignmentFromAssumptions" ), wxT("AlignmentFromAssumptions - Использование предполагаемых интринсиков для загрузки/записи в память.")},
    {wxT("SCCP" ), wxT("SCCP - Разрежённое условное распространение констант.")},
    {wxT("DeadInstElimination" ), wxT("DeadInstElimination - Удаление ненужных (мёртвых) инструкций без модификации графа потока управления функции.")},
    {wxT("RedundantDbgInstElimination" ), wxT("RedundantDbgInstElimination - удаление отладочных инструкций без изменения графа потока управления функции.")},
    {wxT("DeadCodeElimination" ), wxT("DeadCodeElimination -  Данный проход это улучшенная версия 'DeadInstElimination', поскольку может повторно посещать инструкции, после того как их операнды оказались ненужными. Это позволяет устранить целую цепь бесполезных инструкций за один проход.")},
    {wxT("DeadStoreElimination" ), wxT("DeadStoreElimination - Данный проход удаляет ненужные записи в память, если известно, что записанное значение в программе использоваться не будет.")},
    {wxT("CallSiteSplitting" ), wxT("CallSiteSplitting - Проход разделяет место вызова функции в зависимости от известных значений аргументов.")},
    {wxT("AggressiveDCE" ), wxT("AggressiveDCE - Агрессивная версия DeadCodeElimination, основанная на SSA-форме. Данный алгоритм подразумевает, что инструкции не нужны, пока не доказано обратное. Это позволяет удалять неочевидные, но бесполезные инструкции.")},
    {wxT("BitTrackingDCE" ), wxT("BitTrackingDCE - Проход использует версию алгоритма DeadCodeElimination для отслеживания битов, чтобы убрать в программе их ненужные вычисления.")},
    {wxT("SROA" ), wxT("SROA - Замена агрегатов или частей агрегатов на скалярные SSA значения.")},
    {wxT("InductiveRangeCheckElimination" ), wxT("InductiveRangeCheckElimination - В циклах убирает проверки на выход за пределы массива в линейных функциях с индукционными переменными.")},
    {wxT("IndVarSimplify" ), wxT("InductionVariableSimplify - Преобразовывает индукционные переменные в программе в одну индукционную на цикл.")},
    {wxT("LICM" ), wxT("LICM - Вынесение кода за пределы циклов.")},
    {wxT("LoopSink" ), wxT("LoopSink - Перенос инвариантов из заголовка цикла в его тело.")},
    {wxT("LoopInterchange" ), wxT("LoopInterchange - Данный проход меняет вложенные циклы местами для ускорения доступа благодаря процессорному кешированию")},
    {wxT("LoopStrengthReduce" ), wxT("LoopStrengthReduce - Снижение стоимости операций внутри цикла с индукционной переменной.")},
    {wxT("LoopUnswitch" ), wxT("LoopUnswitch - Размыкание цикла, то есть вынесение условных интсрукций (if) за пределы цикла для возможного его распараллеливания.")},
    {wxT("LoopInstSimplify" ), wxT("LoopInstSimplify - Упрощение инструкций в теле цикла.")},
    {wxT("LoopUnroll" ), wxT("LoopUnroll - Разворачивание циклов.")},
    {wxT("SimpleLoopUnroll" ), wxT("Разворачивание цикла равно столько раз, сколько будет выполняться цикл.")},
    {wxT("LoopUnrollAndJam" ), wxT("LoopUnrollAndJam - Разворачивание и последующее сжатие цикла.")},
    {wxT("LoopReroll" ), wxT("LoopReroll - Повторное разворачивание нескольких циклов.")},
    {wxT("LoopRotate" ), wxT("LoopRotate - Поворот цикла.")},
    {wxT("LoopIdiom" ), wxT("LoopIdiom - Замена идиом в цикле.")},
    {wxT("DemoteRegisterToMemory" ), wxT("DemoteRegisterToMemoryPass - Замена работы с регистрами на работу с ссылками в памяти.")},
    {wxT("Reassociate" ), wxT("Reassociate - Данный проход меняет порядок коммутативных выражений для дальнейшего облегчения распространения констант и других процедур. Например:  4 + (x + 5)  ->  x + (4 + 5)")},
    {wxT("CFGSimplification" ), wxT("CFGSimplification - Слияние базовых блоков, устранение недостижимых базовых блоков, упрощение терминальных инструкций, конвертация инструкции 'switch' в простой просмотр таблицы и так далее.")},
    {wxT("FlattenCFG" ), wxT("FlattenCFG - Сплющивание графа потока управления, уменьшение количества условных переходов с помощью режимов распараллеливания и т.д.")},
    {wxT("StructurizeCFG" ), wxT("CFG Structurization -  Устраниение 'неприводимости' графа.")},
    {wxT("TailCallElimination" ), wxT("TailCallElimination - Устранение хвостовой рекурсии.")},
    {wxT("EarlyCSE" ), wxT("EarlyCSE - Удаление общих подвыражений на дереве доминаторов.")},
    {wxT("GVNHoist" ), wxT("GVNHoist - Проход глобальной нумерации значений, выполняемый на дереве доминаторов, чтобы поднять общие подвыражения вверх по графу.")},
    {wxT("GVNSink" ), wxT("GVNSink - Этот проход реализует обратную нумерацию значений, чтобы определить похожие выражения и вынести их вних по графу.")},
    {wxT("NewGVN" ), wxT("GVN - Глобальная нумерация значений и устранение ненужных доступов в память одновременно.")},
    {wxT("DivRemPairs" ), wxT("DivRemPairs - Декомпозиция инструкций для целочисленного деления и взятия остатка.")},
    {wxT("MemCpyOpt" ), wxT("MemCpyOpt -  Устанение вызовов к 'memcpy' или комбинация нескольких 'memcpy' в один.")},
    {wxT("LoopDeletion" ), wxT("LoopDeletion - Устранение небесконечных циклов, когда известно, что они ненужны")},
    {wxT("ConstantHoisting" ), wxT("ConstantHoisting - Вынесение дорогостоящих констант вверх по графу.")},
    {wxT("LowerAtomic" ), wxT("LowerAtomic - Понижение атомичных (потокобезопасных) интринсиков до не-атомичной формы")},
    {wxT("LowerMatrixIntrinsics" ), wxT("LowerMatrixIntrinsics - Понижение матричных интринсиков до векторных операций.")},
    {wxT("MergeICmpsLegacy" ), wxT("MergeICmps - Слияние серий целочисленных сравненений в 'memcmp'.")},
    {wxT("CorrelatedValuePropagation" ), wxT("ValuePropagation - Распространение информации для значений, полученных из графа.")},
    {wxT("LowerExpectIntrinsic" ), wxT("LowerExpectIntrinsics - Удаление llvm.expect интринсиков и создание метаданных 'block_weights'.")},
    {wxT("LowerConstantIntrinsics" ), wxT("LowerConstantIntrinsicss - Распространение оставшихся интринсик-вызовов llvm.objectsize и llvm.is.constant.")},
    {wxT("PartiallyInlineLibCalls" ), wxT("PartiallyInlineLibCalls - Инлайнинг вызовов к стандартной библиотеке, например 'sqrt'.")},
    {wxT("StraightLineStrengthReduce" ), wxT("StraightLineStrengthReduce - Уменьшение стоимости операций некоторых в линейном коде.")},
    {wxT("Float2Int" ), wxT("Float2Int - Конвертация чисел с плавающей запятой в целочисленные, где это возможно.")},
    {wxT("NaryReassociate" ), wxT("NaryReassociate - Упрощение n-арных операций перемещением.")},
    {wxT("LoopDistribute" ), wxT("LoopDistribute - Дистрибьюция циклов.")},
    {wxT("LoopFuse" ), wxT("LoopFuse - Склеивание циклов.")},
    {wxT("LoopLoadElimination" ), wxT("LoopLoadElimination - Устранение загрузок из памяти с учётом циклов.")},
    {wxT("LoopSimplifyCFG" ), wxT("LoopSimplifyCFG - Данный прохож выполняет базовые упрощения графа потока управления, в основном, чтобы упростить задачу остальным проходам.")},
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
