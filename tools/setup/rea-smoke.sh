#!/bin/bash
# Phase 0 smoke test: REA + Ghidra open app.bin as headerless ARM at 0x40000000.
source "$(dirname "$0")/../env.sh"
cd "$DPOS_ROOT"
time rea inspect build/app.bin --target-format raw-binary --architecture arm \
  --entry-point 0x40000000 --provider ghidra --json > evidence/phase0-rea-inspect.json
echo "exit=$?"; head -c 3000 evidence/phase0-rea-inspect.json; echo
