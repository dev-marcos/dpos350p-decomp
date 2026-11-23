#!/usr/bin/env python3
"""Build a TEST image: the stock app with every EQUIVALENT C function compiled in place.

FOR THE EMULATOR ONLY. Never write a TEST image to a device. The output name always carries TEST.

For each nonmatching/**/func_XXXXXXXX.c whose tools/emu result is EQUIVALENT:
  1. compile it with GCC (the nmbuild flags, -O2) and link it AT ITS ORIGINAL ADDRESS, with every
     other symbol resolved to the original address;
  2. if code + constants fit in the original function's size, write them over the stock bytes;
  3. otherwise try -Os.
The TEST builds use VFP without NEON (the stock code has none, and neither does the browser
emulator), so each chosen build is verified again with its tools/emu cases and used only if it
is EQUIVALENT.
Skipped: functions whose stock code is Thumb (callers enter them in Thumb state), C files that
define their own variables (.data/.bss: the stock code uses the original ones), and code that
does not fit.

Output (build/test/): app_TEST.bin, DPOS350P_TEST.iso (same container layout and checksum
rules as the stock .iso), testiso_report.json.
Usage (inside dpos-re): python3 tools/testiso.py [--no-os] [-j N]
The C files are processed in parallel (one worker per CPU). Passing emulator checks of the TEST
builds are remembered in build/test/verify_cache.json, keyed like tools/emu results, and the whole
outcome per function (chosen build or reason) in build/test/process_cache.json, keyed by the C
file, headers, symbol tables, flags, both scripts, the stock size and the test suite;
--no-cache ignores the latter.
"""
import argparse
import csv
import json
import multiprocessing
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from tools import nmbuild               # noqa: E402
from tools.dposiso import pack          # noqa: E402

BASE = 0x40000000
OUT = ROOT / "build/test"


def tflags(opt):
    """nmbuild flags with VFP only (no NEON: the stock code never uses it) and the given -O level"""
    return [("-mfpu=vfpv4" if f.startswith("-mfpu=") else f) for f in nmbuild.CFLAGS if not f.startswith("-O")] + [opt]


def sizes_and_types():
    info = {}
    for r in csv.DictReader(open(ROOT / "symbols/symbols_2025-06-19.csv")):
        if r["type"].startswith("function"):
            info[int(r["address"], 16)] = (int(r["size"]), r["type"])
    return info


def compile_at(src, orig, syms, opt):
    """-> (bytes, sections dict) of src linked at orig, or raises."""
    stem = OUT / "obj" / f"func_{orig:08x}{opt}"
    stem.parent.mkdir(parents=True, exist_ok=True)
    flags = tflags(opt)
    subprocess.run([nmbuild.CC, *flags, str(src), "-o", str(stem) + ".o"], check=True, capture_output=True)
    nm = subprocess.run(["arm-none-eabi-nm", str(stem) + ".o"], capture_output=True, text=True, check=True).stdout
    defined = {l.split()[-1] for l in nm.splitlines() if re.match(r"^[0-9a-f]+ [TtDdRrBb] ", l)}
    own_vars = [l for l in nm.splitlines() if re.match(r"^[0-9a-f]+ [DdBb] ", l)]
    name = re.search(r"^\s*\*\s*(\w+) -- stock", src.read_text(), re.M).group(1)
    lds = [f"ENTRY({name})", "SECTIONS {",
           f"  .text 0x{orig:08X} : SUBALIGN(4) {{ KEEP(*(.text.{name})) *(.text*) *(.rodata*) }}",
           "  .data : { *(.data*) *(.bss*) *(COMMON) }",
           "  /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }", "}"]
    undef = sorted({l.split()[-1] for l in nm.splitlines() if re.match(r"^\s+U ", l)})
    lds += [f"{k} = 0x{syms[k]:08X};" for k in undef if k in syms]   # only what the object uses
    Path(str(stem) + ".ld").write_text("\n".join(lds) + "\n")
    subprocess.run(["arm-none-eabi-ld", "-q", "-T", str(stem) + ".ld", "-o", str(stem) + ".elf", str(stem) + ".o"],
                   check=True, capture_output=True)
    subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text", str(stem) + ".elf", str(stem) + ".bin"],
                   check=True)
    bad = nmbuild.fix_thumb_calls(str(stem) + ".elf", str(stem) + ".bin", orig, syms)
    if bad:
        raise subprocess.CalledProcessError(1, "link", stderr=("B/BL<cond> into Thumb code: " + ", ".join(bad)).encode())
    st = subprocess.run(["arm-none-eabi-nm", str(stem) + ".elf"], capture_output=True, text=True).stdout
    entry = int(next(l.split()[0] for l in st.splitlines() if l.split()[-1] == name), 16)
    return Path(str(stem) + ".bin").read_bytes(), entry, own_vars, name


