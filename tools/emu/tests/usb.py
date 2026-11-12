"""Behavioural tests for the USB device stack (EasyUSB-derived, see docs/USB.md).

Every case starts from randomised controller registers (USBC0 0x01C19000..0x01C19500), CCU
clock/reset registers and the SRAM controller word, and compares return value, memory, the
ordered MMIO log and the stub calls (debug_printf_stub arguments are stock string addresses).
"""
from tools.emu.emu import Stub
from tools.emu.tests.fatfs import Case

N = 200
USBC = 0x01C19000
BUF = 0x41000000
PRINTF = Stub("debug_printf_stub", 0x4006711C, lambda m, a: None, 1)


def regs(rng, base, n_words):
    vals = [rng.getrandbits(32) for _ in range(n_words)]
    return lambda m: [m.w32(base + 4 * i, v) for i, v in enumerate(vals)]


def state(rng):
    parts = [regs(rng, USBC, 0x140), regs(rng, 0x01C20000, 0xC0), regs(rng, 0x01C00000, 4)]
    data = bytes(rng.getrandbits(8) for _ in range(256))
    return lambda m: ([p(m) for p in parts], m.write(BUF, data))


def ep_type(r):
    return r.choice([0, 1, 2, 3, 4, r.getrandbits(32)])


def small(r):
    return r.choice([0, 1, 2, 3, 4, 5, 7, 15, r.randrange(32)])


def u32(r):
    return r.choice([0, 1, 0xFF, 0xFFFF, r.getrandbits(8), r.getrandbits(16), r.getrandbits(32)])


def case(args_fn, ret=0):
    def mk(r):
        return Case(args_fn(r), state(r), ret=ret, stubs=(PRINTF,))
    return mk


V, R = 0, 32
NOARG = lambda r: []
TESTS = {}


def add(addrs, args_fn, ret):
    for a in addrs:
        TESTS[a] = case(args_fn, ret)


# no-argument register helpers
add([0x4006C328, 0x4006C368, 0x4006C424, 0x4006C454, 0x4006C488, 0x4006C4CC, 0x4006C4DC, 0x4006C570,
     0x4006C604, 0x4006C618, 0x4006C62C, 0x4006C640, 0x4006C654, 0x4006C668, 0x4006C67C, 0x4006C690, 0x4006C6A4,
     0x4006C76C, 0x4006C798, 0x4006C7AC, 0x4006C7D4, 0x4006C7F8, 0x4006C810, 0x4006C834, 0x4006C978,
     0x4006CA84, 0x4006CA94, 0x4006CAA4, 0x4006CAC0, 0x4006CB74, 0x4006CB84, 0x4006CB94, 0x4006CBAC,
     0x4006CD98, 0x4006CDAC, 0x4006CDC4, 0x4006CE24, 0x4006CE38, 0x4006CE50, 0x4006CEB0], NOARG, V)
add([0x4006C384, 0x4006C3D4, 0x4006C434, 0x4006C468, 0x4006C558, 0x4006C5C0, 0x4006C730, 0x4006C788,
     0x4006C7E8, 0x4006C824, 0x4006C964, 0x4006C974, 0x4006C98C, 0x4006CC54, 0x4006CC64, 0x4006CC74,
     0x4006CC84], NOARG, R)
# one value / index argument
add([0x4006C440, 0x4006C474, 0x4006C49C, 0x4006C4B4, 0x4006C4D0, 0x4006C4E0, 0x4006C564, 0x4006C5AC,
     0x4006C5CC, 0x4006C770, 0x4006C77C], lambda r: [r.choice([small(r), u32(r)])], V)
add([0x4006C998], lambda r: [small(r)], R)
# endpoint-type dispatchers
add([0x4006C538, 0x4006C748, 0x4006CF24], lambda r: [ep_type(r)], V)
add([0x4006C4EC, 0x4006C85C, 0x4006C8B4, 0x4006C90C, 0x4006CADC, 0x4006CB28, 0x4006CBC4, 0x4006CC00,
     0x4006CC94, 0x4006CCB4, 0x4006CCE0], lambda r: [ep_type(r)], R)
