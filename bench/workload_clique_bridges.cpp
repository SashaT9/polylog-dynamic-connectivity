// Adversarial replace workload: two cliques connected by k bridge edges.
//
// Why this stresses replace: at any time exactly one of the k bridges is the
// tree edge connecting the two cliques and the other k-1 are non-tree.
// Removing the tree bridge forces HLT's replace search to find one of the
// remaining bridges among the smaller side's non-tree edges -- and the
// smaller side is one whole clique with O(n^2) internal non-tree edges that
// look like candidates but are not (both endpoints live in the same clique).
// HLT inspects and amortises many such candidates via level promotion before
// locating a real replacement. Naive's BFS-on-cut also walks the whole
// clique on every tree-bridge removal, so its per-op cost climbs as O(n^2).
//
// Setup phase emits all clique-internal edges plus all k bridges (counted
// toward num_ops). The toggle phase picks a uniformly random bridge each
// step and removes it if present or re-inserts it if absent; no queries.
//
// For this workload the bench's --ops should be set well above the setup
// cost (clique A has n/2 choose 2 edges, ditto clique B, plus k bridges).
// At n=1000 with default k=4 the setup is ~250k inserts, so --ops should
// be at least ~10^6 for a meaningful toggle phase.
//
// Params:
//   k (default 4): number of bridge edges. Bridge b connects (b, n/2 + b).

#include "workload.hpp"

#include <random>
#include <utility>
#include <vector>

namespace workload {

std::vector<Op> generate_clique_bridges(const Spec& s) {
    const int k = param_int(s.params, "k", 4);
    const int half = s.n / 2;

    if (s.n < 4 || k < 1 || k > half) {
        return {};
    }

    std::vector<Op> ops;
    ops.reserve(s.num_ops);

    auto push = [&](OpKind kind, int u, int v) {
        if (u > v) std::swap(u, v);
        ops.push_back({kind, u, v});
    };

    for (int i = 0; i < half; ++i) {
        for (int j = i + 1; j < half; ++j) {
            if ((int)ops.size() >= s.num_ops) return ops;
            push(OpKind::Insert, i, j);
        }
    }
    for (int i = half; i < s.n; ++i) {
        for (int j = i + 1; j < s.n; ++j) {
            if ((int)ops.size() >= s.num_ops) return ops;
            push(OpKind::Insert, i, j);
        }
    }

    std::vector<char> bridge_present(k, 0);
    for (int b = 0; b < k; ++b) {
        if ((int)ops.size() >= s.num_ops) return ops;
        push(OpKind::Insert, b, half + b);
        bridge_present[b] = 1;
    }

    std::mt19937_64 rng(s.seed);
    std::uniform_int_distribution<int> pick(0, k - 1);
    while ((int)ops.size() < s.num_ops) {
        const int b = pick(rng);
        if (bridge_present[b]) {
            push(OpKind::Remove, b, half + b);
            bridge_present[b] = 0;
        } else {
            push(OpKind::Insert, b, half + b);
            bridge_present[b] = 1;
        }
    }
    return ops;
}

}  // namespace workload
