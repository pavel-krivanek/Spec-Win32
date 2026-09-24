#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TOOLS="${SPEC_WIN32_TOOLS_ROOT:-}"
if [[ -z "$TOOLS" ]]; then
  for candidate in "$ROOT/../Spec-Win32-Linux-Toolchain-v001" "/mnt/data/Spec-Win32-Linux-Toolchain-v001"; do
    if [[ -x "$candidate/toolchain/bin/clang" ]]; then TOOLS="$candidate"; break; fi
  done
fi
if [[ -z "$TOOLS" || ! -x "$TOOLS/toolchain/bin/clang" ]]; then
  echo "Set SPEC_WIN32_TOOLS_ROOT to the extracted Spec-Win32-Linux-Toolchain-v001 directory." >&2
  exit 2
fi
export PATH="$TOOLS/toolchain/bin:$PATH"
export LD_LIBRARY_PATH="$TOOLS/toolchain/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
RESOURCE_DIR="$TOOLS/toolchain/lib/clang/17"
OUT="${1:-$ROOT/bin/win64/tests}"
mkdir -p "$OUT"
lld-link /lib /machine:x64 /def:"$ROOT/native/tests/user32-frame-injector.def" /out:"$OUT/user32-frame-injector.lib"
lld-link /lib /machine:x64 /def:"$ROOT/native/tests/kernel32-frame-injector.def" /out:"$OUT/kernel32-frame-injector.lib"
clang --target=x86_64-pc-windows-msvc -std=c11 -O2 -Wall -Wextra -Werror -ffreestanding -fno-stack-protector -fno-builtin -resource-dir "$RESOURCE_DIR" -c "$ROOT/native/tests/frame-injector.c" -o "$OUT/frame-injector.obj"
lld-link /subsystem:console /entry:mainCRTStartup /machine:x64 /nodefaultlib /out:"$OUT/frame-injector.exe" "$OUT/frame-injector.obj" "$OUT/user32-frame-injector.lib" "$OUT/kernel32-frame-injector.lib"
rm -f "$OUT/frame-injector.obj"
file "$OUT/frame-injector.exe"