VCACHE = OUT / "verify_cache.json"


def verify(src, orig, syms, opt, vcache=None):
    """Builds src with the TEST flags as the scratch test image and runs its tools/emu cases.
    The normal nmbuild image is rebuilt afterwards. A passing result is remembered in vcache under
    the tools/emu result key of the TEST build, so an unchanged function is not emulated again.
    -> (ok, why, new cache key or None)"""
    from tools.emu import run_tests as rt
    import hashlib
    import importlib
    case_fn = None
    for s in rt.SUITES:
        mod = importlib.import_module(s)
        if orig in mod.TESTS:
            case_fn = mod.TESTS[orig]
            n = 40 if s.endswith("fatfs_fs") else 200
            suite = s
            break
    if case_fn is None:
        return False, "no tests", None
    saved = list(nmbuild.CFLAGS)
    newkey = None
    try:
        nmbuild.CFLAGS[:] = tflags(opt)
        nmbuild.build(src, syms)
        core = rt._hash_files(rt.CORE)
        suite_hash = hashlib.sha256((core + rt._hash_files([suite.replace(".", "/") + ".py"])).encode()).hexdigest()
        key = rt.result_key(orig, suite_hash, n, 20250619)
        if vcache is not None and vcache.get(key):
            fails = []
        else:
            _, fails, _ = rt.run_one(orig, case_fn, n, 20250619)
            if not fails:
                newkey = key
    finally:
        nmbuild.CFLAGS[:] = saved
        nmbuild.build(src, syms)
    return (not fails), (fails[0] if fails else ""), newkey


_W = {}


PCACHE = OUT / "process_cache.json"


def process_key(src, orig, size, typ, no_os):
    """everything a process() outcome depends on: the nmbuild link key of the function (C file,
    headers, flags, nmbuild.py, resolved symbols), this script, the stock size/type, the option and
    the test suite"""
    import hashlib
    import importlib
    from tools.emu import run_tests as rt
    suite_hash = ""
    for s in rt.SUITES:
        if orig in importlib.import_module(s).TESTS:
            suite_hash = rt._hash_files(rt.CORE + [s.replace(".", "/") + ".py"])
            break
    # the nmbuild link key covers the C file, headers, flags, nmbuild.py and the resolved symbols
    nmkey = json.loads((ROOT / f"build/nonmatching/func_{orig:08x}.json").read_text()).get("key", "")
    h = hashlib.sha256(f"{nmkey}|{size}|{typ}|{no_os}|{suite_hash}|".encode())
    h.update(Path(__file__).read_bytes())
    h.update(src.read_bytes())
    return h.hexdigest()


def process(src):
    """one C file -> (rec, chosen (opt, code) or None, new verify-cache keys, process-cache entry);
    runs in a worker; an unchanged function is taken from the process cache"""
    a, equiv, info, syms, vcache, pcache = _W["a"], _W["equiv"], _W["info"], _W["syms"], _W["vcache"], _W["pcache"]
    orig = int(re.search(r"func_([0-9a-f]{8})\.c$", src.name).group(1), 16)
    if orig not in equiv:
        return None, None, [], None
    size, typ = info.get(orig, (0, ""))
    pkey = process_key(src, orig, size, typ, a.no_os)
    hit = pcache.get(f"0x{orig:08X}")
    if hit and hit["key"] == pkey:
        ch = (hit["opt"], bytes.fromhex(hit["code"])) if hit.get("opt") else None
        return hit["rec"], ch, [], None
    rec, chosen, keys = _process(src, orig, size, typ, a, syms, vcache)
    ent = {"key": pkey, "rec": rec, "opt": chosen[0] if chosen else None, "code": chosen[1].hex() if chosen else ""}
    return rec, chosen, keys, (f"0x{orig:08X}", ent)


