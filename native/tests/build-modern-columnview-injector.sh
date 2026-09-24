#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$ROOT/bin/win64/tests"
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
mkdir -p "$OUT"
lld-link /lib /machine:x64 /def:"$ROOT/native/tests/user32-modern-columnview-injector.def" /out:"$OUT/user32-modern-columnview-injector.lib"
lld-link /lib /machine:x64 /def:"$ROOT/native/tests/kernel32-modern-columnview-injector.def" /out:"$OUT/kernel32-modern-columnview-injector.lib"
clang --target=x86_64-pc-windows-msvc -std=c11 -O2 -Wall -Wextra -Werror -ffreestanding -fno-stack-protector -fno-builtin -resource-dir "$RESOURCE_DIR" -c "$ROOT/native/tests/modern-columnview-injector.c" -o "$OUT/modern-columnview-injector.obj"
lld-link /subsystem:console /entry:mainCRTStartup /machine:x64 /nodefaultlib /out:"$OUT/modern-columnview-injector.exe" "$OUT/modern-columnview-injector.obj" "$OUT/user32-modern-columnview-injector.lib" "$OUT/kernel32-modern-columnview-injector.lib"
rm -f "$OUT/modern-columnview-injector.obj"
file "$OUT/modern-columnview-injector.exe"
