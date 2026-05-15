#pragma once

#include <functional>

using Vertex = int;

struct Edge {
    Vertex u, v;
    Edge(Vertex a, Vertex b) : u(a), v(b) {}
    bool operator==(const Edge &other) const {
        return u == other.u && v == other.v;
    }
};

inline Edge canonical(Vertex a, Vertex b) {
    return a <= b ? Edge(a, b) : Edge(b, a);
}

struct EdgeInfo {
    int lvl;
    bool is_tree_edge;
};

struct EdgeHash {
    std::size_t operator()(const Edge &e) const {
        auto h1 = std::hash<Vertex>{}(e.u);
        auto h2 = std::hash<Vertex>{}(e.v);
        return h1 ^ (h2 * 2654435761u);
    }
};
