# FNIRSI DPOS350P firmware decompilation (build 2025-06-19)

A matching decompilation of the FNIRSI DPOS350P oscilloscope firmware: `make` rebuilds
`build/DPOS350P.iso` byte-for-byte identical to the original update file
(SHA-256 `6635ddd661fa6aa6a36672c1428e74d0f78913a24fd5a425cebedff0acefd58c`).

No FNIRSI binaries are stored here. Put your own copy of the original update at
`firmware/DPOS350P-FNIRSI-2025619.iso`; the build extracts what it needs from it.

```bash
wsl -d dpos-re -- bash tools/run.sh make check
```

- `docs/SETUP.md`: toolchain (WSL distro, Ghidra, REA, arm-none-eabi-gcc), all inside `tools/`
- `docs/ISO_FORMAT.md`: the update container layout
- `tools/dposiso.py`: unpack / pack / checksum / verify

- `docs/MEMORY_MAP.md`: image layout, startup/RAM layout, 100% byte classification
- `symbols/symbols_2025-06-19.csv`: every function/pool/data/padding region (`make map`)

- `docs/SPLIT.md`: how `asm/` and `data/` are produced and verified
- `docs/COMPILER.md`: original toolchain identification (FatFs R0.13b; GCC ruled out; armcc pending)

- `docs/CONTRIBUTING.md`: how to decompile a function (C + emulation test)
- `docs/LIBRARY.md`: Arm C library routines found in the image (AEABI, libm) and their status
- `docs/DRIVERS.md`: low-level drivers (PIO, FPGA bus, GT911, MS5351M, watchdog) and the recovered pin map
- `docs/PROGRESS.md`: progress per level and area (`make progress`)
- `docs/STATUS.md`: current status, what is left, suggested next batches and the resume checklist

Status: Phase 5 (equivalent C verified by emulation; Arm Compiler 5 unlicensed, so no
byte matching yet). Earlier phases: The image is rebuilt from per-function assembly (`asm/`) plus four data
blobs extracted from your ISO at build time; no whole-image `.incbin` remains.
