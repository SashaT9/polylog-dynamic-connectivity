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
    virtual std::vector<Edge> tree_edges_of(Vertex v) override;
    virtual std::optional<Edge> scan_non_tree_edges_of_tree(Vertex v, std::function<bool(Vertex, Vertex)> stop) override;
private:
    int n;
    std::vector<std::unordered_set<Vertex>> tree_adj, non_tree_adj;
    std::unordered_set<Vertex> bfs_tree(Vertex s);
};
