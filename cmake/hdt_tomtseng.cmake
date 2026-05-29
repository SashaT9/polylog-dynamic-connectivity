# Builds tomtseng/dynamic-connectivity-hdt from its submodule sources without
# invoking its own CMake (which pins g++-7, downloads googletest at configure
# time, and pulls in Doxygen). We pick up just the library sources we need.
#
# The upstream assert macro uses boost::stacktrace. We force the no-op backend
# so we don't depend on libbacktrace at link time.

set(HDT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hdt-tomtseng")

if(NOT EXISTS "${HDT_DIR}/src/dynamic_graph/include/dynamic_graph/dynamic_connectivity.hpp")
    message(FATAL_ERROR
        "Submodule hdt-tomtseng is empty or missing. "
        "Run: git submodule update --init --recursive")
endif()

find_library(BOOST_STACKTRACE_NOOP_LIB
    NAMES boost_stacktrace_noop
    REQUIRED)

add_library(hdt_tomtseng STATIC
    "${HDT_DIR}/src/dynamic_graph/src/dynamic_connectivity.cpp"
    "${HDT_DIR}/src/dynamic_graph/src/dynamic_forest.cpp"
    "${HDT_DIR}/src/dynamic_graph/src/graph.cpp"
    "${HDT_DIR}/src/dynamic_graph/src/sequence.cpp"
    "${HDT_DIR}/src/utilities/src/assert.cpp"
    "${HDT_DIR}/src/utilities/src/hash.cpp"
)

target_include_directories(hdt_tomtseng SYSTEM PUBLIC
    "${HDT_DIR}/src/dynamic_graph/include"
    "${HDT_DIR}/src/dynamic_graph/src"
    "${HDT_DIR}/src/utilities/include"
)

target_compile_definitions(hdt_tomtseng PUBLIC BOOST_STACKTRACE_USE_NOOP)
target_link_libraries(hdt_tomtseng PUBLIC ${BOOST_STACKTRACE_NOOP_LIB})

# Suppress warnings from third-party code so our -Wall -Wextra stays useful.
target_compile_options(hdt_tomtseng PRIVATE -w)
