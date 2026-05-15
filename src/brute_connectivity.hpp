#pragma once

#include "edge.hpp"
#include <unordered_set>
#include <queue>


class BruteConnectivity {
public:
    explicit BruteConnectivity(int n) : n(n), adj(n) {}
    void insert(Vertex u, Vertex v) {
        adj[u].insert(v);
        adj[v].insert(u);
    }
    void remove(Vertex u, Vertex v) {
        adj[u].erase(v);
        adj[v].erase(u);
    }
    bool connected(Vertex u, Vertex v) {
        std::vector<bool> vis(n, false);
        std::queue<Vertex> q;
        vis[u] = true;
        q.push(u);
        while (!q.empty()) {
            Vertex cur = q.front();
            q.pop();
            if (cur == v) {
                return true;
            }
            for (auto to : adj[cur]) {
                if (!vis[to]) {
                    vis[to] = true;
                    q.push(to);
                }
            }
        }
        return false;
    }
private:
    int n;
    std::vector<std::unordered_set<Vertex>> adj;
};
