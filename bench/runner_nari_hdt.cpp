#include "runner.hpp"

#include "dynamic_connectivity.hpp"
#include "et_forest_lvl.hpp"
#include "naive_forest_lvl.hpp"
#include <chrono>
#include <memory>

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

template <class Init>
RunResult run_loop(Init init, const std::vector<Op>& ops) {
    RunResult r = count_ops(ops);
    DynamicConnectivity dc = init();
    volatile bool sink = false;
    const auto t0 = std::chrono::steady_clock::now();
    for (const Op& o : ops) {
        switch (o.kind) {
        case OpKind::Insert: dc.insert(o.u, o.v); break;
        case OpKind::Remove: dc.remove(o.u, o.v); break;
        case OpKind::Query:  sink ^= dc.connected(o.u, o.v); break;
        }
    }
    const auto t1 = std::chrono::steady_clock::now();
    (void)sink;
    r.total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    return r;
}
}

RunResult run_nari_hdt_et(int n, const std::vector<Op>& ops) {
    return run_loop([n] {
        return DynamicConnectivity(n, [](int sz) {
            return std::make_unique<EtForestLvl>(sz);
        });
    }, ops);
}

RunResult run_nari_hdt_naive(int n, const std::vector<Op>& ops) {
    return run_loop([n] {
        return DynamicConnectivity(n, [](int sz) {
            return std::make_unique<NaiveForestLvl>(sz);
        });
    }, ops);
}
