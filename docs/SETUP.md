# Environment setup

Everything the project needs lives inside the project folder (`<PROJECT>` =
`F:\dpos350p-decomp`, `/mnt/f/dpos350p-decomp` inside WSL). The Windows system drive has very
little free space, so nothing is installed there. The only change outside the project
is the user's global `.wslconfig` (see below).

## Host

| Item | Value |
|---|---|
| OS | Windows 11 Pro 10.0.26200 |
| WSL | 3.0.5, kernel 6.18.58.1 (already installed; no admin or reboot needed) |
| Distro | `dpos-re`, Ubuntu 24.04.5 LTS, WSL2 |
| Distro disk | `<PROJECT>\tools\wsl\ext4.vhdx` |
| WSL swap | `<PROJECT>\tools\wsl\swap.vhdx` (via `%USERPROFILE%\.wslconfig`) |

`%USERPROFILE%\.wslconfig` (global, it also applies to the other distros):

```ini
[wsl2]
swapFile=F:\\dpos350p-decomp\\tools\\wsl\\swap.vhdx
```

The backslashes must be doubled, because WSL rejects `\d` as an invalid escape.

## Reproducing from scratch

```powershell
# 1. distro (image SHA-256 from Microsoft's DistributionInfo.json:
#    bb415d824822c4b878125729af451a5d18fb13d1cf5cbed9a7393ad64ac6039e)
curl.exe -L -o tools\downloads\ubuntu-24.04.5-wsl-amd64.wsl https://releases.ubuntu.com/24.04.5/ubuntu-24.04.5-wsl-amd64.wsl
wsl --install --from-file tools\downloads\ubuntu-24.04.5-wsl-amd64.wsl --name dpos-re --location F:\dpos350p-decomp\tools\wsl --no-launch
wsl -d dpos-re -u root -- bash /mnt/f/dpos350p-decomp/tools/setup/00-distro-root.sh
wsl --shutdown          # applies /etc/wsl.conf (default user dev, metadata mounts)
# 2. portable tools, venv, REA, references
wsl -d dpos-re -u dev -- bash /mnt/f/dpos350p-decomp/tools/setup/10-tools.sh
wsl -d dpos-re -u dev -- bash /mnt/f/dpos350p-decomp/tools/setup/30-python-rea.sh
wsl -d dpos-re -u dev -- bash /mnt/f/dpos350p-decomp/tools/setup/20-third-party.sh
# 3. acceptance
wsl -d dpos-re -u dev -- bash /mnt/f/dpos350p-decomp/tools/setup/check.sh
wsl -d dpos-re -u dev -- bash /mnt/f/dpos350p-decomp/tools/setup/rea-doctor.sh
```

Daily use:

```bash
wsl -d dpos-re -- bash -lc "source /mnt/f/dpos350p-decomp/tools/env.sh && <command>"
```

## Installed components

| Component | Version | Path (under `tools/`) | Size |
|---|---|---|---|
| Ubuntu distro and apt packages (build-essential, git, make, unzip, xz-utils, python3-venv, python3-dev, curl, file, bison, flex) | 24.04.5 | `wsl/ext4.vhdx` | 2.2 GB |
| Eclipse Temurin JDK | 21.0.13+9 | `jdk/jdk-21.0.13+9` (`jdk/current`) | 345 MB |
| Ghidra | 12.1.4 (PUBLIC_20260921) | `ghidra/ghidra_12.1.4_PUBLIC` (`ghidra/current`) | 864 MB |
| Node.js | 22.23.4 | `node/node-v22.23.4-linux-x64` (`node/current`) | 187 MB |
| REA (`rea-agents`) | 6.8.0 | `rea/` (installed with `npm -g --prefix`), CLI `rea/bin/rea` | 249 MB |
| Arm GNU Toolchain (arm-none-eabi) | 15.2.Rel1 (gcc 15.2.1 20251203) | `compilers/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi` (`compilers/arm-gnu-current`) | 1.1 GB |
| Python venv | 3.12.3; capstone 5.0.7, unicorn 2.1.4, pillow 13.0.0, numpy 2.5.4 | `venv/` (`tools/requirements.txt`) | 144 MB |
| Downloaded archives (can be deleted) | | `downloads/` | 1.3 GB |
| npm/pip caches | | `cache/` | 213 MB |
| Arm Compiler 5.06u7 build 960 (licensed; **unusable without a licence**, see docs/COMPILER.md �5) | 5.06u7 | `compilers/armcc-5.06u7/` (Windows build, `msiexec /a` extraction) | 462 MB |
| Keil MDK 5.43a licence-variant maps only (installer not executed) | 5.43a | `keil/ARM/sw/mappings/` | 0.1 MB |
| FatFs R0.13�R0.14 source archives (Phase 4 probe) | | `downloads/fatfs/`, unpacked to `third_party/fatfs/` | 12 MB |
| AC5 / MDK installers supplied by the owner (can be deleted) | | `downloads/armcc/`, `downloads/keil/` | 1.0 GB |
| **Total** | | | **~8.2 GB** |

