// Adversarial path workload: build the path 0-1-...-(n-1), then toggle
// random path edges interspersed with connectivity queries.
//
// Why this stresses HDT: when a tree-edge in a graph with no non-tree edges
// is deleted, HDT's level-promotion search scans candidates at every level
// 0..log n and finds no replacement on each — the canonical worst case for
// the O(log^2 n) amortized bound. Naive's BFS pays O(n) per query because
// the path forces it to walk most of the graph before deciding (dis)connection.
//
// The first min(n-1, num_ops) operations build the path; thereafter each op
// is either a Query (random pair) with probability p_query, or a Toggle of
// a random path edge — Insert if absent, Remove if present. In steady state
// each path edge spends ~half its time present (symmetric random walk), so
// removes account for ~half of toggles.
//
// Params:
//   p_query (default 0.34)

#include "workload.hpp"

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

namespace workload {

std::vector<Op> generate_forest_path(const Spec& s) {
    const double p_query = param_double(s.params, "p_query", 0.33);

    std::mt19937_64 rng(s.seed);
    std::uniform_int_distribution<int> vtx(0, s.n - 1);
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    std::uniform_int_distribution<int> path_idx(0, s.n - 2);

    std::vector<char> present(s.n - 1, 0);
    std::vector<Op> ops;
    ops.reserve(s.num_ops);

    auto random_pair = [&](int& u, int& v) {
        u = vtx(rng);
        v = vtx(rng);
        while (u == v) v = vtx(rng);
        if (u > v) std::swap(u, v);
    };

    const int build_ops = std::min(s.n - 1, s.num_ops);
    for (int i = 0; i < build_ops; ++i) {
        present[i] = 1;
        ops.push_back({OpKind::Insert, i, i + 1});
    }

    for (int i = build_ops; i < s.num_ops; ++i) {
        if (uni(rng) < p_query) {
            int u, v;
            random_pair(u, v);
            ops.push_back({OpKind::Query, u, v});
            continue;
        }
        const int idx = path_idx(rng);
        if (present[idx]) {
            present[idx] = 0;
            ops.push_back({OpKind::Remove, idx, idx + 1});
        } else {
            present[idx] = 1;
            ops.push_back({OpKind::Insert, idx, idx + 1});
        }
    }
    return ops;
}

}  // namespace workload
