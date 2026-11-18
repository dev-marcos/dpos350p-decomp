# DPOS350P update container (build 2025-06-19)

This file covers `DPOS350P-FNIRSI-2025619.iso`: 948,310 bytes (`0xE7856`), SHA-256
`6635ddd661fa6aa6a36672c1428e74d0f78913a24fd5a425cebedff0acefd58c`. Despite the extension it
is not ISO 9660. It is implemented by `tools/dposiso.py`, and `make check` proves the layout
below is complete: every byte outside the logo pixels and the application image is
regenerated from constants.

Evidence levels follow the Hack repo convention: HW-CONFIRMED / STATIC-CONFIRMED /
DERIVED / OPEN.

| ISO offset | Size | Content | Level |
|---|---|---|---|
| `0x000000` | `0x1000` | `"DPOS350P"` + zeros | STATIC-CONFIRMED |
| `0x001000` | `0x50000` | eGON.BMP block (boot logo) | STATIC-CONFIRMED |
| `0x051000` | `0x95800` | eGON.EXE block (application) | STATIC-CONFIRMED |
| `0x0E6800` | `0x1032` | zeros | STATIC-CONFIRMED |
| `0x0E7832` | 36 | checksum footer | STATIC-CONFIRMED (reproduced) |

## eGON.BMP block (`0x001000`)

| Block offset | Bytes | Meaning | Level |
|---|---|---|---|
| `+0x00` | `06 00 00 EA` | ARM `b` over the header | STATIC-CONFIRMED |
| `+0x04` | `"eGON.BMP"` | magic | STATIC-CONFIRMED |
| `+0x0C` | `0x6B144BAD` | check word. It is **not** the Allwinner eGON sum (stamp `0x5F0A6C39`) over `0x18A00`, `0x50000` or `0x95800` bytes, and **not** CRC32, a word sum, an XOR or a byte sum over the block, the pixels from `+0x20`/`+0x40`, or the whole `0x50000`. Kept as a constant. | OPEN |
| `+0x10` | `0x00018A00` | length field (= `0x28` + pixels + 290 bytes of slack) | STATIC-CONFIRMED value, OPEN meaning of the slack |
| `+0x14` | `"BMP\0"` | sub-magic | STATIC-CONFIRMED |
| `+0x18` | `00 10 11 02 5F 00 01 1B` | image descriptor: `0x10` = 16 bpp?, u16 width = 529, u16 height = 95, `0x01`, `0x1B` | DERIVED |
| `+0x20` | same 8 bytes again | second copy of the descriptor | STATIC-CONFIRMED value |
| `+0x28` | 100,510 bytes | logo pixels, RGB565 LE, 529 × 95, row-major | DERIVED |
| … | zeros | | STATIC-CONFIRMED |
| `+0x4FFF8` (ISO `0x50FF8`) | `5A 5A 55 AA 00 00 00 00` | trailer marker at the end of the block | OPEN (meaning) |

**Pixel start = `+0x28`.** The earlier candidates were `+0x20` and `+0x40`. Rendering at every
even offset from `+0x20` to `+0x40` shows that the artwork only fits without wrapping around
the row when the pixels start between `+0x22` and `+0x2C`. At `+0x28` the logo has a 2 px left
margin and a 3 px right margin. `+0x28` is also exactly where the second descriptor copy ends,
and at `+0x20` the descriptor bytes themselves show up as 4 stray pixels in row 0. This is
DERIVED; no boot code has been read yet to confirm it.

The 4-byte trailer marker means the range `0x019A00`–`0x050FFF` is **not** all zeros, which
corrects the starting brief.

## eGON.EXE block (`0x051000`)

| Block offset | Value | Meaning | Level |
|---|---|---|---|
| `+0x00` | `0xEA000006` | branch over the header | STATIC-CONFIRMED |
| `+0x04` | `"eGON.EXE"` | magic | STATIC-CONFIRMED |
| `+0x0C` | `3` | at the same position as the BMP check word, but the value is 3 (a version or type?) | OPEN |
| `+0x10` | `0x00095800` | block length, header included | STATIC-CONFIRMED |
| `+0x14` | `"EXEC"` | sub-magic | STATIC-CONFIRMED |
| `+0x18` | 8 zero bytes | | STATIC-CONFIRMED |
| `+0x20` | 612,320 bytes | application image, loaded at `0x40000000` | STATIC-CONFIRMED (upstream) |

The last non-zero byte of the application is at ISO `0xE67D6`. The image is kept at full
block length, trailing zeros included, so the linker script asserts exactly `0x957E0` bytes.

## Footer (`0x0E7832`)

The footer is 9 × uint32 big-endian over `iso[0x1000 : len-36]`, with indices relative to
`0x1000`:

- `cd[0]` is the sum of all bytes.
- `cd[1..8]` is, for bit 7 down to bit 0, the sum of the indices of the bytes that have that bit set.

Everything is taken mod 2³². This is reproduced exactly by `dposiso.py`; the algorithm is
documented upstream in `third_party/FNIRSI-DPOS350P-Hack/docs/checksum.md`.

The firmware checks the same footer and the `DPOS350P` header when it finds a `*.iso` in the SD
root at start-up (`sd_update_check`, 0x4004F134, see `DRIVERS.md` batch 45). An invalid file
is deleted; a valid one starts an update.

## Build pipeline (Phase 1)

```
make extract   ISO -> build/extract/{app.bin, logo.rgb565}     (dposiso.py unpack)
asm/app.s      .incbin "build/extract/app.bin"  -> as -> ld (ld/dpos350p.ld @0x40000000)
               -> objcopy -O binary -> build/app.bin
make           dposiso.py pack (headers, padding, trailer, footer) -> build/DPOS350P.iso
make check     footer + SHA-256 against the golden hash
```

`dposiso.py unpack` refuses to extract an ISO whose non-content bytes do not match the
known layout, so a different firmware build cannot be fed in silently.
