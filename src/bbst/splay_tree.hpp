#pragma once

#include <utility>

namespace splay_tree {
template<typename Node>
int side(Node* n) {
    return n->parent->child[0] == n ? 0 : 1;
}
template<typename Node>
inline void rotate(Node* n) {
    Node* p = n->parent;
    Node* g = p->parent;
    int s = side(n);
    Node* b = n->child[1 - s];
    p->child[s] = b;
    if (b) {
        b->parent = p;
    }
    n->child[1 - s] = p;
    p->parent = n;
    n->parent = g;
    if (g) {
        g->child[g->child[0] == p ? 0 : 1] = n;
    }
    p->pull();
    n->pull();
}
template<typename Node>
void splay(Node* n) {
    while (n->parent) {
        Node* p = n->parent;
        if (!p->parent) {
            rotate<Node>(n);
        } else if (side<Node>(n) == side<Node>(p)) {
            rotate<Node>(p);
            rotate<Node>(n);
        } else {
            rotate<Node>(n);
            rotate<Node>(n);
        }
    }
}
template<typename Node>
Node* root(Node* n) {
    while (n->parent) {
        n = n->parent;
    }
    return n;
}
template<typename Node>
Node* join(Node* A, Node* B) {
    if (!A) return B;
    if (!B) return A;
    Node* r = A;
    while (r->child[1]) {
        r = r->child[1];
    }
    splay<Node>(r);
    r->child[1] = B;
    B->parent = r;
    r->pull();
    return r;
}
template<typename Node>
std::pair<Node*, Node*> split_after(Node* n) {
    splay<Node>(n);
    Node* r = n->child[1];
    if (r) {
        r->parent = nullptr;
        n->child[1] = nullptr;
        n->pull();
    }
    return {n, r};
}
template<typename Node>
std::pair<Node*, Node*> split_before(Node* n) {
    splay<Node>(n);
    Node* l = n->child[0];
    if (l) {
        l->parent = nullptr;
        n->child[0] = nullptr;
        n->pull();
    }
    return {l, n};
}
}
