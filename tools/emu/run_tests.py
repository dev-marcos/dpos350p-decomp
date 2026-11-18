#!/usr/bin/env python3
"""Run behavioural equivalence tests: stock function vs nonmatching C build.

Usage (inside dpos-re, after tools/nmbuild.py):
  python3 -m tools.emu.run_tests [--only 0x40056A70 ...] [--cases N] [--seed S] [-j N] [--no-cache]
Writes build/emu/results.json (consumed by tools/progress.py).

Functions run in parallel worker processes (one per CPU by default). Every case draws from
random.Random(seed ^ addr), so the outcome does not depend on the order or the worker.

Cache: each result stores a key, a SHA-256 of the function's C build (.bin + entry), the test
suite file, the harness core (emu.py, fatdisk.py, this file, tests/__init__.py, tests/fatfs.py),
the stock app.bin, the case count and the seed. A function whose key is unchanged keeps its
result without running again; --no-cache runs everything.
"""
import argparse
import hashlib
import importlib
import json
import multiprocessing
import os
import random
import sys
from pathlib import Path

from tools.emu.emu import Machine, load_c_build, outcome_equal

ROOT = Path(__file__).resolve().parents[2]
SUITES = ["tools.emu.tests.fatfs", "tools.emu.tests.fatfs_fs", "tools.emu.tests.rtlib", "tools.emu.tests.drivers", "tools.emu.tests.usb", "tools.emu.tests.platform", "tools.emu.tests.fpga", "tools.emu.tests.sweep", "tools.emu.tests.heavy"]
CORE = ["tools/emu/emu.py", "tools/emu/fatdisk.py", "tools/emu/run_tests.py", "tools/emu/tests/__init__.py",
        "tools/emu/tests/fatfs.py", "build/extract/app.bin"]


_THUMB = None


def stock_entry(orig):
    """Stock entry address with the Thumb bit set for Thumb functions (from the Phase 2 map)."""
    global _THUMB
    if _THUMB is None:
        m = json.loads((ROOT / "build/analysis/map.json").read_text())
        _THUMB = {f["start"] for f in m["functions"] if f["mode"] == "thumb"}
    return orig | (1 if orig in _THUMB else 0)


def run_one(orig, case_fn, cases, seed):
    info, code = load_c_build(orig)
    stock = stock_entry(orig)
    rng = random.Random(seed ^ orig)
    fails = []
    crashed = 0
    for i in range(cases):
        st = rng.getstate()
        c = case_fn(rng)
        res = []
        for entry in (stock, info["entry"]):
            if getattr(c, "build", None):          # scenario: machine prepared by stock calls
                m = c.build(code)
            else:
                m = Machine(code=code, stubs=c.stubs)
                if c.setup:
                    c.setup(m)
            res.append(m.call(entry, c.args, c.stack, getattr(c, 'dregs', None)))
        d = outcome_equal(res[0], res[1], c.ret)
        if not d and res[0]["error"]:
            crashed += 1
        if d:
            fails.append({"case": i, "args": [hex(a) for a in c.args], "diff": d})
            if len(fails) >= 5:
                break
    return info, fails, crashed


def _hash_files(paths):
    h = hashlib.sha256()
    for p in paths:
        f = ROOT / p
        h.update(p.encode() + b"\0" + (f.read_bytes() if f.exists() else b"<missing>"))
    return h.hexdigest()


def result_key(orig, suite_hash, n, seed):
    stem = ROOT / f"build/nonmatching/func_{orig:08x}"
    info = json.loads(Path(str(stem) + ".json").read_text())
    h = hashlib.sha256(f"{suite_hash}|{n}|{seed}|{info['entry']}|".encode())
    h.update(Path(str(stem) + ".bin").read_bytes())
    return h.hexdigest()


def _job(task):
    """worker: (suite, orig, n, seed) -> (orig, info, fails, crashed)"""
    suite, orig, n, seed = task
    fn = importlib.import_module(suite).TESTS[orig]
    try:
        info, fails, crashed = run_one(orig, fn, n, seed)
    except Exception as e:                   # a broken test must not take the whole run down
        info, _ = load_c_build(orig)
        fails, crashed = [{"case": -1, "diff": [f"harness error: {type(e).__name__}: {e}"]}], 0
    return orig, info, fails, crashed


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", type=lambda s: int(s, 0), nargs="+")
    ap.add_argument("--cases", type=int)
    ap.add_argument("--seed", type=int, default=20250619)
    ap.add_argument("-j", type=int, default=os.cpu_count() or 1, help="worker processes")
    ap.add_argument("--no-cache", action="store_true", help="run every test even when its key is unchanged")
    a = ap.parse_args()
    out = ROOT / "build/emu/results.json"
    out.parent.mkdir(parents=True, exist_ok=True)
    results = json.loads(out.read_text()) if out.exists() else {}
    core = _hash_files(CORE)

    tasks, order, cached = [], [], {}
    for suite in SUITES:
        mod = importlib.import_module(suite)
        suite_hash = hashlib.sha256((core + _hash_files([suite.replace(".", "/") + ".py"])).encode()).hexdigest()
        for orig in mod.TESTS:
            if a.only and orig not in a.only:
                continue
            if not (ROOT / f"build/nonmatching/func_{orig:08x}.json").exists():
                print(f"0x{orig:08x}: no C build (run tools/nmbuild.py)")
                continue
            n = a.cases or getattr(mod, "N", 200)
            key = result_key(orig, suite_hash, n, a.seed)
            order.append((orig, n, key))
            old = results.get(f"0x{orig:08X}")
            if not a.no_cache and old and old.get("key") == key and old.get("status"):
                cached[orig] = old
            else:
                tasks.append((suite, orig, n, a.seed))

    done = {}
    if tasks:
        jobs = max(1, min(a.j, len(tasks)))
        if jobs == 1:
            for t in tasks:
                r = _job(t)
                done[r[0]] = r
        else:
            with multiprocessing.get_context("fork").Pool(jobs) as pool:
                for r in pool.imap_unordered(_job, tasks):
                    done[r[0]] = r

    bad = 0
    for orig, n, key in order:
        k = f"0x{orig:08X}"
        if orig in cached:
            r = cached[orig]
            bad += r["status"] != "EQUIVALENT"
            print(f"{r['status']:<10} 0x{orig:08x} {r['name']:<14} {n} cases (cached)")
            continue
        _, info, fails, crashed = done[orig]
        if not fails and crashed > n // 2:
            fails = [{"case": -1, "diff": [f"{crashed}/{n} cases fault in both versions: test does not exercise the function"]}]
        status = "EQUIVALENT" if not fails else "DIFFERENT"
        bad += bool(fails)
        results[k] = {"name": info["name"], "status": status, "cases": n, "both_fault": crashed,
                      "seed": a.seed, "src": info["src"], "fails": fails, "key": key}
        print(f"{status:<10} 0x{orig:08x} {info['name']:<14} {n} cases" + (f" ({crashed} both-fault)" if crashed else "") +
              (f"  first: {fails[0]['diff']}" if fails else ""))
    out.write_text(json.dumps(results, indent=1, sort_keys=True))
    print(f"{len(order)} functions: {len(order) - len(cached)} run, {len(cached)} cached, {bad} not EQUIVALENT")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
