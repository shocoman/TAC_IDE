cmake_minimum_required(VERSION 3.16.3)
project(toy_optimizer CXX)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE RELEASE)

#set(CMAKE_CXX_FLAGS_DEBUG "-O1")
#set(CMAKE_CXX_FLAGS_DEBUG "-O3")


### My Program ###
set(SOURCE_FILES src/data_flow_analyses/reaching_definitions.hpp src/utilities/graph_writer/graph_writer.cpp src/structure/basic_block.cpp src/structure/function.cpp src/utilities/quad_preparation.cpp src/optimizations/ssa.cpp src/optimizations/ssa.hpp src/optimizations/value_numbering.cpp src/optimizations/useless_code_elimination.cpp src/optimizations/sparse_simple_constant_propagation.cpp src/data_flow_analyses/data_flow_analyses.cpp src/data_flow_analyses/dominators.cpp src/data_flow_analyses/expressions_analyses/utilities.cpp src/data_flow_analyses/expressions_analyses/earliest_expressions.cpp src/data_flow_analyses/expressions_analyses/earliest_expressions.hpp src/data_flow_analyses/data_flow_framework.hpp src/optimizations/lazy_code_motion.cpp src/optimizations/lazy_code_motion.hpp src/optimizations/sparse_conditional_constant_propagation.cpp src/optimizations/sparse_conditional_constant_propagation.hpp src/optimizations/operator_strength_reduction.cpp src/optimizations/operator_strength_reduction.hpp src/optimizations/copy_propagation.cpp src/optimizations/copy_propagation.hpp src/data_flow_analyses/ssa_graph.cpp src/data_flow_analyses/ssa_graph.hpp src/data_flow_analyses/print_graph.cpp src/data_flow_analyses/print_graph.hpp src/data_flow_analyses/use_def_graph.cpp src/data_flow_analyses/use_def_graph.hpp src/utilities/new_name_generator.cpp src/utilities/new_name_generator.hpp src/structure/program.cpp src/structure/program.hpp src/data_flow_analyses/reaching_definitions.hpp src/data_flow_analyses/live_variable_analysis.hpp src/all_headers.hpp src/optimizations/constant_folding.cpp src/optimizations/constant_folding.hpp src/data_flow_analyses/critical_edges.cpp src/data_flow_analyses/critical_edges.hpp)

# Source paths for export
get_directory_property(hasParent PARENT_DIRECTORY)
if (hasParent)
    set(TOY_OPTIMIZER_SOURCE_FILES ${SOURCE_FILES} PARENT_SCOPE)
endif ()


### {fmt}: Library for text formatting ###
add_subdirectory(lib/fmt EXCLUDE_FROM_ALL)


### Creating exe and linking ###
add_executable(toy_optimizer main.cpp ${SOURCE_FILES})
target_include_directories(toy_optimizer PUBLIC src lib)
target_compile_definitions(toy_optimizer PRIVATE DISPLAY_GRAPHS)


### Graphviz ###
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_plugins")
find_package(Graphviz)
target_include_directories(toy_optimizer PUBLIC ${graphviz_INCLUDE_DIRECTORIES})
target_include_directories(toy_optimizer PUBLIC src/utilities/parser)
target_link_libraries(toy_optimizer PRIVATE ${graphviz_LIBRARIES})


### Flex/Bison ###
if (WIN32)
    set(BISON_COMPILE_FLAGS, "-Wcounterexamples")
    set(FLEX_COMPILE_FLAGS, "--wincompat")
endif ()
find_package(BISON)
find_package(FLEX)
file(MAKE_DIRECTORY src/utilities/parser/compiled_parser)
BISON_TARGET(MyParser src/utilities/parser/grammar/parser.y ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities/parser/compiled_parser/parser.cpp COMPILE_FLAGS ${BISON_COMPILE_FLAGS})
FLEX_TARGET(MyScanner src/utilities/parser/grammar/lexer.l ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities/parser/compiled_parser/lexer.cpp COMPILE_FLAGS ${FLEX_COMPILE_FLAGS})
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

add_library(Parser SHARED ${BISON_MyParser_OUTPUTS} ${FLEX_MyScanner_OUTPUTS})
target_sources(Parser PUBLIC
        src/utilities/parser/driver/driver.cpp
        src/utilities/parser/compiled_parser/lexer.cpp
        src/utilities/parser/compiled_parser/parser.cpp
        )
target_include_directories(Parser PUBLIC src/utilities/parser)
target_link_libraries(toy_optimizer PRIVATE Parser fmt::fmt)

