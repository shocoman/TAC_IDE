# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2020.1.2\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2020.1.2\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = D:\programming\c\tac_parser

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = D:\programming\c\tac_parser\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/tac_parser.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/tac_parser.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tac_parser.dir/flags.make

parser.cpp: ../grammar/parser.y
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "[BISON][MyParser] Building parser with bison 3.7.1"
	D:\programming\_mingw730_64\win_flex_bison-2.5.23\bison.exe -Wcounterexamples -d -o D:/programming/c/tac_parser/cmake-build-debug/parser.cpp D:/programming/c/tac_parser/grammar/parser.y

parser.hpp: parser.cpp
	@$(CMAKE_COMMAND) -E touch_nocreate parser.hpp

lexer.cpp: ../grammar/lexer.l
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "[FLEX][MyScanner] Building scanner with flex 2.6.4"
	D:\programming\_mingw730_64\win_flex_bison-2.5.23\flex.exe --wincompat -oD:/programming/c/tac_parser/cmake-build-debug/lexer.cpp D:/programming/c/tac_parser/grammar/lexer.l

CMakeFiles/tac_parser.dir/main.cpp.obj: CMakeFiles/tac_parser.dir/flags.make
CMakeFiles/tac_parser.dir/main.cpp.obj: CMakeFiles/tac_parser.dir/includes_CXX.rsp
CMakeFiles/tac_parser.dir/main.cpp.obj: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/tac_parser.dir/main.cpp.obj"
	D:\programming\_mingw730_64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\tac_parser.dir\main.cpp.obj -c D:\programming\c\tac_parser\main.cpp

CMakeFiles/tac_parser.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tac_parser.dir/main.cpp.i"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\programming\c\tac_parser\main.cpp > CMakeFiles\tac_parser.dir\main.cpp.i

CMakeFiles/tac_parser.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tac_parser.dir/main.cpp.s"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\programming\c\tac_parser\main.cpp -o CMakeFiles\tac_parser.dir\main.cpp.s

CMakeFiles/tac_parser.dir/DotWriter/DotWriter.cpp.obj: CMakeFiles/tac_parser.dir/flags.make
CMakeFiles/tac_parser.dir/DotWriter/DotWriter.cpp.obj: CMakeFiles/tac_parser.dir/includes_CXX.rsp
CMakeFiles/tac_parser.dir/DotWriter/DotWriter.cpp.obj: ../DotWriter/DotWriter.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/tac_parser.dir/DotWriter/DotWriter.cpp.obj"
	D:\programming\_mingw730_64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\tac_parser.dir\DotWriter\DotWriter.cpp.obj -c D:\programming\c\tac_parser\DotWriter\DotWriter.cpp

CMakeFiles/tac_parser.dir/DotWriter/DotWriter.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tac_parser.dir/DotWriter/DotWriter.cpp.i"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\programming\c\tac_parser\DotWriter\DotWriter.cpp > CMakeFiles\tac_parser.dir\DotWriter\DotWriter.cpp.i

CMakeFiles/tac_parser.dir/DotWriter/DotWriter.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tac_parser.dir/DotWriter/DotWriter.cpp.s"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\programming\c\tac_parser\DotWriter\DotWriter.cpp -o CMakeFiles\tac_parser.dir\DotWriter\DotWriter.cpp.s

CMakeFiles/tac_parser.dir/driver/driver.cpp.obj: CMakeFiles/tac_parser.dir/flags.make
CMakeFiles/tac_parser.dir/driver/driver.cpp.obj: CMakeFiles/tac_parser.dir/includes_CXX.rsp
CMakeFiles/tac_parser.dir/driver/driver.cpp.obj: ../driver/driver.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/tac_parser.dir/driver/driver.cpp.obj"
	D:\programming\_mingw730_64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\tac_parser.dir\driver\driver.cpp.obj -c D:\programming\c\tac_parser\driver\driver.cpp

CMakeFiles/tac_parser.dir/driver/driver.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tac_parser.dir/driver/driver.cpp.i"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\programming\c\tac_parser\driver\driver.cpp > CMakeFiles\tac_parser.dir\driver\driver.cpp.i

CMakeFiles/tac_parser.dir/driver/driver.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tac_parser.dir/driver/driver.cpp.s"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\programming\c\tac_parser\driver\driver.cpp -o CMakeFiles\tac_parser.dir\driver\driver.cpp.s

