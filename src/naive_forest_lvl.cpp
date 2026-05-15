#include "naive_forest_lvl.hpp"
#include <queue>

NaiveForestLvl::NaiveForestLvl(int n) : n(n), tree_adj(n), non_tree_adj(n) {}
void NaiveForestLvl::link(Vertex u, Vertex v) {
    this->tree_adj[u].insert(v);
    this->tree_adj[v].insert(u);
}
void NaiveForestLvl::cut(Vertex u, Vertex v) {
    this->tree_adj[u].erase(v);
    this->tree_adj[v].erase(u);
}
bool NaiveForestLvl::connected(Vertex u, Vertex v) {
    auto component = bfs_tree(u);
    return component.find(v) != component.end();
}
int NaiveForestLvl::tree_size(Vertex v) {
    return bfs_tree(v).size();
}
void NaiveForestLvl::add_non_tree_edge(Vertex u, Vertex v) {
    this->non_tree_adj[u].insert(v);
    this->non_tree_adj[v].insert(u);
}
void NaiveForestLvl::remove_non_tree_edge(Vertex u, Vertex v) {
    this->non_tree_adj[u].erase(v);
    this->non_tree_adj[v].erase(u);
}
std::vector<Edge> NaiveForestLvl::tree_edges_of(Vertex v) {
    auto component = bfs_tree(v);
    std::vector<Edge> edges;
    for (auto u : component) {
        for (auto to : this->tree_adj[u]) {
            if (u < to) {
                edges.emplace_back(u, to);
            }
        }
    }
    return edges;
}
std::optional<Edge> NaiveForestLvl::scan_non_tree_edges_of_tree(Vertex v, std::function<bool(Vertex, Vertex)> stop) {
    auto component = bfs_tree(v);
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
std::unordered_set<Vertex> NaiveForestLvl::bfs_tree(Vertex s) {
    std::unordered_set<Vertex> vis;
    std::queue<Vertex> q;
    vis.insert(s);
    q.push(s);
    while (!q.empty()) {
        Vertex v = q.front();
        q.pop();
        for (auto to : this->tree_adj[v]) {
            if (vis.find(to) == vis.end()) {
                vis.insert(to);
                q.push(to);
            }
        }
    }
    return vis;
}