add([0x4006C510, 0x4006C580], lambda r: [ep_type(r), small(r)], V)
add([0x4006CC3C, 0x4006CC48], lambda r: [ep_type(r), r.choice([0, 1, 2, r.getrandbits(32)])], R)
add([0x4006C5D8], lambda r: [r.choice([0, 1, 2, r.getrandbits(32)])], V)
add([0x4006C6A8], lambda r: [r.choice([0, 1, 2, 3, 4, 5, r.getrandbits(32)]), r.choice([0, 1, 2, 3, 4, r.getrandbits(32)])], V)
add([0x4006C264], lambda r: [r.choice([0, 1, 2]), ep_type(r), small(r)], V)
add([0x4006C2B8], lambda r: [r.getrandbits(8), u32(r) | r.choice([0, 0x80000000]), r.choice([-1, 0, 1, 2, 5, 8, 13])], V)
fifo = lambda r: [r.choice([0, 1, r.getrandbits(32)]), r.choice([0, 1, 64, 511, 512, 513, 1024, 4096, r.getrandbits(16), r.getrandbits(32)]),
                  r.choice([0, 8, 0x200, r.getrandbits(16), r.getrandbits(32)])]
add([0x4006C9A8, 0x4006C9FC], fifo, V)
add([0x4006CA50], lambda r: [ep_type(r)] + fifo(r), V)
cfg = lambda r: [r.choice([0, 1, 2, 3, 4, r.getrandbits(32)]), r.choice([0, 1, r.getrandbits(32)]), u32(r)]
add([0x4006CDDC, 0x4006CE68], cfg, V)
add([0x4006CEC0], lambda r: (lambda c: [c[0], ep_type(r), c[1], c[2]])(cfg(r)), R)


def packet(r):
    fifo_ = r.choice([USBC + 4 * r.randrange(6), BUF + 0x800])
    return [fifo_, r.choice([0, 1, 2, 3, 4, 5, 8, 63, 64, 65, r.randrange(200)]), BUF + r.choice([0, 0, 4, 1, 2])]


add([0x4006CD10, 0x4006CD54], packet, R)


# ---------------------------------------------------------------- ISCR helpers (0x01C19400)
add([0x4006C108, 0x4006C114, 0x4006C134, 0x4006C154, 0x4006C178, 0x4006C198, 0x4006C1DC, 0x4006C1FC,
     0x4006C220], NOARG, V)
add([0x4006C10C], lambda r: [u32(r)], R)
add([0x4006C1B8, 0x4006C240], lambda r: [ep_type(r)], V)

# ---------------------------------------------------------------- usb_dev.c core
import random as _random

G = 0x42E68000
EP_MAX = 0x40095470


def dev_model(seed, p1):
    """CSR0/TXCSR, RXCSR: bits re-drawn on every read (seeded); COUNT0/RXCOUNT: small lengths."""
    rng = _random.Random(seed)

    def dev(m, op, addr, size, value):
        if op != "R":
            return None
        if addr in (USBC + 0x82, USBC + 0x86):
            return rng.getrandbits(16) & ~1 | (1 if rng.random() < p1 else 0)
        if addr == USBC + 0x88:
            return rng.choice([8, 8, 8, 0, 1, 7, 9, 64, rng.randrange(80)])
        return None
    return dev


def cb(name, addr, nargs=2):
    return Stub(name, addr, lambda m, a: None, nargs)


def usb_setup_fn(m, a):
    from unicorn import arm_const as A
    m.calls.append(("usb_setup_handle*", bytes(m.read(m.reg(A.UC_ARM_REG_R0), 8)), m.reg(A.UC_ARM_REG_R1)))


