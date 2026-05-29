#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "matplotlib>=3.8",
#     "numpy>=1.26",
# ]
# ///
"""Sweep dynamic-connectivity benchmarks across (n, seed) and plot results.

For each n in --ns and each seed in --seeds, invokes the bench binary with
the chosen --workload and algos. Aggregates seconds/run by (algo, n) across
seeds, then plots mean with a shaded +/- 1 std band.

Default output files include the workload name so multiple workloads can
co-exist in bench/: sweep_<workload>.csv and sweep_<workload>.png.

Run:
    uv run bench/plot.py
    uv run bench/plot.py --workload sparse_random
    uv run bench/plot.py --workload build_then_query --param avg_degree=8
    uv run bench/plot.py --ns 500,1000,2000,5000,10000 --seeds 1,2,3,4,5
    uv run bench/plot.py --skip-run                       # just replot from CSV
    uv run bench/plot.py --algos nari_hdt_et,nari_hdt_naive,tomtseng_hdt
"""

import argparse
import csv
import math
import shutil
import statistics
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent

# Single physical core every bench invocation is pinned to. Matches the
# methodology committed to in docs/04_testing.typ (Benchmark methodology §).
TASKSET_CORE = "1"


def log_spaced_ns(n_min, n_max, count):
    """count log-spaced integers in [n_min, n_max], deduped, sorted."""
    if count < 2:
        return [n_min]
    log_lo = math.log10(n_min)
    log_hi = math.log10(n_max)
    step = (log_hi - log_lo) / (count - 1)
    seen = set()
    out = []
    for i in range(count):
        v = int(round(10 ** (log_lo + i * step)))
        v = max(2, v)
        if v not in seen:
            seen.add(v)
            out.append(v)
    return sorted(out)


def run_sweep(ns, seeds, ops, workload, params, algos, csv_path, label,
              build_dir, verbose, timeout):
    """Invoke bench once per (n, seed, algo) so the timeout is per-algo.

    Once an algo times out at some n, it is disabled for the rest of the sweep
    (the cost of dynamic-connectivity ops is monotone in n for our workloads,
    so larger n would only time out harder).
    """
    bin_path = build_dir / "bench" / "bench"
    if not bin_path.exists():
        sys.exit(f"bench binary not found at {bin_path}.\n"
                 f"Build it: cmake --build {build_dir} --target bench")
    taskset = shutil.which("taskset")
    if not taskset:
        sys.exit("taskset(1) not found on PATH; install util-linux or run "
                 "without core pinning by editing plot.py.")
    if csv_path.exists():
        csv_path.unlink()
    total = len(ns) * len(seeds) * len(algos)
    i = 0
    disabled = set()
    for n in ns:
        for seed in seeds:
            for algo in algos:
                i += 1
                if algo in disabled:
                    continue
                print(f"[{i:>5}/{total}] n={n:<6} seed={seed:<3} algo={algo}",
                      flush=True)
                cmd = [
                    taskset, "-c", TASKSET_CORE,
                    str(bin_path),
                    "--n", str(n),
                    "--ops", str(ops),
                    "--seed", str(seed),
                    "--workload", workload,
                    "--out", str(csv_path),
                    "--label", label,
                    "--algos", algo,
                ]
                for k, v in params.items():
                    cmd.extend(["--param", f"{k}={v}"])
                try:
                    result = subprocess.run(
                        cmd,
                        stderr=None if verbose else subprocess.PIPE,
                        stdout=None if verbose else subprocess.PIPE,
                        text=True,
                        timeout=timeout,
                    )
                except subprocess.TimeoutExpired:
                    print(f"    TIMEOUT after {timeout}s on algo='{algo}' "
                          f"at n={n} seed={seed}; disabling for rest of sweep",
                          flush=True)
                    disabled.add(algo)
                    continue
                if result.returncode != 0:
                    sys.stderr.write(result.stderr or "")
                    sys.stderr.write(result.stdout or "")
                    sys.exit(f"bench failed: {' '.join(cmd)}")


def load_rows(path):
    with open(path) as f:
        for row in csv.DictReader(f):
            row["n"] = int(row["n"])
            row["total_ns"] = float(row["total_ns"])
            yield row


def aggregate(rows):
    """Returns {(algo, n): [seconds_per_run_samples]}."""
    buckets = defaultdict(list)
    for row in rows:
        buckets[(row["algo"], row["n"])].append(row["total_ns"] / 1e9)
    return buckets


