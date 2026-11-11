"""Behavioural tests for low-level drivers: return values, memory and the ordered MMIO log."""
from tools.emu.tests.fatfs import Case

N = 300
PORTS = [0x01C20800 + 0x24 * n for n in range(7)]
PE, FLAG = 0x01C20890, 0x42E68D18


def regs(rng, base, n_words):
    vals = [rng.getrandbits(32) for _ in range(n_words)]
    return lambda m: [m.w32(base + 4 * i, v) for i, v in enumerate(vals)]


def compose(*fns):
    return lambda m: [f(m) for f in fns]


def pio_case(argfn, ret=0):
    def case(rng):
        port = rng.choice(PORTS)
        return Case([port] + argfn(rng), regs(rng, port, 9), ret=ret)
    return case


def pin(rng):
    return rng.choice([0, 1, 7, 8, 15, 16, 21, 31, rng.randrange(32)])


def t_readl(rng):
    a = rng.choice([0x01C20800, 0x01C20CB8, 0x01C0F000, 0x01000000, 0x41000000])
    return Case([a], regs(rng, a, 1), ret=32)


def t_writel(rng):
    a = rng.choice([0x01C20800, 0x01C20CB8, 0x01C0F000, 0x41000000])
    return Case([a, rng.getrandbits(32)], ret=0)


def fpga_state(rng):
    flag = rng.choice([0, 1])
    return compose(regs(rng, PE, 9), lambda m: m.w8(FLAG, flag))


def t_fpga(nargs, ret=0):
    def case(rng):
        return Case([rng.getrandbits(8) if rng.random() < 0.7 else rng.getrandbits(32)][:nargs], fpga_state(rng), ret=ret)
    return case


def t_sd_pins(rng):
    return Case([rng.choice([0, 1, 2, 3, -1 & 0xFFFFFFFF])],
                compose(regs(rng, 0x01C20848, 9), regs(rng, 0x01C208B4, 9)), ret=0)


def wdt_state(rng):
    return regs(rng, 0x01C20CA0, 7)


def t_wdt_config(rng):
    return Case([rng.randrange(16), rng.randrange(4), rng.choice([0, 1, 2])], wdt_state(rng), ret=32)


def t_wdt(ret):
    def case(rng):
        return Case([], wdt_state(rng), ret=ret)
    return case


TESTS = {
    0x400631D4: pio_case(lambda r: [pin(r), r.choice([0, 1, 2, 3, 6, 7, 0xF]), r.randrange(4)]),
    0x40063274: pio_case(lambda r: [pin(r)]), 0x40063288: pio_case(lambda r: [pin(r)]),
    0x400632A0: pio_case(lambda r: [r.choice([pin(r), 32, 40, 255])], ret=32),
    0x400632B8: pio_case(lambda r: [pin(r), r.choice([0, 1, 2, 3, 4, 9, 0xFFFFFFFF])]),
    0x40067598: t_readl, 0x40067478: t_readl, 0x40068E64: t_readl,
    0x400675A0: t_writel, 0x40067480: t_writel, 0x40068E6C: t_writel,
    0x400490DC: t_fpga(0), 0x400490F4: t_fpga(0), 0x40049110: t_fpga(0),
    0x400491B8: t_fpga(0, ret=32), 0x40049214: t_fpga(1), 0x4004927C: t_fpga(1),
    0x400675B0: t_sd_pins,
    0x40067488: t_wdt_config, 0x4006750C: t_wdt(0), 0x4006752C: t_wdt(32), 0x40067574: t_wdt(0),
}


# ---------------------------------------------------------------- GT911 bit-banged I2C
import random as _random
from tools.emu.emu import Stub

PG, PG_DAT = 0x01C208D8, 0x01C208E8
DELAY_US, DELAY_MS = 0x40042D0C, 0x40042CF0


def delay_stubs():
    return (Stub("delay_us", DELAY_US, lambda m, a: None, 1), Stub("delay_ms", DELAY_MS, lambda m, a: None, 1))


