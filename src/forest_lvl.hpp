#pragma once

#include "edge.hpp"
#include <vector>
#include <optional>

class ForestLvl {
public:
    virtual void link(Vertex u, Vertex v) = 0;
    virtual void cut(Vertex u, Vertex v) = 0;
    virtual bool connected(Vertex u, Vertex v) = 0;
    virtual int tree_size(Vertex v) = 0;
    virtual void add_non_tree_edge(Vertex u, Vertex v) = 0;
    virtual void remove_non_tree_edge(Vertex u, Vertex v) = 0;
    virtual std::vector<Edge> tree_edges_of(Vertex v) = 0;
    virtual std::optional<Edge> scan_non_tree_edges_of_tree(Vertex v, std::function<bool(Vertex, Vertex)> stop) = 0;
    virtual ~ForestLvl() = default;
};
