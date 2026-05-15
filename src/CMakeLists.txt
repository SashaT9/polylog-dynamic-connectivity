add_subdirectory(bbst)

add_library(connectivity
    dynamic_connectivity.cpp
    et_tree.cpp
    naive_forest_lvl.cpp
    et_forest_lvl.cpp
)
target_include_directories(connectivity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(connectivity PUBLIC bbst)
