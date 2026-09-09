#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
BIN_DIR="${SCRIPT_DIR}/build-linux"

if [ ! -x "${BIN_DIR}/SurrealEngine" ]; then
	echo "Could not find build-linux/SurrealEngine. Build the project first, or copy the packaged binaries next to this script." >&2
	exit 1
fi

export LD_LIBRARY_PATH="${BIN_DIR}:${LD_LIBRARY_PATH:-}"

exec "${BIN_DIR}/SurrealEngine" "$@"
