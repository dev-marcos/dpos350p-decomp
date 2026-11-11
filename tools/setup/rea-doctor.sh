#!/bin/bash
source "$(dirname "$0")/../env.sh"
rea --version
rea doctor --provider ghidra --json
