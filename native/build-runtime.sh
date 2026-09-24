#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TOOLS="${SPEC_WIN32_TOOLS_ROOT:-}"
if [[ -z "$TOOLS" ]]; then
  for candidate in \
    "$ROOT/../Spec-Win32-Linux-Toolchain-v001" \
    "/mnt/data/Spec-Win32-Linux-Toolchain-v001"; do
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
OUT="$ROOT/bin/win64"
OBJ="$OUT/spec_win32_runtime.obj"
mkdir -p "$OUT/lib"

lld-link /lib /machine:x64 /def:"$ROOT/native/imports/kernel32.def" /out:"$OUT/lib/kernel32.lib"
lld-link /lib /machine:x64 /def:"$ROOT/native/imports/user32.def" /out:"$OUT/lib/user32.lib"
lld-link /lib /machine:x64 /def:"$ROOT/native/imports/gdi32.def" /out:"$OUT/lib/gdi32.lib"
lld-link /lib /machine:x64 /def:"$ROOT/native/imports/comctl32.def" /out:"$OUT/lib/comctl32.lib"
lld-link /lib /machine:x64 /def:"$ROOT/native/imports/msimg32.def" /out:"$OUT/lib/msimg32.lib"
lld-link /lib /machine:x64 /def:"$ROOT/native/imports/comdlg32.def" /out:"$OUT/lib/comdlg32.lib"
lld-link /lib /machine:x64 /def:"$ROOT/native/imports/shell32.def" /out:"$OUT/lib/shell32.lib"
lld-link /lib /machine:x64 /def:"$ROOT/native/imports/ole32.def" /out:"$OUT/lib/ole32.lib"

clang --target=x86_64-pc-windows-msvc -std=c11 -O2 -Wall -Wextra -Werror \
  -ffreestanding -fno-stack-protector -fno-builtin \
  -resource-dir "$RESOURCE_DIR" \
  -c "$ROOT/native/spec_win32_runtime.c" -o "$OBJ"

lld-link /dll /noentry /machine:x64 /nodefaultlib \
  /out:"$OUT/spec-win32-runtime.dll" \
  "$OBJ" "$OUT/lib/user32.lib" "$OUT/lib/gdi32.lib" "$OUT/lib/msimg32.lib" "$OUT/lib/comctl32.lib" "$OUT/lib/comdlg32.lib" "$OUT/lib/shell32.lib" "$OUT/lib/ole32.lib" "$OUT/lib/kernel32.lib"
rm -f "$OBJ"

file "$OUT/spec-win32-runtime.dll"
llvm-objdump -p "$OUT/spec-win32-runtime.dll" > "$OUT/spec-win32-runtime.objdump.txt"
printf '\nExports/imports written to %s\n' "$OUT/spec-win32-runtime.objdump.txt"
