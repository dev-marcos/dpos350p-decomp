"""Behavioural tests for the Arm C library pieces (AEABI helpers, conversions, libm)."""
import math
import struct

from tools.emu.tests.fatfs import Case, BUF, BUF2, rand_bytes

N = 400
f2b = lambda x: struct.unpack("<Q", struct.pack("<d", x))[0]

SPECIAL_D = [0.0, -0.0, 0.5, 1.0, -1.0, 1.5, 2.0, 1e-310, -1e-310, 5e-324, 2.2250738585072014e-308,
             1.7976931348623157e308, -1.7976931348623157e308, math.inf, -math.inf, math.nan,
             2.0 ** 31, -2.0 ** 31, 2.0 ** 31 - 0.5, 2.0 ** 63, -2.0 ** 63, 2.0 ** 64, 3e9, -3e9, 1e20, -1e20,
             123456.789, -0.75, 1.5e-323, 0.1, 10.0, 100.0, -100.0, 3.141592653589793, 709.0, 710.0, -745.0]


def rand_double(rng):
    k = rng.random()
    if k < 0.3:
        return f2b(rng.choice(SPECIAL_D)) ^ (rng.getrandbits(1) << 63 if rng.random() < 0.2 else 0)
    if k < 0.6:
        return rng.getrandbits(64)                                   # any bit pattern
    e = rng.choice([0, 1, 0x3FE, 0x3FF, 0x41D, 0x41E, 0x41F, 0x43D, 0x43E, 0x43F, 0x7FE, 0x7FF]) + rng.randrange(-2, 3)
    return (rng.getrandbits(1) << 63) | ((e & 0x7FF) << 52) | rng.getrandbits(52)


def rand_float(rng):
    if rng.random() < 0.5:
        return rng.getrandbits(32)
    e = rng.choice([0, 1, 0x7E, 0x7F, 0x9D, 0x9E, 0x9F, 0xBD, 0xBE, 0xBF, 0xFE, 0xFF])
    return (rng.getrandbits(1) << 31) | ((e & 0xFF) << 23) | rng.getrandbits(23)


def rand_u64(rng):
    return rng.choice([0, 1, 2, 0xFFFFFFFF, 1 << 32, (1 << 63) - 1, 1 << 63, (1 << 64) - 1, 1 << 53, (1 << 53) + 1,
                       rng.getrandbits(64), rng.getrandbits(32), rng.getrandbits(rng.randrange(1, 64))])


def split(v):
    return [v & 0xFFFFFFFF, (v >> 32) & 0xFFFFFFFF]


def t_uidivmod(rng):
    return Case([rng.choice([0, 1, 7, 0xFFFFFFFF, rng.getrandbits(32)]),
                 rng.choice([0, 1, 2, 3, 0xFFFFFFFF, rng.getrandbits(32), rng.getrandbits(8)])], ret=64)


def t_uldivmod(rng):
    return Case(split(rand_u64(rng)) + split(rng.choice([0, 1, 2, 3, 10, rand_u64(rng), rng.getrandbits(16)])), ret=128)


def t_ldiv0(rng):
    return Case([rng.getrandbits(32) for _ in range(4)], ret=128)


def t_mem(align):
    def case(rng):
        n = rng.choice([0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 64, 100, rng.randrange(0, 300)])
        so, do = (0, 0) if align == "aligned" else (rng.randrange(4), rng.randrange(4))
        if align == "dst_unaligned":
            so, do = 0, rng.choice([1, 2, 3])
        src = rand_bytes(rng, 400)
        return Case([BUF2 + do, BUF + so, n], lambda m: (m.write(BUF, src), m.write(BUF2, bytes(400))), ret=0)
    return case


def t_memset4(rng):
    n = rng.choice([0, 1, 2, 3, 4, 5, 7, 8, 12, 15, 16, 17, 20, 31, 32, 33, rng.randrange(0, 300)])
    return Case([BUF, n, rng.getrandbits(32)], lambda m: m.write(BUF, bytes(range(256)) * 2), ret=0)


def t_memclr4(rng):
    n = rng.choice([0, 1, 3, 4, 15, 16, 17, rng.randrange(0, 300)])
    return Case([BUF, n], lambda m: m.write(BUF, b"\xAA" * 400), ret=0)


def t_llabs(rng):
    return Case(split(rand_u64(rng)), ret=64)


def t_d2l(rng):
    return Case(split(rand_double(rng)), ret=64)


def t_f2l(rng):
    return Case([rand_float(rng)], ret=64)


def t_l2d(rng):
    return Case(split(rand_u64(rng)), ret=64)


