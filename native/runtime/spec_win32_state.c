/* Internal state -------------------------------------------------------- */
typedef struct {
    uint64_t id;
    uint64_t parent_id;
    HWND hwnd;
    HWND tree_hwnd;
    HWND table_hwnd;
    uint32_t type;
    uint32_t kind;
    uint32_t alive;
    uint32_t suppress_notifications;
    int32_t normalized_value;
    uint32_t indeterminate;
    uint32_t component_row_selected;
    uint32_t code_line_numbers;
    uint32_t code_foreground_rgb;
    uint32_t code_background_rgb;
    uint32_t code_mark_count;
    int32_t code_mark_lines[SPW_MAX_CODE_MARKS];
    uint32_t code_mark_kinds[SPW_MAX_CODE_MARKS];
    uint32_t code_mark_rgbs[SPW_MAX_CODE_MARKS];
    uint32_t table_column_count;
    uint32_t table_headers_visible;
    uint32_t mouse_inside;
    uint32_t context_menu_enabled;
    uint32_t spinner_phase;
    HBITMAP image_bitmap;
    HIMAGELIST button_image_list;
    HIMAGELIST list_image_list;
    HIMAGELIST tree_column_tree_image_list;
    HIMAGELIST tree_column_table_image_list;
    int32_t image_width;
    int32_t image_height;
    uint32_t image_auto_scale;
    uint32_t text_undo_enabled;
    uint32_t popup_window;
    uint32_t popup_autohide;
    uint32_t wait_cursor;
    uint64_t scroll_content_id;
    int32_t scroll_x;
    int32_t scroll_y;
    int32_t scroll_content_width;
    int32_t scroll_content_height;
    uint32_t scroll_h_enabled;
    uint32_t scroll_v_enabled;
    uint32_t scroll_h_policy;
    uint32_t scroll_v_policy;
    uint64_t paned_first_content_id;
    uint64_t paned_second_content_id;
    int32_t paned_position;
    int32_t paned_last_main_extent;
    int32_t paned_first_min;
    int32_t paned_second_min;
    int32_t paned_drag_offset;
    uint32_t paned_vertical;
    uint32_t paned_first_present;
    uint32_t paned_second_present;
    uint32_t paned_first_resize;
    uint32_t paned_second_resize;
    uint32_t paned_dragging;
    uint32_t paned_initialized;
    int32_t mouse_x;
    int32_t mouse_y;
    HWND tooltip_hwnd;
    WCHAR tooltip_text[SPW_MAX_TOOLTIP_WCHARS];
    WCHAR placeholder_text[SPW_MAX_PLACEHOLDER_WCHARS];
    int32_t table_column_widths[SPW_MAX_TABLE_COLUMNS];
    int32_t table_column_expandables[SPW_MAX_TABLE_COLUMNS];
} SpwObject;

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t object_id;
    int64_t argument1;
    int64_t argument2;
} SpwEvent;

_Static_assert(sizeof(SpwEvent) == 32, "SpwEvent ABI must remain 32 bytes");

typedef struct {
    uint64_t object_id;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} SpwBounds;

_Static_assert(sizeof(SpwBounds) == 24, "SpwBounds ABI must remain 24 bytes");

