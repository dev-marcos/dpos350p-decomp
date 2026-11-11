# Source this inside the dpos-re WSL distro:  source /mnt/f/dpos350p-decomp/tools/env.sh
# Every tool, cache, config and temp dir points inside <PROJECT>/tools.
_env_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export DPOS_ROOT="$(dirname "$_env_dir")"
export DPOS_TOOLS="$_env_dir"
unset _env_dir

export JAVA_HOME="$DPOS_TOOLS/jdk/current"
export GHIDRA_INSTALL_DIR="$DPOS_TOOLS/ghidra/current"
export NODE_HOME="$DPOS_TOOLS/node/current"
export ARM_GCC_HOME="$DPOS_TOOLS/compilers/arm-gnu-current"
export VIRTUAL_ENV="$DPOS_TOOLS/venv"

export HOME="$DPOS_TOOLS/home"
export TMPDIR="$DPOS_TOOLS/tmp"
export XDG_CACHE_HOME="$DPOS_TOOLS/cache"
export XDG_CONFIG_HOME="$DPOS_TOOLS/config"
export XDG_DATA_HOME="$DPOS_TOOLS/share"
export npm_config_cache="$DPOS_TOOLS/cache/npm"
export npm_config_prefix="$DPOS_TOOLS/rea"
export PIP_CACHE_DIR="$DPOS_TOOLS/cache/pip"
export PYTHONPYCACHEPREFIX="$DPOS_TOOLS/cache/pycache"

# Ghidra: keep user settings/projects/temp inside tools/
export _JAVA_OPTIONS="-Duser.home=$DPOS_TOOLS/home -Djava.io.tmpdir=$DPOS_TOOLS/tmp"
export GHIDRA_PROJECTS="$DPOS_TOOLS/ghidra-projects"
export REA_GHIDRA_STARTUP_TIMEOUT_MS="${REA_GHIDRA_STARTUP_TIMEOUT_MS:-600000}"
export REA_ANALYSIS_PROVIDER="${REA_ANALYSIS_PROVIDER:-ghidra}"

export PATH="$VIRTUAL_ENV/bin:$DPOS_TOOLS/rea/bin:$NODE_HOME/bin:$JAVA_HOME/bin:$GHIDRA_INSTALL_DIR/support:$ARM_GCC_HOME/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

mkdir -p "$HOME" "$TMPDIR" "$XDG_CACHE_HOME" "$XDG_CONFIG_HOME" "$XDG_DATA_HOME" "$GHIDRA_PROJECTS" 2>/dev/null
