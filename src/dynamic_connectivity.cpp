#include "dynamic_connectivity.hpp"
#include "et_forest_lvl.hpp"
#include <cmath>
#include <memory>
#include <optional>

DynamicConnectivity::DynamicConnectivity(int n)
    : DynamicConnectivity(n, [](int sz) {
        return std::make_unique<EtForestLvl>(sz);
      })
{}
DynamicConnectivity::DynamicConnectivity(int n, ForestLvlFactory make_lvl) {
    this->n = n;
    this->L = std::log2(n) + 1;
    this->lvls.reserve(this->L);
    for (int i = 0; i < this->L; ++i) {
        this->lvls.push_back(make_lvl(n));
    }
}
void DynamicConnectivity::insert(Vertex u, Vertex v) {
    Edge e = canonical(u, v);
    if (edges.count(e)) return;
    if (!this->lvls[0]->connected(u, v)) {
        edges[e] = {0, true};
        this->lvls[0]->link(u, v);
        this->lvls[0]->set_tree_edge_is_at_my_lvl(u, v, true);
    } else {
        edges[e] = {0, false};
        this->lvls[0]->add_non_tree_edge(u, v);
    }
}
void DynamicConnectivity::remove(Vertex u, Vertex v) {
    Edge e = canonical(u, v);
    auto it = edges.find(e);
    if (it == edges.end()) return;
    int lvl = it->second.lvl;
    bool is_tree_edge = it->second.is_tree_edge;
    edges.erase(it);
    if (!is_tree_edge) {
        lvls[lvl]->remove_non_tree_edge(u, v);
    } else {
        for (int j = 0; j <= lvl; ++j) {
            lvls[j]->cut(u, v);
        }
        replace(u, v, lvl);
    }
}
bool DynamicConnectivity::connected(Vertex u, Vertex v) {
    return this->lvls[0]->connected(u, v);
}
bool DynamicConnectivity::replace(Vertex u, Vertex v, int lvl) {
    int sz_u = this->lvls[lvl]->tree_size(u);
    int sz_v = this->lvls[lvl]->tree_size(v);
    Vertex small = (sz_u < sz_v) ? u : v;
    Vertex large = (sz_u < sz_v) ? v : u;
    if (lvl + 1 < L) {
        while (auto e_opt = lvls[lvl]->find_lvl_tree_edge_in_tree(small)) {
            Edge e = *e_opt;
            auto it = edges.find(e);
            lvls[lvl]->set_tree_edge_is_at_my_lvl(e.u, e.v, false);
            it->second.lvl = lvl + 1;
            lvls[lvl + 1]->link(e.u, e.v);
            lvls[lvl + 1]->set_tree_edge_is_at_my_lvl(e.u, e.v, true);
        }
    }
    while (auto edge_opt = lvls[lvl]->find_non_tree_edge_in_tree(small)) {
        Vertex in = edge_opt->u;
        Vertex out = edge_opt->v;
        Edge e = canonical(in, out);
        auto it = edges.find(e);
        if (lvls[lvl]->connected(out, large)) {
            int e_lvl = it->second.lvl;
            lvls[e_lvl]->remove_non_tree_edge(e.u, e.v);
            for (int i = 0; i <= e_lvl; ++i) {
                lvls[i]->link(e.u, e.v);
            }
            lvls[e_lvl]->set_tree_edge_is_at_my_lvl(e.u, e.v, true);
            it->second.is_tree_edge = true;
            return true;
        }
        if (lvl + 1 < L) {
            lvls[lvl]->remove_non_tree_edge(e.u, e.v);
            lvls[lvl + 1]->add_non_tree_edge(e.u, e.v);
            it->second.lvl = lvl + 1;
        } else {
            break;
        }
    }
    if (lvl > 0) {
        return replace(u, v, lvl - 1);
    }
    return false;
}
