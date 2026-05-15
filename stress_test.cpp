#include "dynamic_connectivity.hpp"
#include "brute_connectivity.hpp"
#include <iostream>
#include <random>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    int n = 20;
    int num_ops = 5000;
    uint64_t seed = std::random_device{}();

    if (argc >= 2) seed = std::stoull(argv[1]);
    if (argc >= 3) n = std::stoi(argv[2]);
    if (argc >= 4) num_ops = std::stoi(argv[3]);

    std::cerr << "stress: seed=" << seed << " n=" << n << " ops=" << num_ops << "\n";

    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> vtx_dist(0, n - 1);
    std::uniform_int_distribution<int> op_dist(0, 9);

    DynamicConnectivity dc(n);
    BruteConnectivity bc(n);

    std::vector<std::string> history;
    history.reserve(num_ops);

    for (int i = 0; i < num_ops; ++i) {
        Vertex u = vtx_dist(rng), v = vtx_dist(rng);
        while (u == v) v = vtx_dist(rng);

        int op = op_dist(rng);
        if (op < 4) {
            dc.insert(u, v);
            bc.insert(u, v);
            history.push_back("INSERT " + std::to_string(u) + " " + std::to_string(v));
        } else if (op < 7) {
            dc.remove(u, v);
            bc.remove(u, v);
            history.push_back("REMOVE " + std::to_string(u) + " " + std::to_string(v));
        } else {
            bool got = dc.connected(u, v);
            bool want = bc.connected(u, v);
            history.push_back("QUERY  " + std::to_string(u) + " " + std::to_string(v)
                              + " -> " + (got ? "true" : "false"));
            if (got != want) {
                std::cerr << "\nMISMATCH at op " << i << ":\n"
                          << "  connected(" << u << ", " << v << ")\n"
                          << "  dc = " << (got ? "true" : "false") << "\n"
                          << "  bc = " << (want ? "true" : "false") << "\n\n"
                          << "History (" << history.size() << " ops):\n";
                for (const auto& h : history) {
                    std::cerr << "  " << h << "\n";
                }
                std::cerr << "\nReproduce: ./stress_test " << seed << " " << n << " " << num_ops << "\n";
                return 1;
            }
        }
    }

    std::cerr << "OK: " << num_ops << " ops, no mismatches\n";
    return 0;
}
