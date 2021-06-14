cmake_minimum_required(VERSION 3.16.3)
project(toy_ide)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_CXX_FLAGS_DEBUG "-O1")

#message("Folders: ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ")



set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(src/toy_optimizer EXCLUDE_FROM_ALL)
list(TRANSFORM TOY_OPTIMIZER_SOURCE_FILES PREPEND "src/toy_optimizer/")


### WxWidgets ########################################
find_package(wxWidgets COMPONENTS REQUIRED core base stc html  aui  #[[ gl net media ribbon xrc xml richtext propgrid ]] )
include(${wxWidgets_USE_FILE})
message("WX: ${wxWidgets_USE_FILE}")

#### Tac Simulator ###############################
add_subdirectory(src/tac_simulator)
set_target_properties(tac_simulator PROPERTIES
#        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_NAME tac_simulator_exe
        )
#add_executable(tac_simulator src/tac_simulator/tac_project.cpp)


#### LLVM ########################################
option(ENABLE_LLVM "Build project with LLVM if possible" ON)
find_package(LLVM CONFIG)
#message("Enable? : ${ENABLE_LLVM}")
IF (NOT ENABLE_LLVM)
    SET(LLVM_FOUND 0)
    SET(llvm_libs "")
ENDIF()
IF (${LLVM_FOUND})
    MESSAGE("Found LLVM")
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
    add_executable(llvm_simulator ${CMAKE_CURRENT_SOURCE_DIR}/src/llvm_simulator/llvm_simulator.cpp)
    llvm_map_components_to_libnames(llvm_libs support core irreader mcjit jitlink x86asmparser x86codegen)
    target_link_libraries(llvm_simulator ${llvm_libs})
ELSE ()
    MESSAGE("Could not locate LLVM. Skipping...")
ENDIF ()


### Flex/Bison ###
add_library(BisonParser SHARED ${BISON_MyParser_OUTPUTS} ${FLEX_MyScanner_OUTPUTS})
target_sources(BisonParser PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/toy_optimizer/src/utilities/parser/driver/driver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/toy_optimizer/src/utilities/parser/compiled_parser/lexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/toy_optimizer/src/utilities/parser/compiled_parser/parser.cpp
        )
target_include_directories(BisonParser PUBLIC ${CMAKE_SOURCE_DIR}/src/toy_optimizer/src/utilities/parser)


### GUI (main program) ########################################
set(EDITOR_SOURCES src/gui_code_editor/the_app.cpp
        src/gui_code_editor/windows/main_window/main_window.cpp
        src/gui_code_editor/windows/main_window/code_editor_control.cpp
        src/gui_code_editor/windows/optimization_window/llvm_ir/optimization_select.cpp
        src/gui_code_editor/windows/function_selection_window/function_chooser.cpp
        src/gui_code_editor/windows/image_viewer/convert_dot_to_image.cpp
        src/gui_code_editor/windows/image_viewer/image_panel.cpp
        src/gui_code_editor/windows/image_viewer/graph_view.cpp
        src/gui_code_editor/windows/optimization_window/toy_ir/toy_optimization_choose_window.cpp
        src/gui_code_editor/windows/optimization_window/toy_ir/my_html_window.cpp
        src/gui_code_editor/generated_gui_classes/my_generated_gui.cpp
        src/gui_code_editor/windows/optimization_window/toy_ir/toy_optimization_description_window.cpp
        src/gui_code_editor/windows/optimization_window/toy_ir/toy_optimization_description_window.hpp src/gui_code_editor/utilities/image_loading.cpp src/gui_code_editor/utilities/image_loading.hpp)
add_executable(toy_ide main.cpp ${EDITOR_SOURCES} ${TOY_OPTIMIZER_SOURCE_FILES})
target_include_directories(toy_ide PUBLIC src src/gui_code_editor)
target_link_libraries(toy_ide PUBLIC fmt::fmt BisonParser)


#### Graphviz ########################################
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/src/toy_optimizer/cmake_plugins")
find_package(Graphviz)
target_include_directories(toy_ide PUBLIC "${graphviz_INCLUDE_DIRECTORIES}")

target_link_libraries(toy_ide PUBLIC ${wxWidgets_LIBRARIES} ${llvm_libs} ${graphviz_LIBRARIES})
target_compile_definitions(toy_ide PRIVATE LLVM_FOUND=${LLVM_FOUND})


# move assets to a folder with an executable file
file(COPY _Examples _Tutorial DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