def dcase(rng, args=(), setup=None, nd=1):
    c = Case(list(args), setup, ret="d")
    c.dregs = {i: rand_double(rng) for i in range(nd)}
    return c


def t_sqrt(rng):
    return dcase(rng)


def t_frexp(rng):
    c = dcase(rng, [BUF], lambda m: m.w32(BUF, 0x55555555))
    return c


def t_ldexp(rng):
    n = rng.choice([0, 1, -1, 2, -2, 52, -52, 53, -53, 54, -54, 0x3FB, 0x3FC, 0x3FD, 1023, 1024, 2046, 2047, 2100,
                    -1022, -1023, -1074, -1075, -2000, 0x7FFFFFFF, -0x80000000, rng.randrange(-3000, 3000)])
    return dcase(rng, [n & 0xFFFFFFFF])


TESTS = {
    0x4003E6F8: t_uidivmod, 0x4003E704: t_ldiv0, 0x40034590: t_uldivmod, 0x4003453C: t_uldivmod,
    0x4005C31C: t_mem("any"), 0x4005C340: t_mem("aligned"), 0x4005C38C: t_mem("dst_unaligned"),
    0x4006A6FC: t_memset4, 0x40068E5C: t_memclr4, 0x4006A4DA: t_llabs,
    0x4003DF94: t_d2l, 0x4005F6E8: t_d2l, 0x4006A4E8: t_f2l, 0x4003A5D8: t_l2d, 0x4005F730: t_l2d,
    0x4003DF74: t_sqrt, 0x40069F54: t_frexp, 0x4006B86C: t_ldexp,
}


def t_exp(rng):
    c = dcase(rng)
    k = rng.random()
    if k < 0.5:                                   # interesting range of exp
        c.dregs = {0: f2b(rng.uniform(-760, 720))}
    elif k < 0.6:
        c.dregs = {0: f2b(rng.choice([709.78, 709.79, -708.4, -745.13, -745.14, 1e-17, -1e-17, 2.0 ** -54,
                                      2.0 ** -53, 0.3465735902799726, -0.3465735902799726]))}
    return c


TESTS[0x40068F08] = t_exp


def t_log(rng):
    c = dcase(rng)
    k = rng.random()
    if k < 0.4:
        c.dregs = {0: f2b(rng.uniform(0, 4))}
    elif k < 0.6:
        c.dregs = {0: f2b(10.0 ** rng.uniform(-320, 308))}
    elif k < 0.7:
        c.dregs = {0: f2b(rng.choice([0.7071067811865476, 0.7071067811865475, 1.0, 1.0000000000000002,
                                      0.9999999999999999, 2.0, 10.0, 100.0, 1e-310, 5e-324]))}
    return c


TESTS[0x4006A31C] = t_log
TESTS[0x4005DAD0] = t_log


def t_tanh(rng):
    c = dcase(rng)
    k = rng.random()
    if k < 0.6:
        c.dregs = {0: f2b(rng.uniform(-22, 22))}
    elif k < 0.75:
        v = rng.choice([2.0 ** -27, 2.0 ** -27 * 0.999, 0.34657359027997264, 0.3465735902799726, 0.6931471805599453,
                        0.6931471805599452, 19.061547465398494, 19.0615474653985, 1e-300])
        c.dregs = {0: f2b(v if rng.random() < 0.5 else -v)}
    return c


TESTS[0x4005F91C] = t_tanh


def trig_arg(rng):
    k = rng.random()
    if k < 0.4:
        return f2b(rng.uniform(-10, 10))
    if k < 0.6:
        return f2b(rng.uniform(-1e6, 1e6))
    if k < 0.75:
        return f2b(rng.choice([1.0, -1.0]) * 10.0 ** rng.uniform(8, 308))
    if k < 0.85:
        v = rng.choice([3.141592653589793, 1.5707963267948966, 2.0 ** -27, 2.0 ** -28, 3.37e9, 3.3732e9, 1e-300,
                        6.283185307179586, 1e22, 2.0 ** 1023])
        return f2b(v if rng.random() < 0.5 else -v)
    return rand_double(rng)


def t_sincos_core(rng):
    c = Case([rng.choice([0, 1, 2, 3, 4, -1, 0x7FFFFFFF])], ret="d")
    c.dregs = {0: trig_arg(rng)}
    return c


def t_trig(rng):
    c = Case([], ret="d")
    c.dregs = {0: trig_arg(rng)}
    return c


TESTS[0x4006613C] = t_sincos_core
TESTS[0x40066530] = t_trig
TESTS[0x40066534] = t_trig
