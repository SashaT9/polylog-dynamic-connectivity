#pragma once

#include "edge.hpp"
#include <optional>
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
    void set_vertex_has_nt(Vertex v, bool b);
    std::optional<Vertex> find_vertex_with_nt(Vertex v);
    void set_edge_is_at_this_lvl(Vertex u, Vertex v, bool b);
    std::optional<Edge> find_lvl_tree_edge_in_tree(Vertex v);
private:
    struct Node {
        Node* parent = nullptr;
        Node* child[2] = {nullptr, nullptr};
        int sz = 1;
        bool own_has_nt = false;
        bool non_tree_adj = false;
        bool own_is_lvl_edge = false;
        bool lvl_tree_edge = false;
        Edge id;
        Node(Edge e) : id(e) {}
        void pull();
    };
    Node* make_first(Node* n);
    Node* find_non_tree_adj(Node* n);
    Node* find_lvl_tree_edge_descent(Node* n);
    int n;
    std::vector<Node*> vertex_node;
    std::unordered_map<Edge, Node*, EdgeHash> edge_node;
};
