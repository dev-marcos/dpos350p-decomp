#!/usr/bin/env python3
"""Unicorn harness: run a stock function and its rewritten C version from the same state
and compare their observable behaviour.

Observable behaviour of one call =
  * return registers (masked to the declared return width)
  * final value of every byte written outside the callee's own stack frame
  * the ordered list of calls into stubbed functions (name + argument registers)

Memory map of the emulated machine:
  0x40000000 64 MiB  DRAM, stock image loaded at its base (ZI and stacks above it)
  0x48000000  1 MiB  scratch: the C build of the function under test (tools/nmbuild.py)
  0x4FFF0000 64 KiB  return trampoline (execution stops here)
  0x01C00000  1 MiB  peripheral window (RAM-backed; accesses are logged)
"""
import json
import struct
from pathlib import Path

from unicorn import Uc, UcError, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn import arm_const as R

ROOT = Path(__file__).resolve().parents[2]
BASE = 0x40000000
DRAM = 64 << 20
SCRATCH = 0x48000000
STOP = 0x4FFF0000
MMIO = 0x01C00000
STACK_TOP = 0x43F00000          # free RAM above the stock stacks (stock IRQ top 0x43068D28)
STACK_GUARD = 0x10000           # writes in [sp-64K, STACK_TOP) are the callee's frame

ARGREGS = [R.UC_ARM_REG_R0, R.UC_ARM_REG_R1, R.UC_ARM_REG_R2, R.UC_ARM_REG_R3]

_IMAGE = None


def image():
    global _IMAGE
    if _IMAGE is None:
        _IMAGE = (ROOT / "build/extract/app.bin").read_bytes()
    return _IMAGE


class Stub:
    """Python replacement for a stock function (I/O, drivers). fn(machine, args)->r0[,r1]."""
    def __init__(self, name, addr, fn, nargs=4):
        self.name, self.addr, self.fn, self.nargs = name, addr, fn, nargs


class Machine:
    def __init__(self, code=None, stubs=(), max_insns=2_000_000):
        uc = Uc(UC_ARCH_ARM, UC_MODE_ARM)
        try:
            from unicorn import UC_CTL_CPU_MODEL
            from unicorn.arm_const import UC_CPU_ARM_CORTEX_A7
            uc.ctl_set_cpu_model(UC_CPU_ARM_CORTEX_A7)
        except Exception:
            pass
        uc.mem_map(BASE, DRAM)
        uc.mem_map(SCRATCH, 1 << 20)
        uc.mem_map(STOP, 0x10000)
        uc.mem_map(MMIO, 1 << 20)
        uc.mem_write(BASE, image())
        if code:
            uc.mem_write(SCRATCH, code)
        uc.mem_write(STOP, b"\xfe\xff\xff\xea" * 4)   # b . (never executed: emu stops at STOP)
        # same FPU state as stock vfp_enable (0x4008CB94): CPACR cp10/cp11 full, FPEXC.EN,
        # FPSCR = 0x02000000 (default NaN)
        uc.reg_write(R.UC_ARM_REG_C1_C0_2, uc.reg_read(R.UC_ARM_REG_C1_C0_2) | (0xF << 20))
        uc.reg_write(R.UC_ARM_REG_FPEXC, 0x40000000)
        uc.reg_write(R.UC_ARM_REG_FPSCR, 0x02000000)
        self.uc = uc
        self.max_insns = max_insns
        self.calls = []
        self.writes = set()
        self.stubs = {s.addr: s for s in stubs}
        for s in stubs:
            uc.hook_add(UC_HOOK_CODE, self._stub_hook, s, s.addr, s.addr)
        uc.hook_add(UC_HOOK_MEM_WRITE, self._write_hook)

    # -- helpers for test setup
    def w8(self, a, v): self.uc.mem_write(a, bytes([v & 0xFF]))
    def w32(self, a, v): self.uc.mem_write(a, struct.pack("<I", v & 0xFFFFFFFF))
    def write(self, a, b): self.uc.mem_write(a, bytes(b))
    def read(self, a, n): return bytes(self.uc.mem_read(a, n))
    def r32(self, a): return struct.unpack("<I", self.read(a, 4))[0]
    def reg(self, r): return self.uc.reg_read(r)

    def _stub_hook(self, uc, addr, size, stub):
        args = [uc.reg_read(r) for r in ARGREGS[:stub.nargs]]
        self.calls.append((stub.name, tuple(args)))
        ret = stub.fn(self, args)
        if isinstance(ret, tuple):
            uc.reg_write(R.UC_ARM_REG_R0, ret[0] & 0xFFFFFFFF)
            uc.reg_write(R.UC_ARM_REG_R1, ret[1] & 0xFFFFFFFF)
        elif ret is not None:
            uc.reg_write(R.UC_ARM_REG_R0, ret & 0xFFFFFFFF)
        lr = uc.reg_read(R.UC_ARM_REG_LR)
        if lr & 1:
            cpsr = uc.reg_read(R.UC_ARM_REG_CPSR) | 0x20
            uc.reg_write(R.UC_ARM_REG_CPSR, cpsr)
        uc.reg_write(R.UC_ARM_REG_PC, lr & ~1)

    def _write_hook(self, uc, access, addr, size, value, user):
        if STACK_TOP - STACK_GUARD <= addr < STACK_TOP:
            return
        for k in range(size):
            self.writes.add(addr + k)

    def call(self, entry, args=(), stack_args=(), dregs=None):
        """dregs: {n: raw 64-bit value} preloaded into VFP d<n> (hard-float arguments)."""
        uc = self.uc
        for n, v in (dregs or {}).items():
            uc.reg_write(R.UC_ARM_REG_D0 + n, v & 0xFFFFFFFFFFFFFFFF)
        sp = STACK_TOP - 0x100
        # Uninitialised locals must read the same value in both versions (their frames differ):
        # clear the callee's stack area before every measured call.
        uc.mem_write(STACK_TOP - STACK_GUARD, bytes(STACK_GUARD))
        for i, v in enumerate(stack_args):
            uc.mem_write(sp + 4 * i, struct.pack("<I", v & 0xFFFFFFFF))
        for r, v in zip(ARGREGS, args):
            uc.reg_write(r, v & 0xFFFFFFFF)
        uc.reg_write(R.UC_ARM_REG_SP, sp)
        uc.reg_write(R.UC_ARM_REG_LR, STOP)
        thumb = entry & 1
        cpsr = (uc.reg_read(R.UC_ARM_REG_CPSR) & ~0x20) | (0x20 if thumb else 0)
        uc.reg_write(R.UC_ARM_REG_CPSR, cpsr)
        self.writes.clear()
        self.calls.clear()
        err = None
        try:
            uc.emu_start(entry | thumb, STOP, count=self.max_insns)
        except UcError as e:
            err = str(e)
        pc = uc.reg_read(R.UC_ARM_REG_PC)
        if err is None and pc != STOP:
            err = f"did not return (pc=0x{pc:08x}, instruction budget exhausted)"
        r0, r1 = uc.reg_read(R.UC_ARM_REG_R0), uc.reg_read(R.UC_ARM_REG_R1)
        r2, r3 = uc.reg_read(R.UC_ARM_REG_R2), uc.reg_read(R.UC_ARM_REG_R3)
        d0 = uc.reg_read(R.UC_ARM_REG_D0)
        mem = {a: self.read(a, 1)[0] for a in sorted(self.writes)}
        return {"r0": r0, "r1": r1, "r2": r2, "r3": r3, "d0": d0, "mem": mem, "calls": list(self.calls),
                "error": err, "m": self}


