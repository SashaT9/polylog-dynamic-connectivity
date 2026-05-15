#pragma once

#include "et_tree.hpp"
#include "forest_lvl.hpp"
#include <unordered_set>

class EtForestLvl : public ForestLvl {
public:
    explicit EtForestLvl(int n);
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
    EulerTourTree et;
    std::vector<std::unordered_set<Vertex>> non_tree_adj;
};
