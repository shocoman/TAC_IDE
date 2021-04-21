//
// Created by shoco on 2/1/2021.
//

#include "llvm_simulator.hpp"


#include <fstream>
#include <iostream>
#include <memory>

#include <llvm-11/llvm/IR/Constants.h>
#include <llvm-11/llvm/IR/Function.h>
#include <llvm-11/llvm/IR/IRBuilder.h>
#include <llvm-11/llvm/IR/InstrTypes.h>
#include <llvm-11/llvm/IR/Instruction.h>
#include <llvm-11/llvm/IR/LLVMContext.h>
#include <llvm-11/llvm/IR/LegacyPassManager.h>
#include <llvm-11/llvm/IR/Module.h>
#include <llvm-11/llvm/IRReader/IRReader.h>
#include <llvm-11/llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm-11/llvm/ExecutionEngine/GenericValue.h>
#include <llvm-11/llvm/ExecutionEngine/MCJIT.h>
#include <llvm-11/llvm/Support/SourceMgr.h>
#include <llvm-11/llvm/Support/TargetRegistry.h>
#include <llvm-11/llvm/Support/TargetSelect.h>
#include <llvm-11/llvm/Support/raw_os_ostream.h>
#include <llvm-11/llvm/Bitcode/BitcodeWriter.h>
#include <llvm-11/llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm-11/llvm/Transforms/Scalar.h>
#include <llvm-11/llvm/Transforms/Scalar/GVN.h>
#include <llvm-11/llvm/Transforms/Scalar/GVN.h>


llvm::LLVMContext g_context;
std::unique_ptr<llvm::Module> g_module = std::make_unique<llvm::Module>("Main Module", g_context);
llvm::IRBuilder<> g_builder(g_context);
std::unique_ptr<llvm::legacy::FunctionPassManager> g_function_pass_manager;

void buildModule() {
    /* Create main function */
    llvm::FunctionType *funcType = llvm::FunctionType::get(g_builder.getInt32Ty(), false);
    llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", g_module.get());
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(g_context, "entrypoint", mainFunc);
    g_builder.SetInsertPoint(entry);

    /* String constant */
    llvm::Value *helloWorldStr = g_builder.CreateGlobalStringPtr("hello world!\n");

    /* Create "puts" function */
    std::vector<llvm::Type *> putsArgs;
    putsArgs.push_back(g_builder.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type *> argsRef(putsArgs);
    llvm::FunctionType *putsType = llvm::FunctionType::get(g_builder.getInt32Ty(), argsRef, false);
    auto putsFunc = g_module->getOrInsertFunction("puts", putsType);

    g_builder.CreateCall(putsFunc, helloWorldStr);
    g_builder.CreateRet(llvm::ConstantInt::get(g_context, llvm::APInt(32, 10)));
}


void run_main_function() {
    // init MCJIT
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    auto main_func = g_module->getFunction("main");

    std::string error_str;
    llvm::ExecutionEngine *engine = llvm::EngineBuilder(std::move(g_module)).setErrorStr(&error_str).create();
    engine->finalizeObject();

    std::cout << "--> Simulator started..." << std::endl;
    std::vector<llvm::GenericValue> args;
    llvm::GenericValue return_value = engine->runFunction(main_func, args);

    std::cout << "--> Simulator finished. Function return value: ";
    auto return_type = main_func->getReturnType();
    if (return_type->isIntegerTy()) {
        std::cout << '\"' << return_value.IntVal.getSExtValue() << '\"' << std::endl;
    } else if (return_type->isDoubleTy()) {
        std::cout << '\"' << return_value.DoubleVal << '\"' << std::endl;
    }
}

void parse_ir_file(std::string file_name) {
    llvm::SMDiagnostic err;
    auto mod = llvm::parseIRFile(file_name, err, g_context);
    if (!mod) {
        err.print(file_name.c_str(), llvm::errs());
    }
    g_module.swap(mod);
}

void optimization() {
    g_function_pass_manager = std::make_unique<llvm::legacy::FunctionPassManager>(g_module.get());

    g_function_pass_manager->add(llvm::createInstructionCombiningPass());
    g_function_pass_manager->add(llvm::createReassociatePass());
    g_function_pass_manager->add(llvm::createGVNPass());
    g_function_pass_manager->add(llvm::createCFGSimplificationPass());
    g_function_pass_manager->add(llvm::createSROAPass());
    g_function_pass_manager->add(llvm::createConstantPropagationPass());
    g_function_pass_manager->add(llvm::createConstantHoistingPass());
    g_function_pass_manager->add(llvm::createDeadCodeEliminationPass());

    g_function_pass_manager->doInitialization();
    g_function_pass_manager->run(*g_module->getFunction("main"));
}

void print() {
    g_module->print(llvm::outs(), nullptr);
}

void print_to_file(std::string file_name) {
    std::ofstream std_file_stream(file_name);
    llvm::raw_os_ostream file_stream(std_file_stream);
    g_module->print(file_stream, nullptr);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Missing a file path with llvm-ir code!" << std::endl;
        return -1;
    }

    std::string file_name = argv[1];
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cout << "Couldn't open file '" << argv[1] << "'" << std::endl;
        return -1;
    }

    parse_ir_file(file_name);
    run_main_function();

//    buildModule();
//
//    optimization();
//
//    print();
//    print_to_file();
//    run_function();

    return 0;
}

