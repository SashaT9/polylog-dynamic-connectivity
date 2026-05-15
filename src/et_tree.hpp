#pragma once

#include "edge.hpp"
#include <unordered_map>
#include <vector>

class EulerTourTree {
public:
    explicit EulerTourTree(int n);
    ~EulerTourTree();
    void link(Vertex u, Vertex v);
    void cut(Vertex u, Vertex v);
    bool connected(Vertex u, Vertex v);
    int tree_size(Vertex v);
    std::vector<Vertex> vertices_in_tree(Vertex v);
    std::vector<Edge> tree_edges_in_tree(Vertex v);

private:
    struct Node {
        Node* parent = nullptr;
        Node* child[2] = {nullptr, nullptr};
        int sz = 1;
        Edge id;
        Node(Edge e) : id(e) {}
        void pull();
    };
    Node* make_first(Node* n);
    int n;
    std::vector<Node*> vertex_node;
    std::unordered_map<Edge, Node*, EdgeHash> edge_node;
};