The archive checksums are verified by `10-tools.sh`: Node against nodejs.org's SHASUMS256,
the JDK against Adoptium's `.sha256.txt`, and the Arm toolchain against Arm's `.sha256asc`.

The reference repositories are cloned read-only in `third_party/` (git-ignored, pinned by
`tools/setup/20-third-party.sh`):

- mkubi/FNIRSI-DPOS350P-Hack @ `e9413b0`
- mkubi/FNIRSI-DPOS350P-Boot @ `03c30e3`
- vander-mello/dpos350p-analysis @ `f9949be`

## `tools/env.sh`

It sets `PATH`, `JAVA_HOME`, `GHIDRA_INSTALL_DIR`, `HOME`, `TMPDIR`, `XDG_*`,
`npm_config_cache`, `npm_config_prefix`, `PIP_CACHE_DIR`, `PYTHONPYCACHEPREFIX`,
`_JAVA_OPTIONS="-Duser.home=… -Djava.io.tmpdir=…"` (Ghidra settings) and
`REA_GHIDRA_STARTUP_TIMEOUT_MS=600000`. All of them point inside `tools/`. Windows paths are not
appended to `PATH`, because `/etc/wsl.conf` sets `appendWindowsPath=false`.

## REA MCP registration

There is a project-scoped `.mcp.json` at the repo root:

```json
{"mcpServers":{"rea-agents":{"command":"wsl.exe",
  "args":["-d","dpos-re","-u","dev","--","bash","/mnt/f/dpos350p-decomp/tools/rea-mcp.sh"]}}}
```

`tools/rea-mcp.sh` sources `env.sh` and runs `rea mcp` over stdio. A JSON-RPC `initialize`
probe answers with `serverInfo rea 6.8.0`. Claude Code loads the server on the next session
start. The fallback is the CLI: `wsl -d dpos-re -- bash -lc "source …/env.sh && rea …"`.

## Verified (Phase 0 acceptance)

- `tools/setup/check.sh`: java/javac 21.0.13, Ghidra 12.1.4 with the linux_x86_64 native
  decompiler, node 22.23.4, `arm-none-eabi-gcc` 15.2.1, the Python imports and `rea` all
  resolve from `tools/`. The ISO SHA-256 matches `6635ddd6…d58c`.
- `rea doctor --provider ghidra`: `"healthy": true`. The only failures are the providers this
  project doesn't use (Hopper, IDA, jadx, Kuna) and a missing optional REA skill.
- `tools/setup/rea-smoke.sh`: `rea inspect build/app.bin --target-format raw-binary
  --architecture arm --entry-point 0x40000000 --provider ghidra` takes about 2 minutes and
  reports language `ARM:LE:32:v7`, one segment `0x40000000–0x400957E0`, 1073 procedures and
  387 strings. The evidence is in `evidence/phase0-rea-inspect.json`.
- Free space: C: had 3.13 GB at the start and 20.78 GB at the end. The increase came from
  outside this project, and nothing was written to C: apart from `.wslconfig`.

## Notes

- Unpacking archives on `/mnt/f` (drvfs/9p) is slow. Extracting Ghidra took about 10 minutes
  and `npm install` about 10 minutes. Analysis itself is fine.
- The `make` on the Windows `PATH` is Embarcadero's and is not used. All builds run inside
  `dpos-re`.
