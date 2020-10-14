# graphviz_LIBRARIES
# graphviz_FOUND
# graphviz_INCLUDE_DIRECTORIES

#find_package(PkgConfig)
#pkg_check_modules(pc_graphviz ${REQUIRED} libgvc libcdt libcgraph libpathplan)

find_path(graphviz_INCLUDE_DIRECTORIES
        NAMES gvc.h
        HINTS ${pc_graphviz_INCLUDEDIR}
        ${pc_graphviz_INCLUDE_DIRS}
        )

find_library(graphviz_GVC_LIBRARY
        NAMES gvc
        HINTS ${pc_graphviz_LIBDIR}
        ${pc_graphviz_LIBRARY_DIRS}
        )

find_library(graphviz_CDT_LIBRARY
        NAMES cdt
        HINTS ${pc_graphviz_LIBDIR}
        ${pc_graphviz_LIBRARY_DIRS}
        )

find_library(graphviz_GRAPH_LIBRARY
        NAMES cgraph
        HINTS ${pc_graphviz_LIBDIR}
        ${pc_graphviz_LIBRARY_DIRS}
        )

find_library(graphviz_PATHPLAN_LIBRARY
        NAMES pathplan
        HINTS ${pc_graphviz_LIBDIR}
        ${pc_graphviz_LIBRARY_DIRS}
        )

set(graphviz_LIBRARIES
        "${graphviz_GVC_LIBRARY}" "${graphviz_CDT_LIBRARY}"
        "${graphviz_GRAPH_LIBRARY}" "${graphviz_PATHPLAN_LIBRARY}")

if (EXISTS "${graphviz_INCLUDE_DIRECTORIES}/graphviz_version.h")
    file(READ "${graphviz_INCLUDE_DIRECTORIES}/graphviz_version.h" _graphviz_version_content)
    string(REGEX MATCH "#define +PACKAGE_VERSION +\"([0-9]+\\.[0-9]+\\.[0-9]+)\"" _dummy "${_graphviz_version_content}")
    set(graphviz_VERSION "${CMAKE_MATCH_1}")
endif ()

if ("${Graphviz_FIND_VERSION}" VERSION_GREATER "${graphviz_VERSION}")
    message(FATAL_ERROR "Required version (" ${Graphviz_FIND_VERSION} ") is higher than found version (" ${graphviz_VERSION} ")")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Graphviz
        REQUIRED_VARS graphviz_LIBRARIES graphviz_INCLUDE_DIRECTORIES
        VERSION_VAR   graphviz_VERSION)
