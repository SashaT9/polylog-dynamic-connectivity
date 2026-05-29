add_executable(bench
    bench.cpp
    runner_nari_hdt.cpp
    runner_tomtseng_hdt.cpp
    workload.cpp
    workload_uniform.cpp
    workload_sparse_random.cpp
    workload_build_then_query.cpp
    workload_forest_path.cpp
    workload_clique_bridges.cpp
)
target_link_libraries(bench PRIVATE connectivity hdt_tomtseng)
