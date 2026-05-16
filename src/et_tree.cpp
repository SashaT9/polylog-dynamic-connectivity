#include "et_tree.hpp"
#include "bbst/splay_tree.hpp"

void EulerTourTree::Node::pull() {
    sz = 1 + (child[0] ? child[0]->sz : 0) + (child[1] ? child[1]->sz : 0);
    non_tree_adj = own_has_nt
                || (child[0] && child[0]->non_tree_adj)
                || (child[1] && child[1]->non_tree_adj);
    lvl_tree_edge = own_is_lvl_edge
                 || (child[0] && child[0]->lvl_tree_edge)
                 || (child[1] && child[1]->lvl_tree_edge);
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
EulerTourTree::Node* EulerTourTree::find_non_tree_adj(Node* n) {
    if (n->child[0] && n->child[0]->non_tree_adj) {
        return find_non_tree_adj(n->child[0]);
    }
    if (n->child[1] && n->child[1]->non_tree_adj) {
        return find_non_tree_adj(n->child[1]);
    }
    return n;
}
void EulerTourTree::set_vertex_has_nt(Vertex v, bool b) {
    Node* vn = vertex_node[v];
    splay_tree::splay(vn);
    vn->own_has_nt = b;
    vn->pull();
}
std::optional<Vertex> EulerTourTree::find_vertex_with_nt(Vertex v) {
    Node* vn = vertex_node[v];
    splay_tree::splay(vn);
    if (!vn->non_tree_adj) return std::nullopt;
    Node* marker = find_non_tree_adj(vn);
    return marker->id.u;
}
EulerTourTree::Node* EulerTourTree::find_lvl_tree_edge_descent(Node* n) {
    if (n->child[0] && n->child[0]->lvl_tree_edge) {
        return find_lvl_tree_edge_descent(n->child[0]);
    }
    if (n->child[1] && n->child[1]->lvl_tree_edge) {
        return find_lvl_tree_edge_descent(n->child[1]);
    }
    return n;
}
void EulerTourTree::set_edge_is_at_this_lvl(Vertex u, Vertex v, bool b) {
    Node* uv = edge_node[Edge(u, v)];
    splay_tree::splay(uv);
    uv->own_is_lvl_edge = b;
    uv->pull();
    Node* vu = edge_node[Edge(v, u)];
    splay_tree::splay(vu);
    vu->own_is_lvl_edge = b;
    vu->pull();
}
std::optional<Edge> EulerTourTree::find_lvl_tree_edge_in_tree(Vertex v) {
    Node* vn = vertex_node[v];
    splay_tree::splay(vn);
    if (!vn->lvl_tree_edge) return std::nullopt;
    Node* en = find_lvl_tree_edge_descent(vn);
    return canonical(en->id.u, en->id.v);
}
