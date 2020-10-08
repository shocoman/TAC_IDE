cmake_minimum_required(VERSION 3.17)
project(tac_parser CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES main.cpp DotWriter/DotWriter.cpp DotWriter/DotWriter.h)
add_library(Driver driver/driver.cpp driver/driver.hpp )
add_library(TacWorker tac_worker/source.cpp tac_worker/dataflow_graph.hpp tac_worker/quadruple.hpp)
target_include_directories(Driver PUBLIC driver)
target_include_directories(TacWorker PUBLIC tac_worker)


find_package(BISON)
find_package(FLEX)
BISON_TARGET(MyParser grammar/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp COMPILE_FLAGS "-Wcounterexamples")
FLEX_TARGET(MyScanner grammar/lexer.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp COMPILE_FLAGS "--wincompat")
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(tac_parser
        ${SOURCE_FILES}
        ${BISON_MyParser_OUTPUTS}
        ${FLEX_MyScanner_OUTPUTS}
        )
target_link_libraries(tac_parser Driver)
target_link_libraries(tac_parser TacWorker)

