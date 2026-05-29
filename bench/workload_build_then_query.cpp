// Three-phase workload: insert-only, then query-only, then remove-only.
//
// Phase 1 fills the graph up to ~avg_degree*n/2 edges (random missing edges);
// any inserts beyond the target become queries (rare — sized so phase 1 ends
// roughly at the cap). Phase 2 is pure queries on the fully-built graph.
// Phase 3 drains the graph by removing random present edges; once empty, the
// remaining slots are queries.
//
// Separating phases lets the plot attribute cost per op type — a single
// curve hides whether `tomtseng_hdt` wins because its inserts are faster or
// its queries are faster (they're not the same operation cost-wise).
//
// Params:
//   avg_degree         (default 4.0)
//   phase_insert_frac  (default 0.33)  — fraction of ops in phase 1
//   phase_query_frac   (default 0.34)  — fraction of ops in phase 2
//                                        (phase 3 = remainder)

#include "workload.hpp"

#include <cstddef>
#include <random>
#include <unordered_set>
#include <utility>

namespace workload {

std::vector<Op> generate_build_then_query(const Spec& s) {
    const double avg_degree        = param_double(s.params, "avg_degree", 4.0);
    const double phase_insert_frac = param_double(s.params, "phase_insert_frac", 0.33);
    const double phase_query_frac  = param_double(s.params, "phase_query_frac",  0.34);

    const int n_insert = s.num_ops * phase_insert_frac;
    const int n_query  = s.num_ops * phase_query_frac;
    const int n_remove = s.num_ops - n_insert - n_query;
    const long long target_m = avg_degree * s.n / 2.0;

    std::mt19937_64 rng(s.seed);
    std::uniform_int_distribution<int> vtx(0, s.n - 1);
    std::unordered_set<std::pair<int,int>, PairHash> present;
    std::vector<std::pair<int,int>> present_list;

    std::vector<Op> ops;
    ops.reserve(s.num_ops);

    auto random_pair = [&](int& u, int& v) {
        u = vtx(rng);
        v = vtx(rng);
        while (u == v) v = vtx(rng);
        if (u > v) std::swap(u, v);
    };

    for (int i = 0; i < n_insert; ++i) {
        if ((long long)present.size() < target_m) {
            int u, v;
            while (true) {
                random_pair(u, v);
                if (!present.count({u, v})) break;
            }
            present.insert({u, v});
            present_list.emplace_back(u, v);
            ops.push_back({OpKind::Insert, u, v});
        } else {
            int u, v;
            random_pair(u, v);
            ops.push_back({OpKind::Query, u, v});
        }
    }

    for (int i = 0; i < n_query; ++i) {
        int u, v;
        random_pair(u, v);
        ops.push_back({OpKind::Query, u, v});
    }

    for (int i = 0; i < n_remove; ++i) {
        if (!present_list.empty()) {
            std::uniform_int_distribution<std::size_t> pick(
                0, present_list.size() - 1);
            const std::size_t idx = pick(rng);
            const auto edge = present_list[idx];
            present_list[idx] = present_list.back();
            present_list.pop_back();
            present.erase(edge);
            ops.push_back({OpKind::Remove, edge.first, edge.second});
        } else {
            int u, v;
            random_pair(u, v);
            ops.push_back({OpKind::Query, u, v});
        }
    }
    return ops;
}

}  // namespace workload
