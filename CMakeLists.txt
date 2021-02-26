cmake_minimum_required(VERSION 3.17)
project(tac_parser CXX)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE RELEASE)

#set(CMAKE_CXX_FLAGS_DEBUG "-O1")
#set(CMAKE_CXX_FLAGS_DEBUG "-O3")


# My Program
set(SOURCE_FILES
        main.cpp
        src/graph_writer/graph_writer.cpp
        src/tac_worker/optimization_runner.cpp
        src/tac_worker/structure/basic_block.cpp   src/tac_worker/structure/function.cpp
        src/tac_worker/quad_preparation.cpp src/tac_worker/optimizations/ssa.cpp src/tac_worker/optimizations/ssa.hpp src/tac_worker/optimizations/value_numbering.cpp
        src/tac_worker/optimizations/useless_code_elimination.cpp
        src/tac_worker/optimizations/sparse_simple_constant_propagation.cpp
        src/tac_worker/optimizations/data_flow_analyses/data_flow_analyses.cpp
        src/tac_worker/optimizations/data_flow_analyses/dominators.cpp src/tac_worker/optimizations/data_flow_analyses/expressions_analyses/available_expressions.cpp src/tac_worker/optimizations/data_flow_analyses/expressions_analyses/available_expressions.hpp src/tac_worker/optimizations/data_flow_analyses/expressions_analyses/anticipable_expressions.cpp src/tac_worker/optimizations/data_flow_analyses/expressions_analyses/anticipable_expressions.hpp src/tac_worker/print_utility.hpp src/tac_worker/print_utility.cpp src/tac_worker/optimizations/data_flow_analyses/expressions_analyses/utilities.cpp src/tac_worker/optimizations/data_flow_analyses/expressions_analyses/earliest_expressions.cpp src/tac_worker/optimizations/data_flow_analyses/expressions_analyses/earliest_expressions.hpp src/tac_worker/optimizations/data_flow_analyses/data_flow_framework.cpp src/tac_worker/optimizations/data_flow_analyses/data_flow_framework.hpp src/tac_worker/optimizations/lazy_code_motion.cpp src/tac_worker/optimizations/lazy_code_motion.hpp src/tac_worker/optimizations/sparse_conditional_constant_propagation.cpp src/tac_worker/optimizations/sparse_conditional_constant_propagation.hpp src/tac_worker/optimizations/operator_strength_reduction.cpp src/tac_worker/optimizations/operator_strength_reduction.hpp src/tac_worker/optimizations/copy_propagation.cpp src/tac_worker/optimizations/copy_propagation.hpp)


add_subdirectory(src/fmt EXCLUDE_FROM_ALL)

### Creating exe and linking ###
add_executable(tac_parser ${SOURCE_FILES})
target_include_directories(tac_parser PUBLIC src)


### Graphviz ###
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_plugins")
find_package(Graphviz)
target_include_directories(tac_parser PUBLIC "${graphviz_INCLUDE_DIRECTORIES}")
target_include_directories(tac_parser PUBLIC src/parser)
target_link_libraries(tac_parser PRIVATE ${graphviz_LIBRARIES})


### Flex/Bison ###
if (WIN32)
    set(BISON_COMPILE_FLAGS, "-Wcounterexamples")
    set(FLEX_COMPILE_FLAGS, "--wincompat")
endif ()
find_package(BISON)
find_package(FLEX)
file(MAKE_DIRECTORY src/parser/compiled_parser)
BISON_TARGET(MyParser src/parser/grammar/parser.y ../src/parser/compiled_parser/parser.cpp COMPILE_FLAGS ${BISON_COMPILE_FLAGS})
FLEX_TARGET(MyScanner src/parser/grammar/lexer.l ../src/parser/compiled_parser/lexer.cpp COMPILE_FLAGS ${FLEX_COMPILE_FLAGS})
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

add_library(Parser SHARED ${BISON_MyParser_OUTPUTS} ${FLEX_MyScanner_OUTPUTS})
target_sources(Parser PUBLIC
        src/parser/driver/driver.cpp
        src/parser/compiled_parser/lexer.cpp
        src/parser/compiled_parser/parser.cpp
        )
target_include_directories(Parser PUBLIC src/parser)
target_link_libraries(tac_parser PRIVATE Parser fmt::fmt)
