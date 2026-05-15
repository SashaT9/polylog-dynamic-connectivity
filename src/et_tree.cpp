#include "et_tree.hpp"
#include "bbst/splay_tree.hpp"

void EulerTourTree::Node::pull() {
    sz = 1 + (child[0] ? child[0]->sz : 0) + (child[1] ? child[1]->sz : 0);
}
EulerTourTree::EulerTourTree(int n) : n(n), vertex_node(n) {
    for (int v = 0; v < n; ++v) {
        vertex_node[v] = new Node(Edge(v, v));
    }
}
EulerTourTree::~EulerTourTree() {
    for (Node* node : vertex_node) delete node;
    for (auto& kv : edge_node) delete kv.second;
}
EulerTourTree::Node* EulerTourTree::make_first(Node* n) {
    auto [left, right] = splay_tree::split_before(n);
    if (!left) return right;
    return splay_tree::join(right, left);
}
bool EulerTourTree::connected(Vertex u, Vertex v) {
    Node* un = vertex_node[u];
    Node* vn = vertex_node[v];
    splay_tree::splay(un);
    return splay_tree::root(vn) == un;
}
int EulerTourTree::tree_size(Vertex v) {
    Node* vn = vertex_node[v];
    splay_tree::splay(vn);
    return (vn->sz + 2) / 3;
}
void EulerTourTree::link(Vertex u, Vertex v) {
    Node* un = vertex_node[u];
    Node* vn = vertex_node[v];
    Node* tu = make_first(un);
    Node* tv = make_first(vn);
    Node* uv = new Node(Edge(u, v));
    Node* vu = new Node(Edge(v, u));
    Node* t = splay_tree::join(tu, uv);
    t = splay_tree::join(t, tv);
    splay_tree::join(t, vu);
    edge_node[Edge(u, v)] = uv;
    edge_node[Edge(v, u)] = vu;
}
void EulerTourTree::cut(Vertex u, Vertex v) {
    auto it = edge_node.find(Edge(u, v));
    if (it == edge_node.end()) return;
    Node* uv = it->second;
    Node* vu = edge_node[Edge(v, u)];
    make_first(uv);
    splay_tree::split_after(uv);
    splay_tree::split_before(vu);
    splay_tree::split_after(vu);
    delete uv;
    delete vu;
    edge_node.erase(Edge(u, v));
    edge_node.erase(Edge(v, u));
}
std::vector<Vertex> EulerTourTree::vertices_in_tree(Vertex v) {
    Node* vn = vertex_node[v];
    splay_tree::splay(vn);
    std::vector<Vertex> res;
    Node* cur = splay_tree::leftmost(vn);
    while (cur) {
        if (cur->id.u == cur->id.v) res.push_back(cur->id.u);
        cur = splay_tree::successor(cur);
    }
    return res;
}
std::vector<Edge> EulerTourTree::tree_edges_in_tree(Vertex v) {
    Node* vn = vertex_node[v];
    splay_tree::splay(vn);
    std::vector<Edge> res;
    Node* cur = splay_tree::leftmost(vn);
    while (cur) {
        if (cur->id.u < cur->id.v) res.push_back(cur->id);
        cur = splay_tree::successor(cur);
    }
    return res;
}
