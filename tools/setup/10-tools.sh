#!/bin/bash
# Run as user 'dev' inside dpos-re. Downloads and unpacks portable tools into tools/.
set -euo pipefail
source "$(dirname "$0")/../env.sh"
DL="$DPOS_TOOLS/downloads"; mkdir -p "$DL"
fetch() { # url file [sha256]
  if [ ! -f "$DL/$2" ]; then curl -fsSL -o "$DL/$2.part" "$1"; mv "$DL/$2.part" "$DL/$2"; fi
  if [ -n "${3:-}" ]; then echo "$3  $DL/$2" | sha256sum -c -; fi
}
unpack() { # file destdir linkname
  mkdir -p "$2"; if [ ! -e "$2/.done-$(basename "$1")" ]; then
    case "$1" in *.zip) unzip -q -o "$1" -d "$2";; *) tar -xf "$1" -C "$2";; esac
    touch "$2/.done-$(basename "$1")"; fi
}
# --- JDK 21 (Temurin) ---
JDK=OpenJDK21U-jdk_x64_linux_hotspot_21.0.13_9.tar.gz
fetch "https://github.com/adoptium/temurin21-binaries/releases/download/jdk-21.0.13%2B9/$JDK" $JDK
curl -fsSL "https://github.com/adoptium/temurin21-binaries/releases/download/jdk-21.0.13%2B9/$JDK.sha256.txt" | awk '{print $1"  '"$DL/$JDK"'"}' | sha256sum -c -
unpack "$DL/$JDK" "$DPOS_TOOLS/jdk"; ln -sfn jdk-21.0.13+9 "$DPOS_TOOLS/jdk/current"
# --- Ghidra 12.1.4 ---
GZ=ghidra_12.1.4_PUBLIC_20260921.zip
fetch "https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_12.1.4_build/$GZ" $GZ
unpack "$DL/$GZ" "$DPOS_TOOLS/ghidra"; ln -sfn ghidra_12.1.4_PUBLIC "$DPOS_TOOLS/ghidra/current"
# --- Node.js 22 ---
NZ=node-v22.23.4-linux-x64.tar.xz
fetch "https://nodejs.org/dist/v22.23.4/$NZ" $NZ 1805bcd35e6815559bd2eb97db438d3fdb0eb3a5e6a977b7c1badca76337e9ce
unpack "$DL/$NZ" "$DPOS_TOOLS/node"; ln -sfn node-v22.23.4-linux-x64 "$DPOS_TOOLS/node/current"
# --- Arm GNU Toolchain 15.2.rel1 ---
AZ=arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz
fetch "https://developer.arm.com/-/media/Files/downloads/gnu/15.2.rel1/binrel/$AZ" $AZ
curl -fsSL "https://developer.arm.com/-/media/Files/downloads/gnu/15.2.rel1/binrel/$AZ.sha256asc" | awk '{print $1"  '"$DL/$AZ"'"}' | sha256sum -c -
unpack "$DL/$AZ" "$DPOS_TOOLS/compilers"; ln -sfn arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi "$DPOS_TOOLS/compilers/arm-gnu-current"
echo TOOLS_OK
