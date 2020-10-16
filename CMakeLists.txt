cmake_minimum_required(VERSION 3.17)
project(tac_parser CXX)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS_DEBUG "-O3")

# My Program
set(SOURCE_FILES
        main.cpp
        DotWriter/DotWriter.cpp
        driver/driver.cpp
        tac_worker/dataflow_graph.cpp tac_worker/LoopFinder.cpp tac_worker/dag.h)

# Flex/Bison
find_package(BISON)
find_package(FLEX)
BISON_TARGET(MyParser grammar/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp COMPILE_FLAGS "-Wcounterexamples")
FLEX_TARGET(MyScanner grammar/lexer.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp COMPILE_FLAGS "--wincompat")
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

# Creating exe and linking
add_executable(tac_parser ${SOURCE_FILES} ${BISON_MyParser_OUTPUTS} ${FLEX_MyScanner_OUTPUTS})
target_include_directories(tac_parser PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Graphviz
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(graphviz_INCLUDE_DIRECTORIES "C:/Program Files/Graphviz 2.44.1/include/graphviz/")
find_package(Graphviz)
target_include_directories(tac_parser PUBLIC "${graphviz_INCLUDE_DIRECTORIES}")
target_link_libraries(tac_parser ${graphviz_LIBRARIES})



