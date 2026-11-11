#!/usr/bin/env python3
"""Merge the static map with curated and upstream names.

Inputs : build/analysis/map.json (tools/analyze.py), symbols/manual.csv (curated, wins),
         symbols/upstream_refs.csv (tools/import_refs.py; names applied only at function
         entries, evidence DERIVED)
Outputs: symbols/symbols_2025-06-19.csv  address,size,name,type,evidence_level,source
         build/analysis/coverage.txt     per-area byte accounting (pasted into MEMORY_MAP.md)
"""
import csv
import json
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BASE = 0x40000000

# Coarse areas (from the map; see docs/MEMORY_MAP.md for the evidence of each boundary)
AREAS = [
    (0x40000000, 0x40000040, "vectors"),
    (0x40000040, 0x40019AF4, "fonts"),
    (0x40019AF4, 0x4006DDEC, "code"),
    (0x4006DDEC, 0x4008CB28, "graphics"),
    (0x4008CB28, 0x4008CFB8, "startup"),
    (0x4008CFB8, 0x400954A4, "rodata"),
    (0x400954A4, 0x400957E0, "packer_garbage"),
]

def area_of(a):
    for lo, hi, n in AREAS:
        if lo <= a < hi:
            return n
    return "?"


def main():
    m = json.loads((ROOT / "build/analysis/map.json").read_text())
    manual = {int(r["address"], 16): r for r in csv.DictReader(open(ROOT / "symbols/manual.csv"))}
    upstream = {int(r["address"], 16): r for r in csv.DictReader(open(ROOT / "symbols/upstream_refs.csv"))}
    funcs = {f["start"]: f for f in m["functions"]}

    rows = []
    for r in m["regions"]:
        a, e, k = r["start"], r["end"], r["kind"]
        size = e - a
        if k in ("function", "code"):
            f = funcs.get(a)
            name = f"func_{a:08x}" if k == "function" else r.get("name", f"code_{a:08x}")
            ev = r["evidence"]
            src = f"analyze.py:{f['origin']}" if f else "analyze.py"
            typ = "function" if k == "function" else "code"
            if f:
                typ += f"_{f['mode']}"
        elif k == "pool":
            name, typ, ev, src = f"pool_{a:08x}", "literal_pool", r["evidence"], "analyze.py"
        elif k == "padding":
            name, typ, ev, src = f"pad_{a:08x}", "padding", r["evidence"], f"analyze.py:{r.get('name', '')}"
        else:
            name = r.get("name") or f"{area_of(a)}_{a:08x}"
            typ, ev, src = "data", r["evidence"], "analyze.py"
            if ev == "OPEN" and area_of(a) in ("fonts", "graphics", "rodata"):
                ev = "DERIVED"   # area known, internal structure not yet
        if a in upstream and k == "function":
            u = upstream[a]
            name, ev, src = u["name"], "DERIVED", "upstream:" + u["source"]
        if a in manual:
            mm = manual[a]
            name, ev, src = mm["name"], mm["evidence_level"], mm["source"]
        rows.append((a, size, name, typ, ev, src))

    # data symbols named in manual.csv that fall inside a larger region
    have = {r[0] for r in rows}
    for a, mm in manual.items():
        if a not in have and mm["type"] == "data":
            rows.append((a, 0, mm["name"], "label", mm["evidence_level"], mm["source"]))
    rows.sort()

    out = ROOT / "symbols/symbols_2025-06-19.csv"
    with open(out, "w", newline="") as fh:
        w = csv.writer(fh, lineterminator="\n")
        w.writerow(["address", "size", "name", "type", "evidence_level", "source"])
        for a, size, name, typ, ev, src in rows:
            w.writerow([f"0x{a:08X}", size, name, typ, ev, src])

    # coverage
    total = m["size"]
    acc = defaultdict(lambda: defaultdict(int))
    for a, size, name, typ, ev, src in rows:
        if typ == "label":
            continue
        kind = typ.split("_")[0] if typ.startswith(("function", "code", "literal")) else typ
        acc[area_of(a)][kind] += size
        acc["TOTAL"][kind] += size
        acc["TOTAL_EV"][ev] += size
    covered = sum(v for k, v in acc["TOTAL"].items())
    lines = [f"image bytes: {total}  classified: {covered}  ({100 * covered / total:.2f}%)"]
    kinds = ["function", "code", "literal", "data", "padding"]
    lines.append("| area | range | function/code | literal pool | data | padding |")
    lines.append("|---|---|---:|---:|---:|---:|")
    for lo, hi, n in AREAS + [(0, 0, "TOTAL")]:
        d = acc[n]
        rng = f"`0x{lo:08X}–0x{hi:08X}`" if n != "TOTAL" else ""
        lines.append(f"| {n} | {rng} | {d['function'] + d['code']} | {d['literal']} | {d['data']} | {d['padding']} |")
    lines.append("")
    lines.append("| evidence | bytes |")
    lines.append("|---|---:|")
    for ev, b in sorted(acc["TOTAL_EV"].items()):
        lines.append(f"| {ev} | {b} |")
    nf = sum(1 for r in rows if r[3].startswith("function"))
    named = sum(1 for r in rows if r[3].startswith("function") and not r[2].startswith("func_"))
    lines.append("")
    lines.append(f"functions: {nf} (ARM {sum(1 for r in rows if r[3] == 'function_arm')}, "
                 f"Thumb {sum(1 for r in rows if r[3] == 'function_thumb')}), named: {named}")
    (ROOT / "build/analysis/coverage.txt").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))
    assert covered == total, "unclassified bytes!"


if __name__ == "__main__":
    main()
