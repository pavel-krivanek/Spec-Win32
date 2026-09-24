# Native runtime source layout

The Win32 runtime remains one CRT-free C translation unit at build time, but the implementation is split into responsibility-oriented source modules. v052 added a dedicated editor module on top of the v051 split; v053 extends that same module with reusable patch/diff line-decoration metadata. `native/spec_win32_runtime.c` is intentionally a very small unity-build driver that includes these files in dependency order.

This is a conservative intermediate architecture: it removes the 7.4k-line editing bottleneck without changing private/static linkage, initialization order, generated code, exported ABI, or runtime behavior. Independent translation units can be introduced later once shared private declarations are moved into an internal header deliberately rather than as part of an unrelated feature change.

| Module | Responsibility |
| --- | --- |
| `spec_win32_platform.c` | minimal Win32 declarations/imports, constants, public ABI constants/types |
| `spec_win32_state.c` | global runtime state, event queue, UTF-16 offset/newline helpers |
| `spec_win32_editor.c` | RichEdit code-surface initialization, syntax-style spans, line-number/mark gutters and line decorations |
| `spec_win32_objects.c` | object registry/lifetime, menus/images, generic control subclass behavior |
| `spec_win32_collections.c` | control-kind mapping plus list/table/tree transports and TreeColumn composite support |
| `spec_win32_text.c` | UTF-8 conversion, tooltips and multiline text-area recreation |
| `spec_win32_commands.c` | UI-thread command completion, dialogs and command dispatcher |
| `spec_win32_windowing.c` | scrolling/paned geometry, top-level WndProc, class registration and UI-thread loop |
| `spec_win32_exports.c` | exported C ABI entry points |

## Build invariant

`native/build-runtime.sh` compiles only `native/spec_win32_runtime.c`. Do not compile the files in this directory separately unless the runtime is first converted to true independent translation units with an explicit internal header and non-static cross-module interfaces.

`native/verify-runtime.sh` searches the whole native source tree for ABI/runtime invariants, so verification remains valid after the split.
