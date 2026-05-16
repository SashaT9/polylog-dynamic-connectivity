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
    virtual void set_tree_edge_is_at_my_lvl(Vertex u, Vertex v, bool b) override;
    virtual std::optional<Edge> find_lvl_tree_edge_in_tree(Vertex v) override;
    virtual std::optional<Edge> find_non_tree_edge_in_tree(Vertex v) override;
private:
    int n;
    EulerTourTree et;
    std::vector<std::unordered_set<Vertex>> non_tree_adj;
};
