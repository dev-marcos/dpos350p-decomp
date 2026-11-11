#!/usr/bin/env python3
"""Run behavioural equivalence tests: stock function vs nonmatching C build.

Usage (inside dpos-re, after tools/nmbuild.py):
  python3 -m tools.emu.run_tests [--only 0x40056A70] [--cases N] [--seed S]
Writes build/emu/results.json (consumed by tools/progress.py).
"""
import argparse
import importlib
import json
import random
import sys
from pathlib import Path

from tools.emu.emu import Machine, load_c_build, outcome_equal

ROOT = Path(__file__).resolve().parents[2]
SUITES = ["tools.emu.tests.fatfs"]


def run_one(orig, case_fn, cases, seed):
    info, code = load_c_build(orig)
    rng = random.Random(seed ^ orig)
    fails = []
    for i in range(cases):
        st = rng.getstate()
        c = case_fn(rng)
        res = []
        for entry in (orig, info["entry"]):
            m = Machine(code=code, stubs=c.stubs)
            if c.setup:
                c.setup(m)
            res.append(m.call(entry, c.args, c.stack))
        d = outcome_equal(res[0], res[1], c.ret)
        if d:
            fails.append({"case": i, "args": [hex(a) for a in c.args], "diff": d})
            if len(fails) >= 5:
                break
    return info, fails


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", type=lambda s: int(s, 0))
    ap.add_argument("--cases", type=int)
    ap.add_argument("--seed", type=int, default=20250619)
    a = ap.parse_args()
    out = ROOT / "build/emu/results.json"
    out.parent.mkdir(parents=True, exist_ok=True)
    results = json.loads(out.read_text()) if out.exists() else {}
    bad = 0
    for suite in SUITES:
        mod = importlib.import_module(suite)
        for orig, fn in mod.TESTS.items():
            if a.only and orig != a.only:
                continue
            if not (ROOT / f"build/nonmatching/func_{orig:08x}.json").exists():
                print(f"0x{orig:08x}: no C build (run tools/nmbuild.py)")
                continue
            n = a.cases or getattr(mod, "N", 200)
            info, fails = run_one(orig, fn, n, a.seed)
            status = "EQUIVALENT" if not fails else "DIFFERENT"
            bad += bool(fails)
            results[f"0x{orig:08X}"] = {"name": info["name"], "status": status, "cases": n,
                                        "seed": a.seed, "src": info["src"], "fails": fails}
            print(f"{status:<10} 0x{orig:08x} {info['name']:<14} {n} cases" +
                  (f"  first: {fails[0]['diff']}" if fails else ""))
    out.write_text(json.dumps(results, indent=1, sort_keys=True))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
