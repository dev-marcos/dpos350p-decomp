#!/usr/bin/env python3
"""FNIRSI DPOS350P firmware container tool (build 2025-06-19).

The ".iso" update file is not ISO 9660. Layout (see docs/ISO_FORMAT.md):

  0x000000  file header: model string "DPOS350P", zero-padded to 0x1000
  0x001000  eGON.BMP block (0x50000 bytes): 0x28-byte header, RGB565 529x95 logo,
            zero padding, 8-byte trailer marker at block end - 8
  0x051000  eGON.EXE block (0x95800 bytes): 0x20-byte header + application image
            (loaded and run at 0x40000000)
  0x0E6800  zero padding up to the footer
  0x0E7832  footer: 9 x uint32 big-endian checksum over iso[0x1000:-36]

Only the logo pixels and the application image are FNIRSI content; everything else
is regenerated from the constants below, so the repository never stores firmware bytes.

Subcommands:
  unpack ISO OUTDIR            extract logo pixels + app image, check the rest is reproducible
  pack --logo F --app F -o ISO assemble a container and compute the footer
  checksum ISO                 print the computed footer
  verify ISO [--sha256 HEX]    check footer (and optionally the whole-file hash)
"""
import argparse
import hashlib
import struct
import sys
from pathlib import Path

# ---------------------------------------------------------------- file header
FILE_MAGIC = b"DPOS350P"
FILE_HEADER_SIZE = 0x1000

# ---------------------------------------------------------------- eGON.BMP block
BMP_OFFSET = 0x1000
BMP_BLOCK_SIZE = 0x50000          # up to the EXE block            [STATIC-CONFIRMED]
BMP_HDR_LEN_FIELD = 0x18A00       # value of the length word at +0x10
# Word at +0x0C. Not the standard Allwinner eGON checksum, not CRC32/sum/xor of
# the obvious ranges. Kept as an opaque constant.                    [OPEN]
BMP_HDR_CHECK = 0x6B144BAD
# 8-byte image descriptor, present twice (+0x18 and +0x20). Interpretation:
# u8 0x00, u8 0x10 (16 bpp?), u16 width=529, u16 height=95, u8 0x01, u8 0x1B. [DERIVED]
BMP_DESC = bytes.fromhex("0010" "1102" "5f00" "01" "1b")
BMP_PIXELS_OFFSET = 0x28          # derived from logo margins (2 px left, 3 px right) [DERIVED]
LOGO_W, LOGO_H = 529, 95
LOGO_SIZE = LOGO_W * LOGO_H * 2   # RGB565 little-endian
# 8 bytes at the very end of the BMP block (ISO 0x50FF8). Meaning unknown.     [OPEN]
BMP_TRAILER = bytes.fromhex("5a5a55aa00000000")

# ---------------------------------------------------------------- eGON.EXE block
EXE_OFFSET = 0x51000
EXE_BLOCK_SIZE = 0x95800          # value of the length word at +0x10
EXE_HDR_WORD_0C = 3               # word at +0x0C (BMP has a check value there)  [OPEN]
EXE_HDR_SIZE = 0x20
APP_SIZE = EXE_BLOCK_SIZE - EXE_HDR_SIZE   # 612320 bytes, loaded at 0x40000000

BRANCH_OVER_HEADER = 0xEA000006   # ARM "b +0x20" (skips the eGON header)

# ---------------------------------------------------------------- footer
FOOTER_SIZE = 36
ISO_SIZE = 0xE7856                # 948310
FOOTER_OFFSET = ISO_SIZE - FOOTER_SIZE
CHECKSUM_START = 0x1000


def bmp_header() -> bytes:
    h = struct.pack("<I8sII", BRANCH_OVER_HEADER, b"eGON.BMP", BMP_HDR_CHECK, BMP_HDR_LEN_FIELD)
    h += b"BMP\0" + BMP_DESC + BMP_DESC
    assert len(h) == BMP_PIXELS_OFFSET
    return h


def exe_header() -> bytes:
    h = struct.pack("<I8sII4s", BRANCH_OVER_HEADER, b"eGON.EXE", EXE_HDR_WORD_0C,
                    EXE_BLOCK_SIZE, b"EXEC")
    return h.ljust(EXE_HDR_SIZE, b"\0")


