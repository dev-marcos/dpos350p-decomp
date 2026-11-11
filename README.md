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

Status: Phase 1. The application is still one `.incbin` blob.
