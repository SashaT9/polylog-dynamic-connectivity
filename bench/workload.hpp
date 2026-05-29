#pragma once

#include "runner.hpp"

#include <cstdint>
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace workload {

using Params = std::map<std::string, std::string>;
struct Spec {
    int n;
    int num_ops;
    uint64_t seed;
    Params params;
};
using GenFn = std::vector<Op>(*)(const Spec&);
GenFn lookup(const std::string& name);
std::vector<std::string> registered_names();

double param_double(const Params& p, const std::string& key, double dflt);
int    param_int   (const Params& p, const std::string& key, int dflt);

struct PairHash {
    std::size_t operator()(const std::pair<int,int>& p) const noexcept {
        const uint64_t mix = (static_cast<uint64_t>(p.first) << 32)
                           ^ static_cast<uint32_t>(p.second);
        return std::hash<uint64_t>{}(mix);
    }
};

}
