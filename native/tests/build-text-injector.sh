#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TOOLS="${SPEC_WIN32_TOOLS_ROOT:-}"
if [[ -z "$TOOLS" ]]; then
  for candidate in "$ROOT/../Spec-Win32-Linux-Toolchain-v001" "/mnt/data/Spec-Win32-Linux-Toolchain-v001.full" "/mnt/data/Spec-Win32-Linux-Toolchain-v001"; do
    if [[ -x "$candidate/toolchain/bin/clang" ]]; then TOOLS="$candidate"; break; fi
  done
fi
[[ -n "$TOOLS" && -x "$TOOLS/toolchain/bin/clang" ]] || { echo "Set SPEC_WIN32_TOOLS_ROOT" >&2; exit 2; }
export PATH="$TOOLS/toolchain/bin:$PATH"
export LD_LIBRARY_PATH="$TOOLS/toolchain/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
RESOURCE_DIR="$TOOLS/toolchain/lib/clang/17"
OUT="${1:-$ROOT/bin/win64/tests}"
mkdir -p "$OUT"
lld-link /lib /machine:x64 /def:"$ROOT/native/tests/user32-text-injector.def" /out:"$OUT/user32-text-injector.lib"
lld-link /lib /machine:x64 /def:"$ROOT/native/tests/kernel32-text-injector.def" /out:"$OUT/kernel32-text-injector.lib"
clang --target=x86_64-pc-windows-msvc -std=c11 -O2 -Wall -Wextra -Werror -ffreestanding -fno-stack-protector -fno-builtin -resource-dir "$RESOURCE_DIR" -c "$ROOT/native/tests/text-injector.c" -o "$OUT/text-injector.obj"
lld-link /subsystem:console /entry:mainCRTStartup /machine:x64 /nodefaultlib /out:"$OUT/text-injector.exe" "$OUT/text-injector.obj" "$OUT/user32-text-injector.lib" "$OUT/kernel32-text-injector.lib"
rm -f "$OUT/text-injector.obj"
file "$OUT/text-injector.exe"
