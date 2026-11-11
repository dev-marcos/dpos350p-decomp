#!/bin/bash
# Run a command inside the project environment: wsl -d dpos-re -- bash tools/run.sh <cmd...>
source "$(dirname "$0")/env.sh"
cd "$DPOS_ROOT"
exec "$@"
