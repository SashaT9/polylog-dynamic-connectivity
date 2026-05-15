#pragma once

#include "edge.hpp"
#include "forest_lvl.hpp"
#include <unordered_map>
#include <memory>


class DynamicConnectivity {
public:
    explicit DynamicConnectivity(int n);
    void insert(Vertex u, Vertex v);
    void remove(Vertex u, Vertex v);
    bool connected(Vertex u, Vertex v);
private:
    int n, L;
    std::unordered_map<Edge, EdgeInfo, EdgeHash> edges;
    std::vector<std::unique_ptr<ForestLvl>> lvls;
    bool replace(Vertex u, Vertex v, int lvl);
};
