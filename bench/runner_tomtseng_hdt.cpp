#include "runner.hpp"

#include <dynamic_graph/dynamic_connectivity.hpp>
#include <chrono>

namespace {
RunResult count_ops(const std::vector<Op>& ops) {
    RunResult r{0, 0, 0, 0};
    for (const Op& o : ops) {
        if (o.kind == OpKind::Insert) ++r.num_inserts;
        else if (o.kind == OpKind::Remove) ++r.num_removes;
        else ++r.num_queries;
    }
    return r;
}
}

RunResult run_tomtseng_hdt(int n, const std::vector<Op>& ops) {
    RunResult r = count_ops(ops);
    ::DynamicConnectivity dc(n);
    volatile bool sink = false;
    const auto t0 = std::chrono::steady_clock::now();
    for (const Op& o : ops) {
        UndirectedEdge e(o.u, o.v);
        switch (o.kind) {
        case OpKind::Insert: dc.AddEdge(e); break;
        case OpKind::Remove: dc.DeleteEdge(e); break;
        case OpKind::Query:  sink ^= dc.IsConnected(o.u, o.v); break;
        }
    }
    const auto t1 = std::chrono::steady_clock::now();
    (void)sink;
    r.total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    return r;
}