CMakeFiles/tac_parser.dir/tac_worker/dataflow_graph.cpp.obj: CMakeFiles/tac_parser.dir/flags.make
CMakeFiles/tac_parser.dir/tac_worker/dataflow_graph.cpp.obj: CMakeFiles/tac_parser.dir/includes_CXX.rsp
CMakeFiles/tac_parser.dir/tac_worker/dataflow_graph.cpp.obj: ../tac_worker/dataflow_graph.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/tac_parser.dir/tac_worker/dataflow_graph.cpp.obj"
	D:\programming\_mingw730_64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\tac_parser.dir\tac_worker\dataflow_graph.cpp.obj -c D:\programming\c\tac_parser\tac_worker\dataflow_graph.cpp

CMakeFiles/tac_parser.dir/tac_worker/dataflow_graph.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tac_parser.dir/tac_worker/dataflow_graph.cpp.i"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\programming\c\tac_parser\tac_worker\dataflow_graph.cpp > CMakeFiles\tac_parser.dir\tac_worker\dataflow_graph.cpp.i

CMakeFiles/tac_parser.dir/tac_worker/dataflow_graph.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tac_parser.dir/tac_worker/dataflow_graph.cpp.s"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\programming\c\tac_parser\tac_worker\dataflow_graph.cpp -o CMakeFiles\tac_parser.dir\tac_worker\dataflow_graph.cpp.s

CMakeFiles/tac_parser.dir/tac_worker/LoopFinder.cpp.obj: CMakeFiles/tac_parser.dir/flags.make
CMakeFiles/tac_parser.dir/tac_worker/LoopFinder.cpp.obj: CMakeFiles/tac_parser.dir/includes_CXX.rsp
CMakeFiles/tac_parser.dir/tac_worker/LoopFinder.cpp.obj: ../tac_worker/LoopFinder.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/tac_parser.dir/tac_worker/LoopFinder.cpp.obj"
	D:\programming\_mingw730_64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\tac_parser.dir\tac_worker\LoopFinder.cpp.obj -c D:\programming\c\tac_parser\tac_worker\LoopFinder.cpp

CMakeFiles/tac_parser.dir/tac_worker/LoopFinder.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tac_parser.dir/tac_worker/LoopFinder.cpp.i"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\programming\c\tac_parser\tac_worker\LoopFinder.cpp > CMakeFiles\tac_parser.dir\tac_worker\LoopFinder.cpp.i

CMakeFiles/tac_parser.dir/tac_worker/LoopFinder.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tac_parser.dir/tac_worker/LoopFinder.cpp.s"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\programming\c\tac_parser\tac_worker\LoopFinder.cpp -o CMakeFiles\tac_parser.dir\tac_worker\LoopFinder.cpp.s

CMakeFiles/tac_parser.dir/tac_worker/basic_block.cpp.obj: CMakeFiles/tac_parser.dir/flags.make
CMakeFiles/tac_parser.dir/tac_worker/basic_block.cpp.obj: CMakeFiles/tac_parser.dir/includes_CXX.rsp
CMakeFiles/tac_parser.dir/tac_worker/basic_block.cpp.obj: ../tac_worker/basic_block.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/tac_parser.dir/tac_worker/basic_block.cpp.obj"
	D:\programming\_mingw730_64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\tac_parser.dir\tac_worker\basic_block.cpp.obj -c D:\programming\c\tac_parser\tac_worker\basic_block.cpp

CMakeFiles/tac_parser.dir/tac_worker/basic_block.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tac_parser.dir/tac_worker/basic_block.cpp.i"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\programming\c\tac_parser\tac_worker\basic_block.cpp > CMakeFiles\tac_parser.dir\tac_worker\basic_block.cpp.i

CMakeFiles/tac_parser.dir/tac_worker/basic_block.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tac_parser.dir/tac_worker/basic_block.cpp.s"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\programming\c\tac_parser\tac_worker\basic_block.cpp -o CMakeFiles\tac_parser.dir\tac_worker\basic_block.cpp.s

CMakeFiles/tac_parser.dir/parser.cpp.obj: CMakeFiles/tac_parser.dir/flags.make
CMakeFiles/tac_parser.dir/parser.cpp.obj: CMakeFiles/tac_parser.dir/includes_CXX.rsp
CMakeFiles/tac_parser.dir/parser.cpp.obj: parser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/tac_parser.dir/parser.cpp.obj"
	D:\programming\_mingw730_64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\tac_parser.dir\parser.cpp.obj -c D:\programming\c\tac_parser\cmake-build-debug\parser.cpp

CMakeFiles/tac_parser.dir/parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tac_parser.dir/parser.cpp.i"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\programming\c\tac_parser\cmake-build-debug\parser.cpp > CMakeFiles\tac_parser.dir\parser.cpp.i

