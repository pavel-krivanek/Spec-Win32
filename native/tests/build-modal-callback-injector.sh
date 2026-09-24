#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TOOLS="${SPEC_WIN32_TOOLS_ROOT:-/mnt/data/Spec-Win32-Linux-Toolchain-v001}"
export PATH="$TOOLS/toolchain/bin:$PATH"
export LD_LIBRARY_PATH="$TOOLS/toolchain/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
RESOURCE_DIR="$TOOLS/toolchain/lib/clang/17"
OUT="${1:-$ROOT/bin/win64/tests}"
mkdir -p "$OUT"
lld-link /lib /machine:x64 /def:"$ROOT/native/tests/user32-event-injector.def" /out:"$OUT/user32-modal-callback.lib"
clang --target=x86_64-pc-windows-msvc -std=c11 -O2 -Wall -Wextra -Werror -ffreestanding -fno-stack-protector -fno-builtin -resource-dir "$RESOURCE_DIR" -c "$ROOT/native/tests/modal-callback-injector.c" -o "$OUT/modal-callback-injector.obj"
lld-link /subsystem:console /entry:mainCRTStartup /machine:x64 /nodefaultlib /out:"$OUT/modal-callback-injector.exe" "$OUT/modal-callback-injector.obj" "$OUT/user32-modal-callback.lib"
rm -f "$OUT/modal-callback-injector.obj"
file "$OUT/modal-callback-injector.exe"