def plot(csv_path, out_png, title):
    import numpy as np
    import matplotlib.pyplot as plt
    from matplotlib.ticker import FuncFormatter, LogLocator, NullFormatter

    if not csv_path.exists():
        sys.exit(f"CSV not found: {csv_path}")

    buckets = aggregate(load_rows(csv_path))
    algos = sorted({a for a, _ in buckets})

    fig, ax = plt.subplots(figsize=(9, 5.5))
    for algo in algos:
        ns = sorted({n for a, n in buckets if a == algo})
        arrs = [np.asarray(buckets[(algo, n)]) for n in ns]
        medians = np.array([np.median(a) for a in arrs])
        q1 = np.array([np.percentile(a, 25) for a in arrs])
        q3 = np.array([np.percentile(a, 75) for a in arrs])
        line, = ax.plot(ns, medians, marker="o",
                        label=f"{algo}  (k={len(buckets[(algo, ns[0])])})")
        ax.fill_between(ns, q1, q3,
                        alpha=0.18, color=line.get_color())

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("n (vertices)")
    ax.set_ylabel("seconds per run (median over seeds, IQR shaded)")
    ax.set_title(title)
    ax.legend()
    ax.grid(True, which="both", alpha=0.3)

    # Plain decimal tick labels with a "s" suffix, e.g. "0.01 s", "0.1 s",
    # "1 s", "10 s" — avoids confusing scientific notation like "4e-2".
    ax.yaxis.set_major_locator(LogLocator(base=10.0))
    ax.yaxis.set_major_formatter(FuncFormatter(lambda y, _: f"{y:g} s"))
    ax.yaxis.set_minor_formatter(NullFormatter())

    fig.tight_layout()
    fig.savefig(out_png, dpi=150)
    print(f"wrote {out_png}", flush=True)


def parse_kv_list(items):
    """Parse a list of 'KEY=VALUE' strings into a dict."""
    out = {}
    for item in items:
        if "=" not in item:
            sys.exit(f"--param expects KEY=VALUE, got: {item}")
        k, v = item.split("=", 1)
        out[k] = v
    return out


def parse_args():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--workload", default="uniform",
                    help="workload name (uniform, sparse_random, build_then_query, ...)")
    ap.add_argument("--param", action="append", default=[],
                    help="workload param KEY=VALUE; repeatable")
    ap.add_argument("--p-insert", type=float, default=None,
                    help="convenience for uniform: sets --param p_insert=<value>")
    ap.add_argument("--p-remove", type=float, default=None,
                    help="convenience for uniform: sets --param p_remove=<value>")
    ap.add_argument("--ns", default="",
                    help="explicit comma-separated n values; "
                         "if empty, log-spaced via --n-min/--n-max/--num-n")
    ap.add_argument("--n-min", type=int, default=100)
    ap.add_argument("--n-max", type=int, default=200000)
    ap.add_argument("--num-n", type=int, default=50,
                    help="how many log-spaced n values to generate")
    ap.add_argument("--seeds", default="",
                    help="explicit comma-separated seeds; "
                         "if empty, uses 1..--num-seeds")
    ap.add_argument("--num-seeds", type=int, default=50)
    ap.add_argument("--ops", type=int, default=100000)
    ap.add_argument("--algos",
                    default="nari_hdt_et,tomtseng_hdt,nari_hdt_naive")
    ap.add_argument("--timeout", type=float, default=10.0,
                    help="seconds per bench subprocess; if an algo times out "
                         "it's skipped for the rest of the sweep")
    ap.add_argument("--csv", default=None,
                    help="defaults to bench/sweep_<workload>.csv")
    ap.add_argument("--out", default=None,
                    help="defaults to bench/sweep_<workload>.png")
    ap.add_argument("--title", default=None,
                    help="defaults to a title that includes the workload name")
    ap.add_argument("--label", default="sweep")
    ap.add_argument("--skip-run", action="store_true",
                    help="don't re-run the bench, just plot from existing CSV")
    ap.add_argument("--build-dir", default=str(REPO / "build"),
                    help="CMake build directory containing bench/bench")
    ap.add_argument("--verbose", action="store_true",
                    help="print bench stderr/stdout for every invocation")
    return ap.parse_args()


def main():
    args = parse_args()

    params = parse_kv_list(args.param)
    if args.p_insert is not None:
        params["p_insert"] = str(args.p_insert)
    if args.p_remove is not None:
        params["p_remove"] = str(args.p_remove)

    if args.ns.strip():
        ns = [int(x) for x in args.ns.split(",") if x.strip()]
    else:
        ns = log_spaced_ns(args.n_min, args.n_max, args.num_n)
    if args.seeds.strip():
        seeds = [int(x) for x in args.seeds.split(",") if x.strip()]
    else:
        seeds = list(range(1, args.num_seeds + 1))
    algos = [a.strip() for a in args.algos.split(",") if a.strip()]

    csv_path = Path(args.csv) if args.csv else REPO / "bench" / f"sweep_{args.workload}.csv"
    out_png  = Path(args.out) if args.out else REPO / "bench" / f"sweep_{args.workload}.png"
    title    = args.title or f"Dynamic connectivity: seconds per run vs n ({args.workload})"
    build_dir = Path(args.build_dir)

    if not args.skip_run:
        param_str = ",".join(f"{k}={v}" for k, v in params.items()) or "<none>"
        print(f"sweep: workload={args.workload} params={param_str} "
              f"| {len(ns)} ns ({ns[0]}..{ns[-1]}) "
              f"x {len(seeds)} seeds x {len(algos)} algos = "
              f"{len(ns) * len(seeds) * len(algos)} runs, "
              f"algos={','.join(algos)}, timeout={args.timeout}s", flush=True)
        run_sweep(ns, seeds, args.ops, args.workload, params, algos,
                  csv_path, args.label, build_dir, args.verbose, args.timeout)

    plot(csv_path, out_png, title)


if __name__ == "__main__":
    main()
