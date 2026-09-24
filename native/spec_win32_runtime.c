/*
 * Spec Win32 native runtime unity translation unit.
 *
 * The implementation is split by responsibility under native/runtime/.
 * We intentionally keep a unity build for now: it preserves the existing
 * private/static linkage and makes this refactor behavior- and ABI-neutral.
 */

#include "runtime/spec_win32_platform.c"
#include "runtime/spec_win32_state.c"
#include "runtime/spec_win32_editor.c"
#include "runtime/spec_win32_objects.c"
#include "runtime/spec_win32_collections.c"
#include "runtime/spec_win32_text.c"
#include "runtime/spec_win32_commands.c"
#include "runtime/spec_win32_windowing.c"
#include "runtime/spec_win32_exports.c"