CORE_STUBS = (PRINTF, Stub("dbg_printf", 0x4006709C, lambda m, a: None, 1),
              Stub("irq_register", 0x40066BD4, lambda m, a: 0, 4), Stub("irq_enable", 0x40066B88, lambda m, a: None, 0),
              Stub("usb_setup_handle", 0x4006D5E8, usb_setup_fn, 0),
              cb("usb_mass_in_ep_callback", 0x4006B9D8, 0), cb("usb_mass_out_ep_callback", 0x4006B9F0),
              cb("usb_hid_out_ep_callback", 0x4006D7CC), cb("usb_cdc_out_ep_callback", 0x4006D884))


def core_state(r, ps=(0.0, 0.3, 0.7, 1.0)):
    base = state(r)
    seed, p1 = r.getrandbits(32), r.choice(ps)
    gl = bytearray(r.getrandbits(8) for _ in range(0xD30))
    words = {0xCF0: r.choice([0, 0x40, 0x400, r.getrandbits(12)]), 0xCF4: r.choice([0, 1, r.getrandbits(32)]),
             0xCF8: r.choice([0, 1, 2, 3, 4, r.getrandbits(32)]), 0xCFC: r.choice([0, 8, 64, 65, 100, 200, 512]),
             0xD00: r.choice([0, 64, 128]), 0xD04: r.randrange(600), 0xD08: BUF}
    # usb_tx_pos <= usb_tx_buf_len: a negative remainder makes both versions copy ~4 GiB
    words[0xCFC] = max(words[0xCFC], words[0xD00])
    for o, v in words.items():
        gl[o:o + 4] = (v & 0xFFFFFFFF).to_bytes(4, "little")
    gl[0xD1F] = r.choice([0, 1, 2, 3, 4, 5])
    gl[0xD20] = r.choice([0, 2, 3])
    ep = b"".join(r.choice([8, 64, 512]).to_bytes(4, "little") for _ in range(3))

    def setup(m):
        base(m)
        m.write(G, gl)
        m.write(EP_MAX, ep)
        m.devices.append((USBC + 0x80, USBC + 0x8C, dev_model(seed, p1)))
    return setup


def core(args_fn, ret=0, ps=(0.0, 0.3, 0.7, 1.0)):
    def mk(r):
        return Case(args_fn(r), core_state(r, ps), ret=ret, stubs=CORE_STUBS)
    return mk


for a in (0x4006AB18, 0x4006AC40, 0x4006ACB8, 0x4006AD0C, 0x4006AD2C, 0x4006AD84, 0x4006ADBC, 0x4006B290,
          0x4006B624, 0x4006B728):
    TESTS[a] = core(NOARG)
for a in (0x4006AC68,):
    TESTS[a] = core(NOARG, R)
TESTS[0x4006AB24] = core(lambda r: [r.choice([0, 1, 2])])
TESTS[0x4006AC44] = core(lambda r: [r.getrandbits(8)])
TESTS[0x4006AC5C] = core(lambda r: [u32(r)])
TESTS[0x4006ACDC] = core(lambda r: [u32(r)], R)
TESTS[0x4006AD4C] = core(lambda r: [r.choice([0, 1, 2, 3, 4, r.getrandbits(32)])])
TESTS[0x4006AA28] = core(lambda r: [r.choice([0, 1, 2]), BUF, r.choice([0, 1, 8, 63, 64, 65, 100, 200])], R)
TESTS[0x4006AAEC] = core(lambda r: [r.choice([0, 1, 2]), BUF + 0x400, r.choice([0, 1, 8, 64, 65, 100])], R)
# FIFO-busy probability < 1: the 10 000 000-poll timeout path is too long to emulate (not exercised)
TESTS[0x4006AB30] = core(lambda r: [r.choice([1, 2]), BUF, r.choice([0, 1, 8, 64, 65, 200, 600])], R, ps=(0.0, 0.3, 0.7, 0.9))
TESTS[0x4006B0E0] = core(lambda r: [r.choice([1, 2, 3])])
TESTS[0x4006B17C] = core(lambda r: [r.choice([1, 2, 3])])
TESTS[0x4006B4B8] = core(lambda r: [r.choice([0, 1, 2]), r.choice([8, 64, 512]), r.choice([0, 1, 2, 3, 4])])
TESTS[0x4006B550] = core(lambda r: [r.choice([0, 1, 2]), r.choice([8, 64, 512]), r.choice([0, 1, 2, 3, 4])])
TESTS[0x4006B5E8] = core(lambda r: [r.choice([0x81, 0x01, 0x82, 0x02, r.getrandbits(8)])])
TESTS[0x4006B6A4] = core(lambda r: [r.choice([0, 1, 2, 4])], R)