typedef enum {
    SPW_CMD_NONE = 0,
    SPW_CMD_CREATE_WINDOW,
    SPW_CMD_CREATE_POPUP_WINDOW,
    SPW_CMD_SHOW_WINDOW,
    SPW_CMD_SET_TITLE,
    SPW_CMD_SET_WINDOW_MENU,
    SPW_CMD_SET_BOUNDS,
    SPW_CMD_DESTROY_WINDOW,
    SPW_CMD_IS_VISIBLE,
    SPW_CMD_ACTIVATE_WINDOW,
    SPW_CMD_GET_CLIENT_SIZE,
    SPW_CMD_GET_WINDOW_BOUNDS,
    SPW_CMD_IS_MINIMIZED,
    SPW_CMD_IS_MAXIMIZED,
    SPW_CMD_IS_FOREGROUND,
    SPW_CMD_CENTER_WINDOW,
    SPW_CMD_CENTER_WINDOW_RELATIVE,
    SPW_CMD_SET_WINDOW_OWNER,
    SPW_CMD_SET_WINDOW_ENABLED,
    SPW_CMD_IS_WINDOW_ENABLED,
    SPW_CMD_CREATE_CONTROL,
    SPW_CMD_CONTROL_SET_TEXT,
    SPW_CMD_CONTROL_SET_TOOLTIP,
    SPW_CMD_CONTROL_SET_CONTEXT_MENU_ENABLED,
    SPW_CMD_CONTROL_SHOW_CONTEXT_MENU,
    SPW_CMD_CONTROL_SHOW_POPUP_MENU,
    SPW_CMD_CONTROL_GET_TEXT,
    SPW_CMD_CONTROL_SET_CHECKED,
    SPW_CMD_CONTROL_GET_CHECKED,
    SPW_CMD_CONTROL_SET_EDITABLE,
    SPW_CMD_CONTROL_SET_MAX_LENGTH,
    SPW_CMD_CONTROL_SET_PASSWORD,
    SPW_CMD_CONTROL_SET_PLACEHOLDER,
    SPW_CMD_CONTROL_SET_IMAGE_BGRA,
    SPW_CMD_CONTROL_SET_IMAGE_AUTO_SCALE,
    SPW_CMD_CONTROL_SET_ITEMS,
    SPW_CMD_CONTROL_SET_LIST_ROWS,
    SPW_CMD_CONTROL_SET_SELECTED_INDEX,
    SPW_CMD_CONTROL_GET_SELECTED_INDEX,
    SPW_CMD_CONTROL_SET_VALUE,
    SPW_CMD_CONTROL_GET_VALUE,
    SPW_CMD_CONTROL_SET_INDETERMINATE,
    SPW_CMD_CONTROL_SET_ENABLED,
    SPW_CMD_CONTROL_SHOW,
    SPW_CMD_CONTROL_SET_FOCUS,
    SPW_CMD_CONTROL_HAS_FOCUS,
    SPW_CMD_CONTROL_SET_SELECTION,
    SPW_CMD_CONTROL_GET_SELECTION,
    SPW_CMD_TEXT_CONFIGURE,
    SPW_CMD_TEXT_REPLACE_SELECTION,
    SPW_CMD_TEXT_COMMAND,
    SPW_CMD_TEXT_SCROLL_TO_LINE,
    SPW_CMD_CODE_SET_LINE_NUMBERS,
    SPW_CMD_CODE_SET_STYLES,
    SPW_CMD_CODE_SET_LINE_DECORATIONS,
    SPW_CMD_CONTROL_SET_SELECTED_INDEXES,
    SPW_CMD_CONTROL_GET_SELECTED_INDEXES,
    SPW_CMD_CONTROL_SET_MULTIPLE_SELECTION,
    SPW_CMD_CONTROL_SET_HEADER,
    SPW_CMD_CONTROL_SET_TABLE_COLUMNS,
    SPW_CMD_CONTROL_SET_TABLE_CELLS,
    SPW_CMD_CONTROL_SET_TABLE_CELLS_WITH_IMAGES,
    SPW_CMD_CONTROL_SET_TREE_NODES,
    SPW_CMD_CONTROL_SET_TREE_NODES_WITH_IMAGES,
    SPW_CMD_CONTROL_SET_TREE_SELECTED_TOKEN,
    SPW_CMD_CONTROL_GET_TREE_SELECTED_TOKEN,
    SPW_CMD_CONTROL_TREE_SET_EXPANDED,
    SPW_CMD_CONTROL_TREE_IS_EXPANDED,
    SPW_CMD_CONTROL_TREE_ENSURE_VISIBLE,
    SPW_CMD_CONTROL_ENSURE_VISIBLE,
    SPW_CMD_CONTROL_SET_BOUNDS_BATCH,
    SPW_CMD_CONTROL_SET_Z_ORDER,
    SPW_CMD_SCROLL_VIEW_CONFIGURE,
    SPW_CMD_SCROLL_VIEW_GET_PAGE_EXTENT,
    SPW_CMD_SCROLL_VIEW_SET_POSITION,
    SPW_CMD_PANED_CONFIGURE,
    SPW_CMD_PANED_GET_POSITION,
    SPW_CMD_NOTEBOOK_GET_CONTENT_RECT,
    SPW_CMD_CONTROL_DESTROY,
    SPW_CMD_CONTROL_MEASURE,
    SPW_CMD_BARRIER,
    SPW_CMD_BEEP,
    SPW_CMD_SHOW_MESSAGE,
    SPW_CMD_FILE_DIALOG,
    SPW_CMD_SET_WAIT_CURSOR,
    SPW_CMD_GET_WAIT_CURSOR,
    SPW_CMD_SHUTDOWN
} SpwCommandOpcode;

