#!/usr/bin/env python3
"""Harvest "0x400xxxxx  description" listings from the read-only reference repos.

Writes symbols/upstream_refs.csv: address,name,description,source
Names: an explicit identifier given upstream (e.g. sys_clock_init, FPGA_Start) is kept;
otherwise a slug of the description is used. Everything harvested here is an upstream
claim, so it enters symbols_*.csv as DERIVED until verified in this project.
"""
import csv, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
REFS = ROOT / "third_party"
LINE = re.compile(r"^\s*(?:\|\s*)?`?(0x400[0-9A-Fa-f]{5})`?\s*(?:\||:)?\s+(?:BL\s+(0x400[0-9A-Fa-f]{5})\s+)?(.+?)\s*\|?\s*$")
IDENT = re.compile(r"^`?\"?([A-Za-z_][A-Za-z0-9_]*)(?:\(\))?\"?`?(?:\s|\(|$)")
SKIP_WORDS = {"call", "stock", "bl", "the", "a", "an", "branch", "reaches", "decomposed",
              "required", "payload", "main", "updater", "successfully", "with"}

def slug(s):
    s = re.sub(r"\(.*?\)", "", s.lower())
    s = re.sub(r"[^a-z0-9]+", "_", s).strip("_")
    return "_".join(s.split("_")[:6]) or None

rows = {}
for path in sorted(REFS.rglob("*")):
    if ".git" in path.parts or path.suffix not in (".md", ".h", ".c", ".S", ".txt"):
        continue
    for n, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
        m = LINE.match(line)
        if not m:
            continue
        addr = int(m.group(2) or m.group(1), 16)
        desc = m.group(3).strip().strip("|").strip()
        # drop prose fragments ("/ 0x... ...", "passes ... `0x...`", trailing ":")
        if desc.startswith(("/", "0x")) or "`0x" in desc or desc.endswith(":") or len(desc) > 90:
            continue
        if m.group(2):  # "callsite  BL target  name()" form: the name belongs to the target
            pass
        idm = IDENT.match(desc)
        ident = idm.group(1) if idm else None
        if ident and (ident.lower() in SKIP_WORDS or "_" not in ident):
            ident = None
        name = ident or slug(desc)
        if not name:
            continue
        src = f"{path.relative_to(REFS).as_posix()}:{n}"
        rows.setdefault(addr, []).append((name, desc[:120], src, bool(ident)))

out = ROOT / "symbols" / "upstream_refs.csv"
with open(out, "w", newline="") as f:
    w = csv.writer(f, lineterminator="\n")
    w.writerow(["address", "name", "description", "source", "all_sources"])
    for addr in sorted(rows):
        cands = rows[addr]
        best = next((c for c in cands if c[3]), cands[0])  # prefer explicit identifiers
        w.writerow([f"0x{addr:08X}", best[0], best[1], best[2], ";".join(c[2] for c in cands)])
print(f"{len(rows)} addresses -> {out}")
