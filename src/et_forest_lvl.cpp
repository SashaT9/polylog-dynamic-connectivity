#include "et_forest_lvl.hpp"
#include <unordered_set>

EtForestLvl::EtForestLvl(int n) : n(n), et(n), non_tree_adj(n) {}
void EtForestLvl::link(Vertex u, Vertex v) { et.link(u, v); }
void EtForestLvl::cut(Vertex u, Vertex v) { et.cut(u, v); }
bool EtForestLvl::connected(Vertex u, Vertex v) { return et.connected(u, v); }
int EtForestLvl::tree_size(Vertex v) { return et.tree_size(v); }
std::vector<Edge> EtForestLvl::tree_edges_of(Vertex v) { return et.tree_edges_in_tree(v); }
std::optional<Edge> EtForestLvl::scan_non_tree_edges_of_tree(Vertex v, std::function<bool(Vertex, Vertex)> stop) {
    auto vertices = et.vertices_in_tree(v);
    std::unordered_set<Vertex> component(vertices.begin(), vertices.end());
    std::vector<std::pair<Vertex, Vertex>> candidates;
    for (auto u : component) {
        for (auto to : this->non_tree_adj[u]) {
            if (u < to || component.find(to) == component.end()) {
                candidates.emplace_back(u, to);
            }
        }
    }
    for (auto [a, b] : candidates) {
        auto in = component.find(a) != component.end() ? a : b;
        auto out = in == a ? b : a;
        if (stop(in, out)) {
            return canonical(a, b);
        }
    }
    return std::nullopt;
}
void EtForestLvl::add_non_tree_edge(Vertex u, Vertex v) {
    non_tree_adj[u].insert(v);
    non_tree_adj[v].insert(u);
}
void EtForestLvl::remove_non_tree_edge(Vertex u, Vertex v) {
    non_tree_adj[u].erase(v);
    non_tree_adj[v].erase(u);
}
