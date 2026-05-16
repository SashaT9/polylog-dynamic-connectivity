#pragma once

#include "edge.hpp"
#include <optional>

class ForestLvl {
public:
    virtual void link(Vertex u, Vertex v) = 0;
    virtual void cut(Vertex u, Vertex v) = 0;
    virtual bool connected(Vertex u, Vertex v) = 0;
    virtual int tree_size(Vertex v) = 0;
    virtual void add_non_tree_edge(Vertex u, Vertex v) = 0;
    virtual void remove_non_tree_edge(Vertex u, Vertex v) = 0;
    virtual void set_tree_edge_is_at_my_lvl(Vertex u, Vertex v, bool b) = 0;
    virtual std::optional<Edge> find_lvl_tree_edge_in_tree(Vertex v) = 0;
    virtual std::optional<Edge> find_non_tree_edge_in_tree(Vertex v) = 0;
    virtual ~ForestLvl() = default;
};
