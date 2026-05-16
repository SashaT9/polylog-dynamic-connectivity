#include "et_forest_lvl.hpp"

EtForestLvl::EtForestLvl(int n) : n(n), et(n), non_tree_adj(n) {}
void EtForestLvl::link(Vertex u, Vertex v) { et.link(u, v); }
void EtForestLvl::cut(Vertex u, Vertex v) { et.cut(u, v); }
bool EtForestLvl::connected(Vertex u, Vertex v) { return et.connected(u, v); }
int EtForestLvl::tree_size(Vertex v) { return et.tree_size(v); }
void EtForestLvl::set_tree_edge_is_at_my_lvl(Vertex u, Vertex v, bool b) { et.set_edge_is_at_this_lvl(u, v, b); }
std::optional<Edge> EtForestLvl::find_lvl_tree_edge_in_tree(Vertex v) { return et.find_lvl_tree_edge_in_tree(v); }
std::optional<Edge> EtForestLvl::find_non_tree_edge_in_tree(Vertex v) {
    auto w = et.find_vertex_with_nt(v);
    if (!w) return std::nullopt;
    Vertex other = *non_tree_adj[*w].begin();
    return Edge(*w, other);
}
void EtForestLvl::add_non_tree_edge(Vertex u, Vertex v) {
    bool u_was_empty = non_tree_adj[u].empty();
    bool v_was_empty = non_tree_adj[v].empty();
    non_tree_adj[u].insert(v);
    non_tree_adj[v].insert(u);
    if (u_was_empty) et.set_vertex_has_nt(u, true);
    if (v_was_empty) et.set_vertex_has_nt(v, true);
}
void EtForestLvl::remove_non_tree_edge(Vertex u, Vertex v) {
    non_tree_adj[u].erase(v);
    non_tree_adj[v].erase(u);
    if (non_tree_adj[u].empty()) et.set_vertex_has_nt(u, false);
    if (non_tree_adj[v].empty()) et.set_vertex_has_nt(v, false);
}