# ---------------------------------------------------------------- class layer (requests, CDC/HID, mass storage)
def rec(name, nargs):
    return Stub(name, {"delay_ms": 0x40042CF0, "delay_us": 0x40042D0C, "usb_mass_disk_read": 0x4006A964,
                       "usb_mass_disk_write": 0x4006A978}[name], lambda m, a: None, nargs)


CLASS_STUBS = CORE_STUBS[:4] + (rec("delay_ms", 1), rec("delay_us", 1), rec("usb_mass_disk_read", 3),
                                rec("usb_mass_disk_write", 3))
CLASS_STUBS = tuple(s for s in CLASS_STUBS if s.name != "usb_setup_handle")


def req_bytes(r):
    rt = r.choice([0x00, 0x80, 0x21, 0xA1, 0x40, 0x01, 0x02, r.getrandbits(8)])
    rq = r.choice([0, 1, 5, 6, 7, 8, 9, 0x0A, 0x0B, 0x0C, 0x20, 0x21, 0x22, 0xFE, r.getrandbits(8)])
    wv = r.choice([0x0100, 0x0200, 0x0300, 0x0301, 0x0302, 0x0303, 0x0304, 0x0305, 0x0306, 0x03EE, 0x0307, 0x2200,
                   0x0600, r.getrandbits(16)])
    wi = r.choice([0, 0x81, 0x01, 0x82, r.getrandbits(16)])
    wl = r.choice([0, 1, 4, 5, 7, 8, 9, 0x12, 0x20, 0x26, 0x29, 0x40, 0xFF, r.getrandbits(16)])
    return bytes([rt, rq]) + wv.to_bytes(2, "little") + wi.to_bytes(2, "little") + wl.to_bytes(2, "little")


def cbw_bytes(r):
    op = r.choice([0x00, 0x03, 0x04, 0x12, 0x1A, 0x1B, 0x1E, 0x23, 0x25, 0x28, 0x2A, 0x2F, 0x5A, 0x08, 0x55, 0xAF,
                   0x01, 0xB0, r.getrandbits(8)])
    sig = 0x43425355 if r.random() < 0.85 else r.getrandbits(32)
    lun = r.choice([0, 0, 1, 2])
    cnt = r.choice([0, 1, 2, 8])
    blk = 512
    dlen = r.choice([cnt * blk, 0, 36, r.getrandbits(16)])
    cb = bytearray(16)
    cb[0] = op
    cb[1] = r.choice([0, 1, 4, r.getrandbits(8)])
    lba = r.choice([0, 1, 100, 0xFFFF, r.getrandbits(32)])
    cb[2:6] = lba.to_bytes(4, "big")
    cb[4] = cb[4] if op in (0x28, 0x2A) else r.choice([0, 5, 18, 0x13, 0x24, 0x25, 0xFF])
    cb[7:9] = cnt.to_bytes(2, "big")
    return (sig.to_bytes(4, "little") + r.getrandbits(32).to_bytes(4, "little") + dlen.to_bytes(4, "little") +
            bytes([r.choice([0x80, 0x00]), lun, r.choice([6, 10, 12, 0, 17, 16])]) + bytes(cb))


