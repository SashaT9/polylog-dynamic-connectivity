#include "workload.hpp"

#include <random>
#include <unordered_set>
#include <utility>

namespace workload {

std::vector<Op> generate_uniform(const Spec& s) {
    const double p_insert = param_double(s.params, "p_insert", 0.33);
    const double p_remove = param_double(s.params, "p_remove", 0.33);

    std::mt19937_64 rng(s.seed);
    std::uniform_int_distribution<int> vtx(0, s.n - 1);
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    std::unordered_set<std::pair<int,int>, PairHash> present;

    std::vector<Op> ops;
    ops.reserve(s.num_ops);

    for (int i = 0; i < s.num_ops; ++i) {
        int u = vtx(rng);
        int v = vtx(rng);
        while (u == v) v = vtx(rng);
        if (u > v) std::swap(u, v);

        const double r = uni(rng);
        const OpKind kind = r < p_insert
            ? OpKind::Insert
            : (r < p_insert + p_remove ? OpKind::Remove : OpKind::Query);

        if (kind == OpKind::Insert) {
            if (present.insert({u, v}).second) {
                ops.push_back({OpKind::Insert, u, v});
            } else {
                ops.push_back({OpKind::Query, u, v});
            }
        } else if (kind == OpKind::Remove) {
            if (present.erase({u, v})) {
                ops.push_back({OpKind::Remove, u, v});
            } else {
                ops.push_back({OpKind::Query, u, v});
            }
        } else {
            ops.push_back({OpKind::Query, u, v});
        }
    }
    return ops;
}

}  // namespace workload
