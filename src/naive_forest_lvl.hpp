#pragma once

#include "forest_lvl.hpp"
#include <unordered_set>

class NaiveForestLvl : public ForestLvl {
public:
    explicit NaiveForestLvl(int n);
    virtual void link(Vertex u, Vertex v) override;
    virtual void cut(Vertex u, Vertex v) override;
    virtual bool connected(Vertex u, Vertex v) override;
    virtual int tree_size(Vertex v) override;
    virtual void add_non_tree_edge(Vertex u, Vertex v) override;
    virtual void remove_non_tree_edge(Vertex u, Vertex v) override;
    virtual void set_tree_edge_is_at_my_lvl(Vertex u, Vertex v, bool b) override;
    virtual std::optional<Edge> find_lvl_tree_edge_in_tree(Vertex v) override;
    virtual std::optional<Edge> find_non_tree_edge_in_tree(Vertex v) override;
private:
    int n;
    std::vector<std::unordered_set<Vertex>> tree_adj, non_tree_adj;
    std::unordered_set<Edge, EdgeHash> tree_edges_at_my_lvl;
    std::unordered_set<Vertex> bfs_tree(Vertex s);
};