typedef struct {
    uint32_t opcode;
    HANDLE done;
    int32_t status;
    uint32_t win32_error;
    uint64_t object_id;
    uint64_t parent_id;
    uint64_t secondary_id;
    uint32_t kind;
    const char *text;
    uint32_t text_length;
    const char *text2;
    uint32_t text2_length;
    const char *text3;
    uint32_t text3_length;
    uint32_t item_count;
    const int32_t *indexes;
    uint32_t index_count;
    const int32_t *column_widths;
    const int32_t *parent_tokens;
    const int32_t *column_alignments;
    const int32_t *column_expandables;
    uint32_t column_count;
    uint32_t row_count;
    int32_t headers_visible;
    int32_t resizable;
    int32_t *output_indexes;
    uint32_t output_index_capacity;
    uint32_t output_index_count;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    int32_t value;
    int32_t aux_value;
    int32_t result32;
    const SpwBounds *bounds;
    uint32_t bounds_count;
    const uint64_t *object_ids;
    uint32_t object_id_count;
    char *output_text;
    uint32_t output_capacity;
    uint32_t output_length;
    const uint8_t *bytes;
    uint32_t byte_length;
} SpwCommand;

static HANDLE g_ui_thread = NULL;
static HANDLE g_ready_event = NULL;
static HANDLE g_event_available = NULL;
static HWND g_dispatcher = NULL;
static HINSTANCE g_instance = NULL;
static int32_t g_start_status = SPW_STATUS_START_FAILED;
static uint32_t g_start_error = 0u;
static _Atomic uint32_t g_state = 0u; /* 0 stopped, 1 starting, 2 running, 3 stopping */
static _Atomic uint64_t g_next_id = 1u;
static _Atomic uint32_t g_event_read = 0u;
static _Atomic uint32_t g_event_write = 0u;
static _Atomic uint32_t g_event_dropped = 0u;
static SpwEvent g_events[SPW_EVENT_CAPACITY];
static SpwObject g_objects[SPW_OBJECT_CAPACITY];
static HMENU g_popup_child_menus[SPW_MAX_MENU_ITEMS];
static uint32_t g_popup_child_counts[SPW_MAX_MENU_ITEMS];
static HBITMAP g_popup_bitmaps[SPW_MAX_MENU_ITEMS];
static HMENU g_window_menus[SPW_OBJECT_CAPACITY];
static HBITMAP g_window_menu_bitmaps[SPW_OBJECT_CAPACITY][SPW_MAX_MENU_ITEMS];
static uint32_t g_window_menu_item_counts[SPW_OBJECT_CAPACITY];