CMakeFiles/tac_parser.dir/parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tac_parser.dir/parser.cpp.s"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\programming\c\tac_parser\cmake-build-debug\parser.cpp -o CMakeFiles\tac_parser.dir\parser.cpp.s

CMakeFiles/tac_parser.dir/lexer.cpp.obj: CMakeFiles/tac_parser.dir/flags.make
CMakeFiles/tac_parser.dir/lexer.cpp.obj: CMakeFiles/tac_parser.dir/includes_CXX.rsp
CMakeFiles/tac_parser.dir/lexer.cpp.obj: lexer.cpp
CMakeFiles/tac_parser.dir/lexer.cpp.obj: parser.hpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/tac_parser.dir/lexer.cpp.obj"
	D:\programming\_mingw730_64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\tac_parser.dir\lexer.cpp.obj -c D:\programming\c\tac_parser\cmake-build-debug\lexer.cpp

CMakeFiles/tac_parser.dir/lexer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tac_parser.dir/lexer.cpp.i"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\programming\c\tac_parser\cmake-build-debug\lexer.cpp > CMakeFiles\tac_parser.dir\lexer.cpp.i

CMakeFiles/tac_parser.dir/lexer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tac_parser.dir/lexer.cpp.s"
	D:\programming\_mingw730_64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\programming\c\tac_parser\cmake-build-debug\lexer.cpp -o CMakeFiles\tac_parser.dir\lexer.cpp.s

# Object files for target tac_parser
tac_parser_OBJECTS = \
"CMakeFiles/tac_parser.dir/main.cpp.obj" \
"CMakeFiles/tac_parser.dir/DotWriter/DotWriter.cpp.obj" \
"CMakeFiles/tac_parser.dir/driver/driver.cpp.obj" \
"CMakeFiles/tac_parser.dir/tac_worker/dataflow_graph.cpp.obj" \
"CMakeFiles/tac_parser.dir/tac_worker/LoopFinder.cpp.obj" \
"CMakeFiles/tac_parser.dir/tac_worker/basic_block.cpp.obj" \
"CMakeFiles/tac_parser.dir/parser.cpp.obj" \
"CMakeFiles/tac_parser.dir/lexer.cpp.obj"

# External object files for target tac_parser
tac_parser_EXTERNAL_OBJECTS =

tac_parser.exe: CMakeFiles/tac_parser.dir/main.cpp.obj
tac_parser.exe: CMakeFiles/tac_parser.dir/DotWriter/DotWriter.cpp.obj
tac_parser.exe: CMakeFiles/tac_parser.dir/driver/driver.cpp.obj
tac_parser.exe: CMakeFiles/tac_parser.dir/tac_worker/dataflow_graph.cpp.obj
tac_parser.exe: CMakeFiles/tac_parser.dir/tac_worker/LoopFinder.cpp.obj
tac_parser.exe: CMakeFiles/tac_parser.dir/tac_worker/basic_block.cpp.obj
tac_parser.exe: CMakeFiles/tac_parser.dir/parser.cpp.obj
tac_parser.exe: CMakeFiles/tac_parser.dir/lexer.cpp.obj
tac_parser.exe: CMakeFiles/tac_parser.dir/build.make
tac_parser.exe: D:/programming/_mingw730_64/lib/gvc.lib
tac_parser.exe: D:/programming/_mingw730_64/lib/cdt.lib
tac_parser.exe: D:/programming/_mingw730_64/lib/cgraph.lib
tac_parser.exe: D:/programming/_mingw730_64/lib/pathplan.lib
tac_parser.exe: CMakeFiles/tac_parser.dir/linklibs.rsp
tac_parser.exe: CMakeFiles/tac_parser.dir/objects1.rsp
tac_parser.exe: CMakeFiles/tac_parser.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Linking CXX executable tac_parser.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\tac_parser.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tac_parser.dir/build: tac_parser.exe

.PHONY : CMakeFiles/tac_parser.dir/build

CMakeFiles/tac_parser.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\tac_parser.dir\cmake_clean.cmake
.PHONY : CMakeFiles/tac_parser.dir/clean

CMakeFiles/tac_parser.dir/depend: parser.cpp
CMakeFiles/tac_parser.dir/depend: parser.hpp
CMakeFiles/tac_parser.dir/depend: lexer.cpp
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" D:\programming\c\tac_parser D:\programming\c\tac_parser D:\programming\c\tac_parser\cmake-build-debug D:\programming\c\tac_parser\cmake-build-debug D:\programming\c\tac_parser\cmake-build-debug\CMakeFiles\tac_parser.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/tac_parser.dir/depend

