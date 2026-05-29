#include "workload.hpp"

namespace workload {

std::vector<Op> generate_uniform(const Spec&);
std::vector<Op> generate_sparse_random(const Spec&);
std::vector<Op> generate_build_then_query(const Spec&);
std::vector<Op> generate_forest_path(const Spec&);
std::vector<Op> generate_clique_bridges(const Spec&);

namespace {
struct Entry { const char* name; GenFn fn; };
const Entry kRegistry[] = {
    {"uniform",          &generate_uniform},
    {"sparse_random",    &generate_sparse_random},
    {"build_then_query", &generate_build_then_query},
    {"forest_path",      &generate_forest_path},
    {"clique_bridges",   &generate_clique_bridges},
};
}

GenFn lookup(const std::string& name) {
    for (const auto& e : kRegistry) {
        if (name == e.name) return e.fn;
    }
    return nullptr;
}
std::vector<std::string> registered_names() {
    std::vector<std::string> v;
    v.reserve(sizeof(kRegistry) / sizeof(kRegistry[0]));
    for (const auto& e : kRegistry) v.emplace_back(e.name);
    return v;
}
double param_double(const Params& p, const std::string& key, double dflt) {
    auto it = p.find(key);
    return it == p.end() ? dflt : std::stod(it->second);
}
int param_int(const Params& p, const std::string& key, int dflt) {
    auto it = p.find(key);
    return it == p.end() ? dflt : std::stoi(it->second);
}

}  // namespace workload