static const WCHAR g_window_class_name[] = {
    'P','h','a','r','o','S','p','e','c','W','i','n','3','2','R','u','n','t','i','m','e','W','i','n','d','o','w',0
};
static const WCHAR g_paned_class_name[] = {
    'P','h','a','r','o','S','p','e','c','W','i','n','3','2','P','a','n','e','d',0
};
static const WCHAR g_static_class_name[] = { 'S','T','A','T','I','C',0 };
static const WCHAR g_button_class_name[] = { 'B','U','T','T','O','N',0 };
static const WCHAR g_edit_class_name[] = { 'E','D','I','T',0 };
static const WCHAR g_rich_edit_class_name[] = { 'R','I','C','H','E','D','I','T','5','0','W',0 };
static const WCHAR g_msftedit_library_name[] = { 'M','s','f','t','e','d','i','t','.','d','l','l',0 };
static HMODULE g_msftedit_module = NULL;
static const WCHAR g_combo_box_class_name[] = { 'C','O','M','B','O','B','O','X',0 };
static const WCHAR g_trackbar_class_name[] = { 'm','s','c','t','l','s','_','t','r','a','c','k','b','a','r','3','2',0 };
static const WCHAR g_progress_class_name[] = { 'm','s','c','t','l','s','_','p','r','o','g','r','e','s','s','3','2',0 };
static const WCHAR g_tab_class_name[] = { 'S','y','s','T','a','b','C','o','n','t','r','o','l','3','2',0 };
static const WCHAR g_list_view_class_name[] = { 'S','y','s','L','i','s','t','V','i','e','w','3','2',0 };
static const WCHAR g_tree_view_class_name[] = { 'S','y','s','T','r','e','e','V','i','e','w','3','2',0 };
static const WCHAR g_tooltip_class_name[] = { 't','o','o','l','t','i','p','s','_','c','l','a','s','s','3','2',0 };
static const WCHAR g_link_class_name[] = { 'S','y','s','L','i','n','k',0 };
static WCHAR g_control_text_wide[SPW_MAX_CONTROL_TEXT_WCHARS];
static WCHAR g_text_area_wide[SPW_MAX_CONTROL_TEXT_WCHARS];
static WCHAR g_dialog_title_wide[SPW_MAX_TITLE_WCHARS];
static WCHAR g_dialog_path_wide[SPW_MAX_DIALOG_PATH_WCHARS];
static WCHAR g_dialog_initial_dir_wide[SPW_MAX_DIALOG_PATH_WCHARS];
static WCHAR g_dialog_filter_wide[SPW_MAX_DIALOG_FILTER_WCHARS];
static WCHAR g_dialog_display_wide[SPW_MAX_DIALOG_PATH_WCHARS];
static WCHAR g_message_wide[SPW_MAX_CONTROL_TEXT_WCHARS];

/* Event queue ----------------------------------------------------------- */
static int spw_event_queue_has_data(void) {
    uint32_t read_index = atomic_load_explicit(&g_event_read, memory_order_relaxed);
    uint32_t write_index = atomic_load_explicit(&g_event_write, memory_order_acquire);
    return read_index != write_index;
}

static void spw_push_event(uint32_t type, uint64_t object_id, int64_t argument1, int64_t argument2) {
    uint32_t write_index = atomic_load_explicit(&g_event_write, memory_order_relaxed);
    uint32_t next = (write_index + 1u) % SPW_EVENT_CAPACITY;
    uint32_t read_index = atomic_load_explicit(&g_event_read, memory_order_acquire);
    SpwEvent *event;

    if (next == read_index) {
        atomic_fetch_add_explicit(&g_event_dropped, 1u, memory_order_relaxed);
        return;
    }

    event = &g_events[write_index];
    event->type = type;
    event->flags = 0u;
    event->object_id = object_id;
    event->argument1 = argument1;
    event->argument2 = argument2;
    atomic_store_explicit(&g_event_write, next, memory_order_release);

    if (g_event_available != NULL)
        SetEvent(g_event_available);
}

/* UTF-16 / Unicode offset helpers -------------------------------------- */
static int spw_is_high_surrogate(WCHAR ch) {
    return ch >= 0xd800u && ch <= 0xdbffu;
}

static int spw_is_low_surrogate(WCHAR ch) {
    return ch >= 0xdc00u && ch <= 0xdfffu;
}

static int32_t spw_control_text_wide(HWND hwnd, uint32_t *out_units) {
    int length;
    int copied;
    if (hwnd == NULL || out_units == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    length = GetWindowTextLengthW(hwnd);
    if (length < 0)
        return SPW_STATUS_WIN32_ERROR;
    if ((uint32_t)length >= SPW_MAX_CONTROL_TEXT_WCHARS)
        return SPW_STATUS_CAPACITY;
    copied = GetWindowTextW(hwnd, g_control_text_wide, length + 1);
    if (copied < 0)
        return SPW_STATUS_WIN32_ERROR;
    g_control_text_wide[copied] = 0;
    *out_units = (uint32_t)copied;
    return SPW_STATUS_OK;
}


static void spw_text_area_apply_format_rect(SpwObject *object) {
    RECT rect;
    LONG_PTR style;
    if (object == NULL || object->hwnd == NULL || object->kind != SPW_CONTROL_TEXT_AREA)
        return;
    if (!GetClientRect(object->hwnd, &rect))
        return;
    style = GetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE);
    /* A multiline EDIT caches soft wrapping in its formatting rectangle.
       On Wine (and some Windows versions) changing ES_AUTOHSCROLL after
       creation is not enough to recompute existing lines.  Keep the normal
       client-width rectangle for wrapped text; use a deliberately wide
       formatting rectangle for unwrapped text so the standard EDIT can
       expose the line through its horizontal scrollbar without recreating
       the HWND (which would lose native edit state/undo history). */
    if ((style & (LONG_PTR)ES_AUTOHSCROLL_VALUE) != 0)
        rect.right = 32767;
    SendMessageW(object->hwnd, EM_SETRECT_VALUE, 0, (LPARAM)(uintptr_t)&rect);
}