def class_state(r, types=(0, 1, 2, 3, 4, 5), stage=None):
    base = core_state(r, (0.0, 0.3, 0.7))
    req = req_bytes(r)
    c = cbw_bytes(r)
    cur = r.choice(types)
    st = r.choice([0, 1, 2, 3, 4, 5, 6]) if stage is None else r.choice(stage)
    c2 = cbw_bytes(r)
    ml = r.choice([0, 1, 2]).to_bytes(4, "little")
    luns = b"".join(r.choice([512, 4096, 0x1000000, 0x10000, 0]).to_bytes(4, "little") for _ in range(3))

    def setup(m):
        base(m)
        m.write(BUF, req)
        m.write(BUF + 0x100, c)
        m.write(G + 0xB90, c2)
        m.write(G + 0xD0C, luns)
        m.write(G + 0xD1F, bytes([cur]))
        m.write(G + 0xD1D, bytes([st]))
        m.write(G + 0xD23, ml)
    return setup


def cls(args_fn, ret=0, **kw):
    def mk(r):
        return Case(args_fn(r), class_state(r, **kw), ret=ret, stubs=CLASS_STUBS)
    return mk


REQ = lambda r: [BUF]
for a in (0x4006D04C, 0x4006D078, 0x4006D1B0, 0x4006D378, 0x4006D4B0, 0x4006D774, 0x4006D7DC, 0x4006B980):
    TESTS[a] = cls(REQ)
TESTS[0x4006CFE4] = cls(REQ, types=(1, 2, 3, 4))     # other types use an unset register (OPEN)
TESTS[0x4006DDC0] = cls(REQ, R)
TESTS[0x4006D5E8] = cls(lambda r: [BUF, r.choice([8, 8, 8, 7, 0])])
TESTS[0x4006CFB0] = cls(lambda r: [BUF])
TESTS[0x4006D2A0] = cls(lambda r: [r.getrandbits(8)])
for a in (0x4006D2D8, 0x4006D764, 0x4006B9F4, 0x4006DA3C, 0x4006D9FC, 0x4006DA08):
    TESTS[a] = cls(NOARG)
TESTS[0x4006D2DC] = cls(lambda r: [G + 0xD1F])
TESTS[0x4006B9D8] = cls(NOARG, R)
TESTS[0x4006B978] = cls(NOARG, R)
TESTS[0x4006D7CC] = cls(lambda r: [BUF, r.choice([0, 1, 8, 64, 100])], R)
for a in (0x4006D830, 0x4006D884):
    TESTS[a] = cls(lambda r: [BUF, r.choice([0, 1, 8, 64, 100, 512, 1024])], R)
TESTS[0x4006B95C] = cls(lambda r: [r.getrandbits(32), BUF, r.choice([0, 13, 64])], R)
for a in (0x4006B9F0, 0x4006BA98):
    TESTS[a] = cls(lambda r: [BUF + 0x100, r.choice([31, 31, 31, 0, 30, 32, 13])])
TESTS[0x4006C04C] = cls(lambda r: [BUF, r.choice([0, 4, 8, 36, 64])])
TESTS[0x4006C08C] = cls(lambda r: [r.choice([0, 1, 2, r.getrandbits(8)]), r.choice([0, 1, 2])])
TESTS[0x4006D888] = cls(lambda r: [r.choice([0, 1, 2, r.getrandbits(32)])], R)
LUN = lambda r: [r.choice([0, 0, 1, 2])]
for a in (0x4006D898, 0x4006D8CC, 0x4006D95C, 0x4006DA14, 0x4006DBA4, 0x4006DBF0, 0x4006DC44, 0x4006DC90):
    TESTS[a] = cls(LUN)
TESTS[0x4006DA2C] = cls(lambda r: [r.choice([0, 1]), r.getrandbits(8), r.getrandbits(8)])
RW = lambda r: [r.choice([0, 1]), r.choice([0, 1, 100, r.getrandbits(32)]), r.choice([0, 1, 2, 8])]
for a in (0x4006DA48, 0x4006DB0C):
    TESTS[a] = cls(RW, stage=(0, 0, 1, 2, 3))
TESTS[0x4006DCE4] = cls(lambda r: RW(r)[:1] + [r.choice([0x28, 0x2A])] + RW(r)[1:], R)
