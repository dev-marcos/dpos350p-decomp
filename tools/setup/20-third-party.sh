#!/bin/bash
# Clone read-only reference repositories into third_party/ at the pinned commits.
set -euo pipefail
cd "$(dirname "$0")/../../third_party" 2>/dev/null || { mkdir -p "$(dirname "$0")/../../third_party"; cd "$(dirname "$0")/../../third_party"; }
clone() { [ -d "$2" ] || git clone -q https://github.com/$1/$2.git; git -C "$2" checkout -q "$3"; }
clone mkubi FNIRSI-DPOS350P-Hack e9413b070eda216e69188222ca1be75646cf6b11
clone mkubi FNIRSI-DPOS350P-Boot 03c30e3420d37fdb61abbfd6b0eb2230aa7055ed
clone vander-mello dpos350p-analysis f9949bebea502cb13982c5e8b99e1cdbf0c344f4