static uint32_t spw_utf16_units_for_codepoint_offset(const WCHAR *text, uint32_t units, uint32_t offset) {
    uint32_t i = 0u;
    uint32_t points = 0u;
    while (i < units && points < offset) {
        if (spw_is_high_surrogate(text[i]) && i + 1u < units && spw_is_low_surrogate(text[i + 1u]))
            i += 2u;
        else
            i += 1u;
        points += 1u;
    }
    return i;
}

static uint32_t spw_codepoint_offset_for_utf16_units(const WCHAR *text, uint32_t units, uint32_t unit_offset) {
    uint32_t i = 0u;
    uint32_t points = 0u;
    if (unit_offset > units)
        unit_offset = units;
    while (i < unit_offset) {
        if (spw_is_high_surrogate(text[i]) && i + 1u < unit_offset && spw_is_low_surrogate(text[i + 1u]))
            i += 2u;
        else
            i += 1u;
        points += 1u;
    }
    return points;
}

static uint32_t spw_text_area_utf16_units_for_offset(const WCHAR *text, uint32_t units, uint32_t offset) {
    uint32_t i = 0u;
    uint32_t points = 0u;
    while (i < units && points < offset) {
        if (text[i] == (WCHAR)'\r' && i + 1u < units && text[i + 1u] == (WCHAR)'\n')
            i += 2u;
        else if (spw_is_high_surrogate(text[i]) && i + 1u < units && spw_is_low_surrogate(text[i + 1u]))
            i += 2u;
        else
            i += 1u;
        points += 1u;
    }
    return i;
}

static uint32_t spw_text_area_offset_for_utf16_units(const WCHAR *text, uint32_t units, uint32_t unit_offset) {
    uint32_t i = 0u;
    uint32_t points = 0u;
    if (unit_offset > units) unit_offset = units;
    while (i < unit_offset) {
        if (text[i] == (WCHAR)'\r' && i + 1u < unit_offset && text[i + 1u] == (WCHAR)'\n')
            i += 2u;
        else if (spw_is_high_surrogate(text[i]) && i + 1u < unit_offset && spw_is_low_surrogate(text[i + 1u]))
            i += 2u;
        else
            i += 1u;
        points += 1u;
    }
    return points;
}

static uint32_t spw_text_area_normalize_newlines(WCHAR *text, uint32_t units) {
    uint32_t read_index = 0u;
    uint32_t write_index = 0u;
    while (read_index < units) {
        if (text[read_index] == (WCHAR)'\r' && read_index + 1u < units && text[read_index + 1u] == (WCHAR)'\n') {
            text[write_index++] = (WCHAR)'\n';
            read_index += 2u;
        } else {
            text[write_index++] = text[read_index++];
        }
    }
    text[write_index] = 0;
    return write_index;
}

static int32_t spw_text_area_expand_newlines(const WCHAR *source, WCHAR *target, uint32_t capacity) {
    uint32_t read_index = 0u;
    uint32_t write_index = 0u;
    while (source[read_index] != 0) {
        WCHAR ch = source[read_index++];
        if (ch == (WCHAR)'\n' && (read_index < 2u || source[read_index - 2u] != (WCHAR)'\r')) {
            if (write_index + 2u >= capacity) return SPW_STATUS_CAPACITY;
            target[write_index++] = (WCHAR)'\r';
            target[write_index++] = (WCHAR)'\n';
        } else {
            if (write_index + 1u >= capacity) return SPW_STATUS_CAPACITY;
            target[write_index++] = ch;
        }
    }
    target[write_index] = 0;
    return SPW_STATUS_OK;
}

