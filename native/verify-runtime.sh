#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TOOLS="${SPEC_WIN32_TOOLS_ROOT:-}"
if [[ -z "$TOOLS" ]]; then
  for candidate in "$ROOT/../Spec-Win32-Linux-Toolchain-v001" "/mnt/data/Spec-Win32-Linux-Toolchain-v001"; do
    if [[ -x "$candidate/toolchain/bin/llvm-objdump" ]]; then TOOLS="$candidate"; break; fi
  done
fi
if [[ -z "$TOOLS" ]]; then
  echo "Set SPEC_WIN32_TOOLS_ROOT to Spec-Win32-Linux-Toolchain-v001" >&2
  exit 2
fi
export PATH="$TOOLS/toolchain/bin:$PATH"
export LD_LIBRARY_PATH="$TOOLS/toolchain/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
DLL="$ROOT/bin/win64/spec-win32-runtime.dll"
SOURCE_ROOT="$ROOT/native"
[[ -f "$DLL" ]] || { echo "Missing $DLL" >&2; exit 3; }
[[ -f "$SOURCE_ROOT/spec_win32_runtime.c" ]] || { echo "Missing unity source $SOURCE_ROOT/spec_win32_runtime.c" >&2; exit 3; }
[[ -d "$SOURCE_ROOT/runtime" ]] || { echo "Missing runtime module directory $SOURCE_ROOT/runtime" >&2; exit 3; }
source_has() { grep -q -- "$1" "$SOURCE_ROOT/spec_win32_runtime.c" "$SOURCE_ROOT"/runtime/*.c; }
source_has '#define SPW_ABI_VERSION 0x00290000u' || { echo "Native source ABI constant is not v0.41" >&2; exit 6; }
source_has 'static int spw_control_kind_is_valid' || { echo "Missing centralized control-kind validation" >&2; exit 6; }
source_has 'if (!spw_control_kind_is_valid(kind))' || { echo "spw_control_create does not use centralized control-kind validation" >&2; exit 6; }
source_has '#define SPW_CONTROL_TOGGLE_BUTTON 14u' || { echo "Missing ToggleButton control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_SWITCH 15u' || { echo "Missing Switch control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_SPINNER 16u' || { echo "Missing Spinner control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_LINK 17u' || { echo "Missing Link control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_NUMBER_INPUT 18u' || { echo "Missing NumberInput control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_SEARCH_INPUT 19u' || { echo "Missing SearchInput control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_IMAGE 20u' || { echo "Missing Image control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_SCROLL_VIEWPORT 21u' || { echo "Missing scroll viewport control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_SCROLL_CONTENT 22u' || { echo "Missing scroll content control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_PANED 23u' || { echo "Missing Paned control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_PANED_CONTENT 24u' || { echo "Missing Paned content control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_TAB_CONTENT 25u' || { echo "Missing Tab content control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_FRAME 26u' || { echo "Missing Frame control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_TEXT_AREA 27u' || { echo "Missing multiline Text control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_TREE_COLUMN 28u' || { echo "Missing TreeColumnView composite control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_COMPONENT_ROW 29u' || { echo "Missing ComponentList row-host control kind" >&2; exit 6; }
source_has '#define SPW_CONTROL_CODE 30u' || { echo "Missing native Code/RichEdit control kind" >&2; exit 6; }
source_has '#define SPW_MAX_CODE_MARKS 512u' || { echo "Missing code line-decoration capacity" >&2; exit 6; }
source_has 'spw_code_set_line_decorations_impl' || { echo "Missing code line-decoration implementation" >&2; exit 6; }
source_has '#define SPW_SCROLL_POLICY_DISABLED 0u' || { echo "Missing disabled scroll policy" >&2; exit 6; }
source_has '#define SPW_SCROLL_POLICY_AUTOMATIC 1u' || { echo "Missing automatic scroll policy" >&2; exit 6; }
source_has '#define SPW_SCROLL_POLICY_HIDDEN 2u' || { echo "Missing hidden scroll policy" >&2; exit 6; }
source_has '#define SPW_SCROLL_POLICY_ALWAYS 3u' || { echo "Missing always scroll policy" >&2; exit 6; }
source_has 'object->kind == SPW_CONTROL_FRAME' || { echo "Missing Frame child-notification forwarding" >&2; exit 6; }
source_has 'static int32_t spw_refresh_button_image_list' || { echo "Missing Button image-list refresh path" >&2; exit 6; }
source_has 'static int32_t spw_list_apply_rows' || { echo "Missing modern ListView row-image transport" >&2; exit 6; }
source_has 'static int32_t spw_table_apply_cells_with_images' || { echo "Missing ColumnView/table cell-image transport" >&2; exit 6; }
source_has 'static int32_t spw_tree_apply_nodes_with_images' || { echo "Missing TreeListView node-image transport" >&2; exit 6; }
source_has 'LVS_EX_SUBITEMIMAGES_VALUE' || { echo "Missing ListView subitem-image support" >&2; exit 6; }
source_has 'list_image_list' || { echo "Missing ListView image-list lifetime state" >&2; exit 6; }
source_has 'LVM_SETIMAGELIST_VALUE' || { echo "Missing ListView small-image-list attachment" >&2; exit 6; }
source_has 'BCM_SETIMAGELIST_VALUE' || { echo "Missing standard Button image-list attachment" >&2; exit 6; }
source_has '#define SPW_EVENT_NUMBER_STEP 26u' || { echo "Missing NumberInput step event" >&2; exit 6; }
source_has '#define SPW_EVENT_POPUP_DISMISS_REQUESTED 27u' || { echo "Missing popup dismiss-request event" >&2; exit 6; }
source_has '#define SPW_EVENT_MENU_COMMAND 28u' || { echo "Missing persistent menu command event" >&2; exit 6; }
source_has '#define SPW_EVENT_PANED_POSITION_CHANGED 29u' || { echo "Missing Paned position event" >&2; exit 6; }
source_has '#define SPW_EVENT_SCROLL_POSITION_CHANGED 30u' || { echo "Missing scroll-position event" >&2; exit 6; }
source_has 'SPW_CMD_SET_WINDOW_MENU' || { echo "Missing persistent window-menu command" >&2; exit 6; }
source_has 'SPW_CMD_CREATE_POPUP_WINDOW' || { echo "Missing native popup-window command" >&2; exit 6; }
source_has 'WS_EX_TOOLWINDOW_VALUE' || { echo "Missing popup tool-window style" >&2; exit 6; }
source_has 'kind <= SPW_CONTROL_CODE' || { echo "Control-kind validator does not include Code/RichEdit" >&2; exit 6; }
if source_has 'kind > SPW_CONTROL_NOTEBOOK'; then
  echo "Stale Notebook upper bound would reject post-Notebook controls" >&2
  exit 6
fi
file "$DLL" | grep -q 'PE32+.*x86-64'
DUMP="$(llvm-objdump -p "$DLL")"
for symbol in \
  spw_abi_version spw_runtime_start spw_runtime_stop spw_wait_for_activity spw_event_pop \
  spw_window_create spw_window_show spw_window_set_title spw_window_set_menu spw_window_set_bounds \
  spw_window_get_client_size spw_window_destroy spw_window_is_visible spw_window_activate \
  spw_window_get_bounds spw_window_is_minimized spw_window_is_maximized spw_window_is_foreground \
  spw_popup_window_create \
  spw_window_center spw_window_center_relative spw_window_set_owner spw_window_set_wait_cursor spw_window_wait_cursor_active spw_show_message spw_file_dialog \
  spw_control_create spw_control_set_text spw_control_set_tooltip spw_control_get_text \
  spw_control_set_context_menu_enabled spw_control_show_context_menu spw_control_show_popup_menu \
  spw_control_get_screen_bounds spw_object_get_monitor_work_area spw_get_cursor_position \
  spw_control_set_checked spw_control_get_checked spw_control_set_editable \
  spw_control_set_max_length spw_control_set_password spw_control_set_placeholder \
  spw_control_set_image_bgra spw_control_set_image_auto_scale \
  spw_control_set_items spw_list_set_rows spw_control_set_selected_index spw_control_get_selected_index \
  spw_control_set_value spw_control_get_value spw_control_set_indeterminate \
  spw_control_set_enabled spw_control_show spw_control_set_focus spw_control_has_focus \
  spw_control_set_selection spw_control_get_selection spw_text_configure spw_text_replace_selection spw_text_command spw_text_scroll_to_line spw_text_first_visible_line \
  spw_control_set_selected_indexes spw_control_get_selected_indexes \
  spw_control_set_multiple_selection spw_control_set_header spw_control_ensure_visible \
  spw_control_set_table_columns spw_control_set_table_cells spw_table_set_cells_with_images \
  spw_control_set_tree_nodes spw_tree_set_nodes_with_images spw_control_set_tree_selected_token spw_control_get_tree_selected_token \
  spw_control_tree_set_expanded spw_control_tree_is_expanded spw_control_tree_ensure_visible \
  spw_control_set_bounds_batch spw_control_set_z_order spw_scroll_view_configure spw_scroll_view_configure_policies spw_scroll_view_get_page_extent spw_scroll_view_set_position spw_paned_configure spw_paned_get_position spw_notebook_get_content_rect spw_control_destroy spw_control_measure \
  spw_code_set_line_numbers spw_code_set_styles spw_code_set_line_decorations \
  spw_runtime_barrier; do
  grep -q " $symbol$" <<<"$DUMP" || { echo "Missing export: $symbol" >&2; exit 4; }
done
grep -q 'DLL Name: USER32.dll' <<<"$DUMP"
grep -q 'DLL Name: KERNEL32.dll' <<<"$DUMP"
grep -q 'DLL Name: GDI32.dll' <<<"$DUMP"
grep -q 'DLL Name: COMCTL32.dll' <<<"$DUMP"
grep -q 'DLL Name: MSIMG32.dll' <<<"$DUMP"
grep -q 'DLL Name: COMDLG32.dll' <<<"$DUMP"
grep -q 'DLL Name: SHELL32.dll' <<<"$DUMP"
grep -q 'DLL Name: OLE32.dll' <<<"$DUMP"
grep -q 'GetSysColorBrush' <<<"$DUMP" || { echo "Missing USER32 GetSysColorBrush import" >&2; exit 4; }
grep -q 'RoundRect' <<<"$DUMP" || { echo "Missing GDI32 RoundRect import" >&2; exit 4; }
grep -q 'MoveToEx' <<<"$DUMP" || { echo "Missing GDI32 MoveToEx import" >&2; exit 4; }
grep -q 'LineTo' <<<"$DUMP" || { echo "Missing GDI32 LineTo import" >&2; exit 4; }
grep -q 'CreateDIBSection' <<<"$DUMP" || { echo "Missing GDI32 CreateDIBSection import" >&2; exit 4; }
grep -q 'AlphaBlend' <<<"$DUMP" || { echo "Missing MSIMG32 AlphaBlend import" >&2; exit 4; }
grep -q 'ImageList_Create' <<<"$DUMP" || { echo "Missing COMCTL32 ImageList_Create import" >&2; exit 4; }
grep -q 'ImageList_Add' <<<"$DUMP" || { echo "Missing COMCTL32 ImageList_Add import" >&2; exit 4; }
grep -q 'ImageList_Destroy' <<<"$DUMP" || { echo "Missing COMCTL32 ImageList_Destroy import" >&2; exit 4; }
grep -q 'SetMenuItemBitmaps' <<<"$DUMP" || { echo "Missing USER32 SetMenuItemBitmaps import" >&2; exit 4; }
grep -q 'CreateMenu' <<<"$DUMP" || { echo "Missing USER32 CreateMenu import" >&2; exit 4; }
grep -q 'SetMenu' <<<"$DUMP" || { echo "Missing USER32 SetMenu import" >&2; exit 4; }
grep -q 'DrawMenuBar' <<<"$DUMP" || { echo "Missing USER32 DrawMenuBar import" >&2; exit 4; }
grep -q 'GetCursorPos' <<<"$DUMP" || { echo "Missing USER32 GetCursorPos import" >&2; exit 4; }
grep -q 'MessageBoxW' <<<"$DUMP" || { echo "Missing USER32 MessageBoxW import" >&2; exit 4; }
grep -q 'SetCursor' <<<"$DUMP" || { echo "Missing USER32 SetCursor import" >&2; exit 4; }
grep -q 'GetCursor' <<<"$DUMP" || { echo "Missing USER32 GetCursor import" >&2; exit 4; }
grep -q 'GetOpenFileNameW' <<<"$DUMP" || { echo "Missing COMDLG32 GetOpenFileNameW import" >&2; exit 4; }
grep -q 'SHBrowseForFolderW' <<<"$DUMP" || { echo "Missing SHELL32 SHBrowseForFolderW import" >&2; exit 4; }
grep -q 'CoTaskMemFree' <<<"$DUMP" || { echo "Missing OLE32 CoTaskMemFree import" >&2; exit 4; }
grep -q 'SetTimer' <<<"$DUMP" || { echo "Missing USER32 SetTimer import" >&2; exit 4; }
grep -q 'KillTimer' <<<"$DUMP" || { echo "Missing USER32 KillTimer import" >&2; exit 4; }
for symbol in GetScrollInfo SetScrollInfo ShowScrollBar FillRect SetCapture ReleaseCapture GetParent SetWindowPos; do
  grep -q "$symbol" <<<"$DUMP" || { echo "Missing USER32 $symbol import" >&2; exit 4; }
done
if grep -Eqi 'DLL Name: (ucrtbase|msvcrt|vcruntime)' <<<"$DUMP"; then
  echo "Unexpected CRT dependency" >&2
  exit 5
fi
echo "Spec Win32 runtime verification: OK"
