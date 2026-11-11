#!/bin/bash
# Run as root inside dpos-re once: creates user, wsl.conf, apt packages.
set -euo pipefail
id dev >/dev/null 2>&1 || useradd -m -s /bin/bash dev
cat > /etc/wsl.conf <<'CONF'
[user]
default=dev
[automount]
options="metadata,umask=022"
[interop]
appendWindowsPath=false
CONF
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y -qq build-essential git make unzip xz-utils python3-venv python3-dev \
    curl ca-certificates file bison flex >/dev/null
apt-get clean
echo APT_OK
df -h / | tail -1
