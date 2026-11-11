#!/bin/bash
# stdio MCP entry point for REA, launched from Windows via: wsl -d dpos-re -u dev -- bash <this>
source "$(dirname "$0")/env.sh"
cd "$DPOS_ROOT"
exec rea mcp "$@"