def _process(src, orig, size, typ, a, syms, vcache):
    rec = {"addr": f"0x{orig:08X}", "size": size, "src": str(src.relative_to(ROOT))}
    if typ == "function_thumb":
        rec["why"] = "stock code is Thumb"
        return rec, None, []
    chosen, keys = None, []
    for opt in (["-O2"] if a.no_os else ["-O2", "-Os"]):
        try:
            code, entry, own_vars, name = compile_at(src, orig, syms, opt)
        except subprocess.CalledProcessError as e:
            rec["why"] = f"build failed ({opt}): {e.stderr.decode()[-200:] if e.stderr else e}"; break
        rec["name"] = name
        if own_vars:
            rec["why"] = "defines its own variables"; break
        if entry != orig:
            rec["why"] = f"entry not at the start ({opt})"; break
        if len(code) > size:
            rec[f"bytes{opt}"] = len(code); continue
        ok, why, newkey = verify(src, orig, syms, opt, vcache)
        if newkey:
            keys.append(newkey)
        if not ok:
            rec["why"] = f"{opt} build not equivalent: {why}"; break
        chosen = (opt, code)
        break
    if not chosen:
        rec.setdefault("why", "does not fit")
    return rec, chosen, keys


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--no-os", action="store_true", help="only use the verified -O2 builds")
    ap.add_argument("-j", type=int, default=os.cpu_count() or 1, help="worker processes")
    ap.add_argument("--no-cache", action="store_true", help="redo every function (ignore process_cache.json)")
    a = ap.parse_args()
    app = bytearray((ROOT / "build/extract/app.bin").read_bytes())
    logo = (ROOT / "build/extract/logo.rgb565").read_bytes()
    results = json.loads((ROOT / "build/emu/results.json").read_text())
    equiv = {int(k, 0): v for k, v in results.items() if v.get("status") == "EQUIVALENT"}
    info = sizes_and_types()
    syms = nmbuild.original_symbols()
    report = {"patched": [], "skipped": []}
    vcache = json.loads(VCACHE.read_text()) if VCACHE.exists() else {}
    pcache = json.loads(PCACHE.read_text()) if PCACHE.exists() and not a.no_cache else {}
    _W.update(a=a, equiv=equiv, info=info, syms=syms, vcache=vcache, pcache=pcache)
    srcs = sorted(ROOT.glob("nonmatching/**/func_*.c"))
    with multiprocessing.get_context("fork").Pool(max(1, a.j)) as pool:
        outs = pool.map(process, srcs, chunksize=4)
    for rec, chosen, keys, ent in outs:
        for k in keys:
            vcache[k] = True
        if ent:
            pcache[ent[0]] = ent[1]
        if rec is None:
            continue
        if chosen:
            opt, code = chosen
            orig = int(rec["addr"], 16)
            app[orig - BASE:orig - BASE + len(code)] = code
            report["patched"].append({**rec, "opt": opt, "bytes": len(code)})
        else:
            report["skipped"].append(rec)
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "app_TEST.bin").write_bytes(app)
    iso = pack(logo, bytes(app))
    (OUT / "DPOS350P_TEST.iso").write_bytes(iso)
    p = report["patched"]
    report["summary"] = {
        "patched": len(p), "patched_bytes": sum(x["size"] for x in p),
        "with_O2": sum(1 for x in p if x["opt"] == "-O2"), "with_Os": sum(1 for x in p if x["opt"] == "-Os"),
        "skipped": len(report["skipped"]),
    }
    (OUT / "testiso_report.json").write_text(json.dumps(report, indent=1))
    VCACHE.write_text(json.dumps(vcache, indent=0, sort_keys=True))
    PCACHE.write_text(json.dumps(pcache, indent=0, sort_keys=True))
    s = report["summary"]
    print(f"patched {s['patched']} functions ({s['patched_bytes']} stock bytes): {s['with_O2']} with -O2, {s['with_Os']} with -Os")
    why = {}
    for x in report["skipped"]:
        k = x["why"].split(":")[0].split(" (")[0]
        why[k] = why.get(k, 0) + 1
    print("skipped:", ", ".join(f"{k}: {v}" for k, v in sorted(why.items(), key=lambda kv: -kv[1])))
    print(f"-> {OUT.relative_to(ROOT)}/DPOS350P_TEST.iso (EMULATOR ONLY, never write it to a device)")


if __name__ == "__main__":
    main()
