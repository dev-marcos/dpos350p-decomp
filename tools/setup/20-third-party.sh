#!/bin/bash
# Clone read-only reference repositories into third_party/ at the pinned commits.
set -euo pipefail
cd "$(dirname "$0")/../../third_party" 2>/dev/null || { mkdir -p "$(dirname "$0")/../../third_party"; cd "$(dirname "$0")/../../third_party"; }
clone() { [ -d "$2" ] || git clone -q https://github.com/$1/$2.git; git -C "$2" checkout -q "$3"; }
clone mkubi FNIRSI-DPOS350P-Hack e9413b070eda216e69188222ca1be75646cf6b11
clone mkubi FNIRSI-DPOS350P-Boot 03c30e3420d37fdb61abbfd6b0eb2230aa7055ed
clone vander-mello dpos350p-analysis f9949bebea502cb13982c5e8b99e1cdbf0c344f4

# FatFs (ChaN, BSD-style licence) releases used by the Phase 4 compiler probe
mkdir -p fatfs ../tools/downloads/fatfs
fatfs() { # version sha256
  z=../tools/downloads/fatfs/ff$1.zip
  [ -f "$z" ] || curl -fsSL -o "$z" "http://elm-chan.org/fsw/ff/arc/ff$1.zip"
  echo "$2  $z" | sha256sum -c -
  [ -d fatfs/R0.$1 ] || { mkdir -p fatfs/R0.$1 && unzip -q "$z" -d fatfs/R0.$1; }
}
fatfs 13 1ba6f0a7e67eca1a46b65b6a0a4389513ddaa861846fcc499535c19908414b2c
fatfs 13a f8900045336a13988dde1b461448799fa50539fb265b6089850539f995330d5d
fatfs 13b 43a577724e5595d73c643891cb2959919885b458149794a085a98254784f5a9b
fatfs 13c 39feb00dced8585d0ec8f1f6acf47a2ae3ea4123c077a4c0f310b303e89ec391
fatfs 14 50a9e6b0f7384b5ab5708d0bb25ad6fe0f1de82b56d58a4b9aac0fad270d1c4a