def sda_model(seed, p_high):
    """PG_DAT reads: keep the stored value but drive bit 6 (SDA) from a seeded bit stream."""
    rng = _random.Random(seed)

    def dev(m, op, addr, size, value):
        if op == "R" and addr == PG_DAT:
            cur = int.from_bytes(m.uc.mem_read(PG_DAT, 4), "little")
            bit = 1 if rng.random() < p_high else 0
            return (cur & ~0x40) | (bit << 6)
        return None
    return dev


def gt_case(args_fn, ret=0, p_high=None, buf=False):
    def case(rng):
        seed = rng.getrandbits(32)
        ph = p_high if p_high is not None else rng.choice([0.0, 0.3, 0.7, 0.98])
        args = args_fn(rng)
        data = bytes(rng.getrandbits(8) for _ in range(64))
        init = compose(regs(rng, PG, 9), regs(rng, PE, 9), lambda m: m.write(0x41000000, data))

        def setup(m):
            init(m)
            m.devices.append((PG_DAT, PG_DAT + 4, sda_model(seed, ph)))
        return Case(args, setup, ret=ret, stubs=delay_stubs())
    return case


TESTS.update({
    0x40049350: gt_case(lambda r: []), 0x40049428: gt_case(lambda r: []),
    0x40049490: gt_case(lambda r: [], p_high=None), 0x4004951C: gt_case(lambda r: []),
    0x40049554: gt_case(lambda r: [r.getrandbits(8) if r.random() < 0.8 else r.getrandbits(32)]),
    0x400495CC: gt_case(lambda r: [], ret=32),
    0x40049688: gt_case(lambda r: [r.getrandbits(16), r.choice([0, 1, 2, 3]), 0x41000000, r.choice([0, 1, 2, 5, 16])]),
    0x40049700: gt_case(lambda r: [r.getrandbits(16), r.choice([0, 1, 2, 3]), 0x41000000, r.choice([0, 1, 2, 5, 16])]),
    0x40049788: gt_case(lambda r: []), 0x400497D4: gt_case(lambda r: []), 0x40049820: gt_case(lambda r: []),
    0x40049834: gt_case(lambda r: []), 0x400498A4: gt_case(lambda r: []),
    0x400498C4: gt_case(lambda r: [r.getrandbits(16), 0x41000000, r.choice([0, 1, 4, 12])]),
    0x400498D4: gt_case(lambda r: [r.getrandbits(16), 0x41000000, r.choice([0, 1, 4, 12])]),
})


# ---------------------------------------------------------------- MS5351M clock generator
def ms_case(args_fn, ret=0):
    def case(rng):
        args = args_fn(rng)
        return Case(args, compose(regs(rng, PE, 9), regs(rng, 0x40544E68 + 0x5F0, 1), regs(rng, 0x42E68C68, 1)),
                    ret=ret, stubs=delay_stubs())
    return case


def freq(rng):
    return rng.choice([0, 1, 2499, 2500, 3499, 3500, 15999, 16000, 31999, 32000, 64000, 128000, 256000, 511999,
                       512000, 1023999, 1024000, 10_000_000, 95_000_000, 150_000_000, 150_000_001, 200_000_000,
                       200_000_001, 0xFFFFFFFF, rng.randrange(2500, 200_000_000), rng.randrange(2500, 2_000_000)])


TESTS.update({
    0x4005F014: ms_case(lambda r: []), 0x4005F060: ms_case(lambda r: []),
    0x4005F0BC: ms_case(lambda r: [r.getrandbits(8) if r.random() < 0.8 else r.getrandbits(32)]),
    0x4005F164: ms_case(lambda r: [r.getrandbits(8), r.getrandbits(8)]),
    0x4005F1BC: ms_case(lambda r: []),
    0x4005F55C: ms_case(lambda r: [r.choice([0x1A, 0x22, 0xFE, 0x1FF]), r.randrange(15, 37), r.getrandbits(20),
                                   r.choice([0xFFFFF, 1, 1000, r.getrandbits(20) | 1, 0])]),
    0x4005F644: ms_case(lambda r: [r.choice([0x2A, 0x32, 0xFC]), r.choice([4, 6, 8, 900, r.getrandbits(16)]),
                                   r.choice([0, 0x0C, 0x10, 0x70])]),
    0x4005F1F0: ms_case(lambda r: [freq(r)]), 0x4005F38C: ms_case(lambda r: [freq(r)]),
})
