#!/usr/bin/env python3
"""Find the C source of each stock function.

Every C file under nonmatching/ (and src/, for MATCHING code) starts with a header line
    * <name> -- stock 0x<address> (<size> bytes, ARM)
The address in that line, not the file name, says which stock function the file replaces,
so files can be named and grouped for people.
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = re.compile(r"^\s*\*\s*(\w+) -- stock 0x([0-9A-Fa-f]{8})", re.M)


def orig_of(path):
    """stock address that the C file replaces"""
    m = HEADER.search(Path(path).read_text(errors="replace")[:4096])
    if not m:
        raise SystemExit(f"{path}: no '<name> -- stock 0x<address>' header line")
    return int(m.group(2), 16)


def name_of(path):
    return HEADER.search(Path(path).read_text(errors="replace")[:4096]).group(1)


def sources(tree="nonmatching"):
    """{stock address: Path} for every C file under the tree"""
    out = {}
    for p in sorted((ROOT / tree).glob("**/*.c")):
        a = orig_of(p)
        if a in out:
            raise SystemExit(f"{p}: 0x{a:08X} is already in {out[a]}")
        out[a] = p
    return out


def source(orig, tree="nonmatching"):
    return sources(tree)[orig]
