#pragma once

#include <vector>

enum class OpKind : int { Insert, Remove, Query };
struct Op {
    OpKind kind;
    int u;
    int v;
};
struct RunResult {
    long long total_ns;
    int num_inserts;
    int num_removes;
    int num_queries;
};
RunResult run_nari_hdt_et(int n, const std::vector<Op>& ops);
RunResult run_nari_hdt_naive(int n, const std::vector<Op>& ops);
RunResult run_tomtseng_hdt(int n, const std::vector<Op>& ops);
