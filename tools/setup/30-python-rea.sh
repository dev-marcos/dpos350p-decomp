#!/bin/bash
# Run as 'dev' inside dpos-re after 10-tools.sh: Python venv + REA (npm --prefix).
set -euo pipefail
source "$(dirname "$0")/../env.sh"
[ -x "$VIRTUAL_ENV/bin/python" ] || /usr/bin/python3 -m venv "$VIRTUAL_ENV"
"$VIRTUAL_ENV/bin/pip" install -q --upgrade pip
"$VIRTUAL_ENV/bin/pip" install -q -r "$DPOS_TOOLS/requirements.txt"
npm install -g --prefix "$DPOS_TOOLS/rea" rea-agents@latest
echo PY_REA_OK
