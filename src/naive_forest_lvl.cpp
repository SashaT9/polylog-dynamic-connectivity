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
    tree_edges_at_my_lvl.erase(canonical(u, v));
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
void NaiveForestLvl::set_tree_edge_is_at_my_lvl(Vertex u, Vertex v, bool b) {
    Edge e = canonical(u, v);
    if (b) tree_edges_at_my_lvl.insert(e);
    else tree_edges_at_my_lvl.erase(e);
}
std::optional<Edge> NaiveForestLvl::find_lvl_tree_edge_in_tree(Vertex v) {
    auto component = bfs_tree(v);
    for (auto u : component) {
        for (auto to : this->tree_adj[u]) {
            if (u < to) {
                Edge e(u, to);
                if (tree_edges_at_my_lvl.count(e)) return e;
            }
        }
    }
    return std::nullopt;
}
std::optional<Edge> NaiveForestLvl::find_non_tree_edge_in_tree(Vertex v) {
    auto component = bfs_tree(v);
    for (auto u : component) {
        for (auto to : this->non_tree_adj[u]) {
            return Edge(u, to);
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