def load_c_build(orig):
    stem = ROOT / f"build/nonmatching/func_{orig:08x}"
    info = json.loads(Path(str(stem) + ".json").read_text())
    return info, Path(str(stem) + ".bin").read_bytes()


def outcome_equal(a, b, ret_bits):
    """ret_bits: 0 (void), 8/16/32 (r0), 64 (r0:r1), 128 (r0..r3), 'd' (VFP d0, bit-exact), 's' (s0)."""
    diffs = []
    if ret_bits in ("d", "s"):
        if a["error"] != b["error"]:
            return [f"error: stock={a['error']} c={b['error']}"]
        if a["error"]:
            return []
        ma = a["d0"] if ret_bits == "d" else a["d0"] & 0xFFFFFFFF
        mb = b["d0"] if ret_bits == "d" else b["d0"] & 0xFFFFFFFF
        if ma != mb:
            diffs.append(f"{ret_bits}0: stock=0x{ma:x} c=0x{mb:x}")
        ret_bits = 0
    mask = (1 << min(ret_bits, 32)) - 1 if ret_bits else 0
    if a["error"] or b["error"]:
        if a["error"] != b["error"]:
            diffs.append(f"error: stock={a['error']} c={b['error']}")
        else:
            return []      # both fault identically: undefined-state input, counted by the runner
    if ret_bits and (a["r0"] & mask) != (b["r0"] & mask):
        diffs.append(f"r0: stock=0x{a['r0'] & mask:x} c=0x{b['r0'] & mask:x}")
    if ret_bits > 32 and a["r1"] != b["r1"]:
        diffs.append(f"r1: stock=0x{a['r1']:x} c=0x{b['r1']:x}")
    if ret_bits > 64 and (a["r2"], a["r3"]) != (b["r2"], b["r3"]):
        diffs.append(f"r2:r3: stock=0x{a['r3']:x}:{a['r2']:08x} c=0x{b['r3']:x}:{b['r2']:08x}")
    # final memory state over every byte either version wrote (outside its own frame)
    bad = []
    for k in sorted(set(a["mem"]) | set(b["mem"])):
        va = a["mem"][k] if k in a["mem"] else a["m"].read(k, 1)[0]
        vb = b["mem"][k] if k in b["mem"] else b["m"].read(k, 1)[0]
        if va != vb:
            bad.append(k)
    if bad:
        diffs.append(f"memory differs at {[hex(x) for x in bad[:6]]}{' ...' if len(bad) > 6 else ''}")
    if a["calls"] != b["calls"]:
        diffs.append(f"calls: stock={a['calls'][:3]} c={b['calls'][:3]}")
    return diffs
