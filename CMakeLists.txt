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
        src/tac_worker/optimization_runner.cpp src/_garbage_bin/loop_finder.cpp src/tac_worker/optimizations/value_numbering.hpp
        src/tac_worker/structure/basic_block.cpp src/tac_worker/structure/basic_block.hpp src/tac_worker/structure/function.cpp src/tac_worker/structure/function.hpp
        src/tac_worker/quad_preparation.cpp src/tac_worker/optimizations/ssa.cpp src/tac_worker/optimizations/ssa.hpp src/tac_worker/optimizations/value_numbering.cpp)


# Creating exe and linking
add_executable(tac_parser ${SOURCE_FILES})
target_include_directories(tac_parser PUBLIC src)
target_precompile_headers(tac_parser PRIVATE
        src/graph_writer/graph_writer.hpp
        src/tac_worker/optimization_runner.hpp
        src/tac_worker/quad_preparation.hpp
        src/tac_worker/structure/basic_block.hpp
        src/tac_worker/structure/function.hpp
        src/tac_worker/structure/quadruple/destination.hpp
        src/tac_worker/structure/quadruple/operand.hpp
        src/tac_worker/structure/quadruple/quadruple.hpp
        )


# Graphviz
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(Graphviz)
target_include_directories(tac_parser PUBLIC "${graphviz_INCLUDE_DIRECTORIES}")
target_include_directories(tac_parser PUBLIC src/parser)
target_link_libraries(tac_parser PRIVATE ${graphviz_LIBRARIES})


# Flex/Bison
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
target_link_libraries(tac_parser PRIVATE Parser)
