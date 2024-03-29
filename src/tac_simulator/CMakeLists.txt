cmake_minimum_required(VERSION 3.16.3)
project(tac_simulator)
set(CMAKE_CXX_STANDARD 17)


### FLEX / BISON ########################################
if (WIN32)
    set(FLEX_COMPILE_FLAGS, "--wincompat")
    set(BISON_COMPILE_FLAGS, "-Wcounterexamples")
endif ()
find_package(BISON)
find_package(FLEX)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/flex_bison)
BISON_TARGET(MyParser language.yy ${CMAKE_CURRENT_BINARY_DIR}/flex_bison/language.tab.cpp COMPILE_FLAGS ${BISON_COMPILE_FLAGS})
FLEX_TARGET(MyScanner lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/flex_bison/lex.yy.cpp COMPILE_FLAGS ${FLEX_COMPILE_FLAGS})
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)


### TAC SIMULATOR ########################################
set(TAC_SIMULATOR_SOURCES tac_project.cpp tac_tree.cpp interpreter.cpp label_table.cpp semantic_analysis.cpp symbol_table.cpp
        ${BISON_MyParser_OUTPUTS} ${FLEX_MyScanner_OUTPUTS} main.cpp)
add_executable(tac_simulator ${TAC_SIMULATOR_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/flex_bison)
target_include_directories(tac_simulator PUBLIC ./)
