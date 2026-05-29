#include "workload.hpp"

#include <cstddef>
#include <random>
#include <unordered_set>
#include <utility>

namespace workload {

std::vector<Op> generate_sparse_random(const Spec& s) {
    const double avg_degree = param_double(s.params, "avg_degree", 4.0);
    const double p_query = param_double(s.params, "p_query", 0.33);

    const long long target_m = avg_degree * s.n / 2.0;

    std::mt19937_64 rng(s.seed);
    std::uniform_int_distribution<int> vtx(0, s.n - 1);
    std::uniform_real_distribution<double> uni(0.0, 1.0);
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

    for (int i = 0; i < s.num_ops; ++i) {
        if (uni(rng) < p_query) {
            int u, v;
            random_pair(u, v);
            ops.push_back({OpKind::Query, u, v});
            continue;
        }
        if ((long long)present.size() < target_m) {
            int u, v;
            while (true) {
                random_pair(u, v);
                if (!present.count({u, v})) break;
            }
            present.insert({u, v});
            present_list.emplace_back(u, v);
            ops.push_back({OpKind::Insert, u, v});
        } else if (!present_list.empty()) {
            std::uniform_int_distribution<std::size_t> pick(0, present_list.size() - 1);
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
