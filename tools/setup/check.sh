#!/bin/bash
# Phase 0 acceptance check: every tool resolves from inside tools/.
source "$(dirname "$0")/../env.sh"
set -e
echo "java:   $(command -v java)"; java -version 2>&1 | head -1; javac -version
echo "ghidra: $GHIDRA_INSTALL_DIR ($(grep application.version "$GHIDRA_INSTALL_DIR/Ghidra/application.properties"))"
ls "$GHIDRA_INSTALL_DIR/Ghidra/Features/Decompiler/os/linux_x86_64/decompile" >/dev/null && echo "ghidra decompiler (linux_x86_64): present"
echo "node:   $(command -v node) $(node --version)"
echo "gcc:    $(command -v arm-none-eabi-gcc)"; arm-none-eabi-gcc --version | head -1
echo "python: $(command -v python) $(python --version)"
python -c "import capstone,unicorn,PIL,numpy;print('capstone',capstone.__version__,'unicorn',unicorn.__version__,'pillow',PIL.__version__,'numpy',numpy.__version__)"
echo "rea:    $(command -v rea || command -v rea-agents || echo MISSING)"
sha256sum "$DPOS_ROOT/firmware/DPOS350P-FNIRSI-2025619.iso"
