// Benchmark driver for dynamic connectivity implementations.
//
// Generates an op stream from a named workload (uniform, sparse_random,
// build_then_query, ...) and replays it against each implementation. Total
// wall time per impl is written to a CSV that plot.py consumes.
//
// Usage:
//   bench [--n N] [--ops K] [--seed S]
//         [--workload NAME] [--param KEY=VAL] ...
//         [--p-insert P] [--p-remove P]              # sugar for uniform
//         [--out PATH] [--label TAG]
//         [--algos nari_hdt_et,nari_hdt_naive,tomtseng_hdt]
//
// Defaults: n=1000, ops=100000, seed from random_device,
//           workload=uniform, algos=nari_hdt_et,tomtseng_hdt,
//           out=bench.csv.

#include "runner.hpp"
#include "workload.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Args {
    int n = 1000;
    int num_ops = 100000;
    uint64_t seed = 0;
    bool seed_set = false;
    std::string workload = "uniform";
    workload::Params params;
    std::string out = "bench.csv";
    std::string label;
    std::vector<std::string> algos = {"nari_hdt_et", "tomtseng_hdt"};
};

void die(const std::string& msg) {
    std::cerr << "bench: " << msg << "\n";
    std::exit(2);
}

void parse_kv_into(workload::Params& dst, const std::string& kv) {
    const auto eq = kv.find('=');
    if (eq == std::string::npos)
        die("--param expects KEY=VALUE, got: " + kv);
    dst[kv.substr(0, eq)] = kv.substr(eq + 1);
}

Args parse_args(int argc, char** argv) {
    Args a;
    auto need = [&](int i) {
        if (i + 1 >= argc) die(std::string("missing value for ") + argv[i]);
    };
    for (int i = 1; i < argc; ++i) {
        std::string k = argv[i];
        if      (k == "--n")        { need(i); a.n       = std::stoi (argv[++i]); }
        else if (k == "--ops")      { need(i); a.num_ops = std::stoi (argv[++i]); }
        else if (k == "--seed")     { need(i); a.seed    = std::stoull(argv[++i]); a.seed_set = true; }
        else if (k == "--workload") { need(i); a.workload = argv[++i]; }
        else if (k == "--param")    { need(i); parse_kv_into(a.params, argv[++i]); }
        else if (k == "--p-insert") { need(i); a.params["p_insert"] = argv[++i]; }
        else if (k == "--p-remove") { need(i); a.params["p_remove"] = argv[++i]; }
        else if (k == "--out")      { need(i); a.out   = argv[++i]; }
        else if (k == "--label")    { need(i); a.label = argv[++i]; }
        else if (k == "--algos") {
            need(i);
            a.algos.clear();
            std::stringstream ss(argv[++i]);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (!item.empty()) a.algos.push_back(item);
            }
        } else if (k == "-h" || k == "--help") {
            std::cout <<
              "Usage: bench [--n N] [--ops K] [--seed S]\n"
              "             [--workload NAME] [--param KEY=VAL] ...\n"
              "             [--p-insert P] [--p-remove P]   # sugar for uniform\n"
              "             [--out PATH] [--label TAG]\n"
              "             [--algos nari_hdt_et,nari_hdt_naive,tomtseng_hdt]\n"
              "\nWorkloads: ";
            bool first = true;
            for (const auto& w : workload::registered_names()) {
                if (!first) std::cout << ", ";
                std::cout << w;
                first = false;
            }
            std::cout << "\n";
            std::exit(0);
        } else {
            die("unknown arg: " + k);
        }
    }
    if (a.n < 2)       die("--n must be >= 2");
    if (a.num_ops < 1) die("--ops must be >= 1");
    if (!a.seed_set)   a.seed = std::random_device{}();
    return a;
}

std::string serialize_params(const workload::Params& p) {
    std::string out;
    bool first = true;
    for (const auto& [k, v] : p) {
        if (!first) out += ';';
        out += k;
        out += '=';
        out += v;
        first = false;
    }
    return out;
}

const char* CSV_HEADER =
  "algo,workload,workload_params,n,num_ops,seed,"
  "total_ns,num_inserts,num_removes,num_queries,ns_per_op,label\n";

void write_row(std::ostream& out, const std::string& algo, const Args& a,
               const std::string& params_str, const RunResult& r) {
    const double ns_per_op = a.num_ops > 0
        ? static_cast<double>(r.total_ns) / a.num_ops
        : 0.0;
    out << algo
        << ',' << a.workload
        << ',' << params_str
        << ',' << a.n
        << ',' << a.num_ops
        << ',' << a.seed
        << ',' << r.total_ns
        << ',' << r.num_inserts
        << ',' << r.num_removes
        << ',' << r.num_queries
        << ',' << ns_per_op
        << ',' << a.label
        << '\n';
}

}  // namespace

int main(int argc, char** argv) {
    const Args args = parse_args(argc, argv);

    workload::GenFn gen = workload::lookup(args.workload);
    if (!gen) {
        std::string msg = "unknown --workload: " + args.workload + " (known: ";
        bool first = true;
        for (const auto& w : workload::registered_names()) {
            if (!first) msg += ", ";
            msg += w;
            first = false;
        }
        msg += ")";
        die(msg);
    }

    const std::string params_str = serialize_params(args.params);

    std::cerr << "bench: n=" << args.n
              << " ops=" << args.num_ops
              << " seed=" << args.seed
              << " workload=" << args.workload
              << " params=[" << params_str << "]"
              << " algos=";
    for (size_t i = 0; i < args.algos.size(); ++i) {
        if (i) std::cerr << ',';
        std::cerr << args.algos[i];
    }
    std::cerr << "\n";

    const workload::Spec spec{args.n, args.num_ops, args.seed, args.params};
    const std::vector<Op> ops = gen(spec);

    const bool need_header = !std::filesystem::exists(args.out)
        || std::filesystem::file_size(args.out) == 0;
    std::ofstream csv(args.out, std::ios::app);
    if (!csv) die("cannot open --out: " + args.out);
    if (need_header) csv << CSV_HEADER;

    for (const std::string& algo : args.algos) {
        RunResult r;
        if      (algo == "nari_hdt_et")    r = run_nari_hdt_et(args.n, ops);
        else if (algo == "nari_hdt_naive") r = run_nari_hdt_naive(args.n, ops);
        else if (algo == "tomtseng_hdt")   r = run_tomtseng_hdt(args.n, ops);
        else                               die("unknown --algos entry: " + algo);

        const double sec  = r.total_ns / 1e9;
        const double mops = sec > 0 ? (args.num_ops / sec / 1e6) : 0.0;
        std::cerr << "  " << algo
                  << ": " << r.total_ns << " ns total"
                  << " (" << mops << " Mops/s,"
                  << " ins=" << r.num_inserts
                  << " rem=" << r.num_removes
                  << " qry=" << r.num_queries << ")\n";
        write_row(csv, algo, args, params_str, r);
    }
    return 0;
}