def compute_footer(iso_wo_footer: bytes) -> list:
    """9-value FNIRSI checksum over iso[0x1000:], indices relative to 0x1000.

    cd[0] = sum of bytes; cd[k] (k=1..8) = sum of indices whose byte has bit (8-k) set.
    Vectorised per byte value: for each value v, count and index-sum of its occurrences.
    """
    payload = memoryview(iso_wo_footer)[CHECKSUM_START:]
    count = [0] * 256
    isum = [0] * 256
    try:
        import numpy as np
        a = np.frombuffer(payload, dtype=np.uint8)
        count = np.bincount(a, minlength=256).tolist()
        # Index sums stay below ~5e11 < 2^53, so float64 accumulation is exact.
        w = np.arange(len(a), dtype=np.float64)
        isum = [int(x) for x in np.bincount(a, weights=w, minlength=256)]
    except ImportError:
        for i, v in enumerate(payload):
            count[v] += 1
            isum[v] += i
    cd = [sum(v * count[v] for v in range(256))]
    for bit in range(7, -1, -1):
        cd.append(sum(isum[v] for v in range(256) if v >> bit & 1))
    return [x & 0xFFFFFFFF for x in cd]


def footer_bytes(iso_wo_footer: bytes) -> bytes:
    return struct.pack(">9I", *compute_footer(iso_wo_footer))


def pack(logo: bytes, app: bytes) -> bytes:
    if len(logo) != LOGO_SIZE:
        sys.exit(f"logo: expected {LOGO_SIZE} bytes, got {len(logo)}")
    if len(app) != APP_SIZE:
        sys.exit(f"app: expected {APP_SIZE} bytes, got {len(app)}")
    iso = bytearray(FOOTER_OFFSET)
    iso[0:len(FILE_MAGIC)] = FILE_MAGIC
    bmp = bytearray(BMP_BLOCK_SIZE)
    hdr = bmp_header()
    bmp[0:len(hdr)] = hdr
    bmp[BMP_PIXELS_OFFSET:BMP_PIXELS_OFFSET + LOGO_SIZE] = logo
    bmp[-len(BMP_TRAILER):] = BMP_TRAILER
    iso[BMP_OFFSET:BMP_OFFSET + BMP_BLOCK_SIZE] = bmp
    exe = exe_header() + app
    iso[EXE_OFFSET:EXE_OFFSET + EXE_BLOCK_SIZE] = exe
    return bytes(iso) + footer_bytes(iso)


def split(iso: bytes):
    if len(iso) != ISO_SIZE:
        sys.exit(f"unexpected ISO size {len(iso)} (expected {ISO_SIZE})")
    p = BMP_OFFSET + BMP_PIXELS_OFFSET
    logo = iso[p:p + LOGO_SIZE]
    app = iso[EXE_OFFSET + EXE_HDR_SIZE:EXE_OFFSET + EXE_BLOCK_SIZE]
    return logo, app


def cmd_unpack(a):
    iso = Path(a.iso).read_bytes()
    logo, app = split(iso)
    rebuilt = pack(logo, app)
    if rebuilt != iso:
        bad = next(i for i in range(len(iso)) if iso[i] != rebuilt[i])
        sys.exit(f"container has bytes outside the known layout (first diff at 0x{bad:06X});"
                 " update tools/dposiso.py")
    out = Path(a.outdir)
    out.mkdir(parents=True, exist_ok=True)
    (out / "logo.rgb565").write_bytes(logo)
    (out / "app.bin").write_bytes(app)
    print(f"unpacked {a.iso} -> {out}/logo.rgb565 ({len(logo)} B), {out}/app.bin ({len(app)} B)")


def cmd_pack(a):
    iso = pack(Path(a.logo).read_bytes(), Path(a.app).read_bytes())
    Path(a.output).parent.mkdir(parents=True, exist_ok=True)
    Path(a.output).write_bytes(iso)
    print(f"packed {a.output} ({len(iso)} B)")


def cmd_checksum(a):
    iso = Path(a.iso).read_bytes()
    for i, v in enumerate(compute_footer(iso[:-FOOTER_SIZE])):
        print(f"cd[{i}] = 0x{v:08X}")


def cmd_verify(a):
    iso = Path(a.iso).read_bytes()
    ok = iso[-FOOTER_SIZE:] == footer_bytes(iso[:-FOOTER_SIZE])
    print(f"footer: {'OK' if ok else 'MISMATCH'}")
    if a.sha256:
        h = hashlib.sha256(iso).hexdigest()
        match = h == a.sha256.lower()
        print(f"sha256: {h} {'OK' if match else 'MISMATCH (expected ' + a.sha256 + ')'}")
        ok = ok and match
    sys.exit(0 if ok else 1)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("unpack"); s.add_argument("iso"); s.add_argument("outdir"); s.set_defaults(f=cmd_unpack)
    s = sub.add_parser("pack"); s.add_argument("--logo", required=True); s.add_argument("--app", required=True)
    s.add_argument("-o", "--output", required=True); s.set_defaults(f=cmd_pack)
    s = sub.add_parser("checksum"); s.add_argument("iso"); s.set_defaults(f=cmd_checksum)
    s = sub.add_parser("verify"); s.add_argument("iso"); s.add_argument("--sha256"); s.set_defaults(f=cmd_verify)
    a = ap.parse_args()
    a.f(a)


if __name__ == "__main__":
    main()
