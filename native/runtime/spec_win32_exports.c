/* Exported runtime ABI -------------------------------------------------- */
SPW_EXPORT uint32_t spw_abi_version(void) {
    return SPW_ABI_VERSION;
}

SPW_EXPORT uint32_t spw_pointer_bits(void) {
    return (uint32_t)(sizeof(void *) * 8u);
}

SPW_EXPORT int32_t spw_runtime_start(void) {
    DWORD wait_result;
    uint32_t expected = 0u;

    if (atomic_load_explicit(&g_state, memory_order_acquire) == 2u)
        return SPW_STATUS_OK;

    if (!atomic_compare_exchange_strong_explicit(
            &g_state, &expected, 1u, memory_order_acq_rel, memory_order_acquire)) {
        return (expected == 2u) ? SPW_STATUS_OK : SPW_STATUS_START_FAILED;
    }

    atomic_store_explicit(&g_event_read, 0u, memory_order_relaxed);
    atomic_store_explicit(&g_event_write, 0u, memory_order_relaxed);
    atomic_store_explicit(&g_event_dropped, 0u, memory_order_relaxed);
    g_start_status = SPW_STATUS_START_FAILED;
    g_start_error = 0u;

    g_event_available = CreateEventW(NULL, FALSE_VALUE, FALSE_VALUE, NULL);
    g_ready_event = CreateEventW(NULL, TRUE_VALUE, FALSE_VALUE, NULL);
    if (g_event_available == NULL || g_ready_event == NULL) {
        if (g_event_available != NULL) CloseHandle(g_event_available);
        if (g_ready_event != NULL) CloseHandle(g_ready_event);
        g_event_available = NULL;
        g_ready_event = NULL;
        atomic_store_explicit(&g_state, 0u, memory_order_release);
        return SPW_STATUS_WIN32_ERROR;
    }

    g_ui_thread = CreateThread(NULL, 0u, spw_ui_thread_main, NULL, 0u, NULL);
    if (g_ui_thread == NULL) {
        g_start_error = GetLastError();
        CloseHandle(g_event_available);
        CloseHandle(g_ready_event);
        g_event_available = NULL;
        g_ready_event = NULL;
        atomic_store_explicit(&g_state, 0u, memory_order_release);
        return SPW_STATUS_WIN32_ERROR;
    }

    wait_result = WaitForSingleObject(g_ready_event, INFINITE_VALUE);
    CloseHandle(g_ready_event);
    g_ready_event = NULL;

    if (wait_result != WAIT_OBJECT_0_VALUE || g_start_status != SPW_STATUS_OK) {
        WaitForSingleObject(g_ui_thread, INFINITE_VALUE);
        CloseHandle(g_ui_thread);
        g_ui_thread = NULL;
        CloseHandle(g_event_available);
        g_event_available = NULL;
        atomic_store_explicit(&g_state, 0u, memory_order_release);
        return g_start_status;
    }

    return SPW_STATUS_OK;
}

SPW_EXPORT int32_t spw_runtime_stop(void) {
    SpwCommand command;
    int32_t status;

    if (atomic_load_explicit(&g_state, memory_order_acquire) == 0u)
        return SPW_STATUS_OK;
    if (atomic_load_explicit(&g_state, memory_order_acquire) != 2u)
        return SPW_STATUS_NOT_RUNNING;

    atomic_store_explicit(&g_state, 3u, memory_order_release);
    /* spw_dispatch_command requires running state, so post shutdown directly. */
    command.opcode = SPW_CMD_SHUTDOWN;
    command.done = CreateEventW(NULL, FALSE_VALUE, FALSE_VALUE, NULL);
    command.status = SPW_STATUS_START_FAILED;
    command.win32_error = 0u;
    command.object_id = 0u;
    command.text = NULL;
    command.text_length = 0u;
    command.x = command.y = command.width = command.height = command.value = command.aux_value = command.result32 = 0;

    if (command.done == NULL) {
        atomic_store_explicit(&g_state, 2u, memory_order_release);
        return SPW_STATUS_WIN32_ERROR;
    }

    if (!PostMessageW(g_dispatcher, SPW_WM_COMMAND, 0u, (LPARAM)(uintptr_t)&command)) {
        CloseHandle(command.done);
        atomic_store_explicit(&g_state, 2u, memory_order_release);
        return SPW_STATUS_WIN32_ERROR;
    }

    WaitForSingleObject(command.done, INFINITE_VALUE);
    status = command.status;
    CloseHandle(command.done);

    if (g_ui_thread != NULL) {
        WaitForSingleObject(g_ui_thread, INFINITE_VALUE);
        CloseHandle(g_ui_thread);
        g_ui_thread = NULL;
    }

    if (g_event_available != NULL) {
        SetEvent(g_event_available);
        CloseHandle(g_event_available);
        g_event_available = NULL;
    }

    g_dispatcher = NULL;
    atomic_store_explicit(&g_state, 0u, memory_order_release);
    return status;
}

SPW_EXPORT int32_t spw_runtime_is_running(void) {
    return atomic_load_explicit(&g_state, memory_order_acquire) == 2u ? 1 : 0;
}

SPW_EXPORT uint32_t spw_runtime_start_error(void) {
    return g_start_error;
}

SPW_EXPORT uint32_t spw_runtime_object_count(void) {
    uint32_t i;
    uint32_t count = 0u;
    for (i = 0u; i < SPW_OBJECT_CAPACITY; ++i)
        if (g_objects[i].alive) ++count;
    return count;
}

SPW_EXPORT uint32_t spw_runtime_dropped_event_count(void) {
    return atomic_load_explicit(&g_event_dropped, memory_order_relaxed);
}

SPW_EXPORT int32_t spw_runtime_wake(void) {
    if (g_event_available == NULL)
        return SPW_STATUS_NOT_RUNNING;
    return SetEvent(g_event_available) ? SPW_STATUS_OK : SPW_STATUS_WIN32_ERROR;
}

SPW_EXPORT int32_t spw_wait_for_activity(uint32_t timeout_ms) {
    DWORD result;

    if (spw_event_queue_has_data())
        return 1;
    if (g_event_available == NULL)
        return SPW_STATUS_NOT_RUNNING;

    result = WaitForSingleObject(g_event_available, timeout_ms);
    if (result == WAIT_OBJECT_0_VALUE)
        return 1;
    if (result == WAIT_TIMEOUT_VALUE)
        return 0;
    return SPW_STATUS_WIN32_ERROR;
}

SPW_EXPORT int32_t spw_event_pop(SpwEvent *out_event) {
    uint32_t read_index;
    uint32_t write_index;

    if (out_event == NULL)
        return SPW_STATUS_BAD_ARGUMENT;

    read_index = atomic_load_explicit(&g_event_read, memory_order_relaxed);
    write_index = atomic_load_explicit(&g_event_write, memory_order_acquire);
    if (read_index == write_index)
        return 0;

    *out_event = g_events[read_index];
    atomic_store_explicit(&g_event_read,
                          (read_index + 1u) % SPW_EVENT_CAPACITY,
                          memory_order_release);
    return 1;
}

SPW_EXPORT int32_t spw_window_create(const char *utf8_title, uint32_t title_length,
                                     int32_t x, int32_t y, int32_t width, int32_t height,
                                     uint64_t *out_object_id) {
    SpwCommand command;
    int32_t status;

    if (out_object_id == NULL || width <= 0 || height <= 0)
        return SPW_STATUS_BAD_ARGUMENT;

    command.opcode = SPW_CMD_CREATE_WINDOW;
    command.object_id = 0u;
    command.text = utf8_title;
    command.text_length = title_length;
    command.x = x;
    command.y = y;
    command.width = width;
    command.height = height;
    command.value = 0;

    status = spw_dispatch_command(&command);
    if (status == SPW_STATUS_OK)
        *out_object_id = command.object_id;
    return status;
}

SPW_EXPORT int32_t spw_popup_window_create(uint64_t owner_object_id,
                                           int32_t x, int32_t y,
                                           int32_t width, int32_t height,
                                           int32_t autohide,
                                           uint64_t *out_object_id) {
    SpwCommand command = {0};
    int32_t status;
    if (out_object_id == NULL || width <= 0 || height <= 0)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CREATE_POPUP_WINDOW;
    command.parent_id = owner_object_id;
    command.x = x;
    command.y = y;
    command.width = width;
    command.height = height;
    command.value = autohide ? 1 : 0;
    status = spw_dispatch_command(&command);
    if (status == SPW_STATUS_OK)
        *out_object_id = command.object_id;
    return status;
}

SPW_EXPORT int32_t spw_window_show(uint64_t object_id, int32_t command_value) {
    SpwCommand command;
    command.opcode = SPW_CMD_SHOW_WINDOW;
    command.object_id = object_id;
    command.value = command_value;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_window_set_title(uint64_t object_id,
                                        const char *utf8_title, uint32_t title_length) {
    SpwCommand command;
    command.opcode = SPW_CMD_SET_TITLE;
    command.object_id = object_id;
    command.text = utf8_title;
    command.text_length = title_length;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_window_set_menu(uint64_t object_id, const void *menu_blob,
                                       uint32_t blob_length, uint32_t item_count) {
    SpwCommand command = {0};
    if (item_count > SPW_MAX_MENU_ITEMS || (item_count > 0u && menu_blob == NULL))
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_SET_WINDOW_MENU;
    command.object_id = object_id;
    command.text = (const char *)menu_blob;
    command.text_length = blob_length;
    command.item_count = item_count;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_window_set_bounds(uint64_t object_id,
                                         int32_t x, int32_t y, int32_t width, int32_t height) {
    SpwCommand command;
    if (width <= 0 || height <= 0)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_SET_BOUNDS;
    command.object_id = object_id;
    command.x = x;
    command.y = y;
    command.width = width;
    command.height = height;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_window_get_client_size(uint64_t object_id,
                                                  int32_t *out_width, int32_t *out_height) {
    SpwCommand command;
    int32_t status;
    if (out_width == NULL || out_height == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_GET_CLIENT_SIZE;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status == SPW_STATUS_OK) {
        *out_width = command.width;
        *out_height = command.height;
    }
    return status;
}

SPW_EXPORT int32_t spw_window_destroy(uint64_t object_id) {
    SpwCommand command;
    command.opcode = SPW_CMD_DESTROY_WINDOW;
    command.object_id = object_id;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_window_is_visible(uint64_t object_id) {
    SpwCommand command;
    int32_t status;
    command.opcode = SPW_CMD_IS_VISIBLE;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK)
        return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_window_activate(uint64_t object_id) {
    SpwCommand command;
    int32_t status;
    command.opcode = SPW_CMD_ACTIVATE_WINDOW;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK)
        return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_window_get_bounds(uint64_t object_id,
                                              int32_t *out_x, int32_t *out_y,
                                              int32_t *out_width, int32_t *out_height) {
    SpwCommand command = {0};
    int32_t status;
    if (out_x == NULL || out_y == NULL || out_width == NULL || out_height == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_GET_WINDOW_BOUNDS;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status == SPW_STATUS_OK) {
        *out_x = command.x;
        *out_y = command.y;
        *out_width = command.width;
        *out_height = command.height;
    }
    return status;
}

SPW_EXPORT int32_t spw_window_is_minimized(uint64_t object_id) {
    SpwCommand command = {0};
    int32_t status;
    command.opcode = SPW_CMD_IS_MINIMIZED;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK) return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_window_is_maximized(uint64_t object_id) {
    SpwCommand command = {0};
    int32_t status;
    command.opcode = SPW_CMD_IS_MAXIMIZED;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK) return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_window_is_foreground(uint64_t object_id) {
    SpwCommand command = {0};
    int32_t status;
    command.opcode = SPW_CMD_IS_FOREGROUND;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK) return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_window_center(uint64_t object_id) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_CENTER_WINDOW;
    command.object_id = object_id;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_window_center_relative(uint64_t object_id, uint64_t relative_object_id) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_CENTER_WINDOW_RELATIVE;
    command.object_id = object_id;
    command.parent_id = relative_object_id;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_window_set_owner(uint64_t object_id, uint64_t owner_object_id) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_SET_WINDOW_OWNER;
    command.object_id = object_id;
    command.parent_id = owner_object_id;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_window_set_enabled(uint64_t object_id, int32_t enabled) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_SET_WINDOW_ENABLED;
    command.object_id = object_id;
    command.value = enabled ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_window_is_enabled(uint64_t object_id) {
    SpwCommand command = {0};
    int32_t status;
    command.opcode = SPW_CMD_IS_WINDOW_ENABLED;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK) return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_control_create(uint32_t kind, uint64_t parent_object_id,
                                      const char *utf8_text, uint32_t text_length,
                                      int32_t x, int32_t y, int32_t width, int32_t height,
                                      uint64_t *out_object_id) {
    SpwCommand command;
    int32_t status;
    if (out_object_id == NULL || width <= 0 || height <= 0)
        return SPW_STATUS_BAD_ARGUMENT;
    if (!spw_control_kind_is_valid(kind))
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CREATE_CONTROL;
    command.kind = kind;
    command.parent_id = parent_object_id;
    command.object_id = 0u;
    command.text = utf8_text;
    command.text_length = text_length;
    command.x = x;
    command.y = y;
    command.width = width;
    command.height = height;
    status = spw_dispatch_command(&command);
    if (status == SPW_STATUS_OK)
        *out_object_id = command.object_id;
    return status;
}

SPW_EXPORT int32_t spw_control_set_text(uint64_t object_id,
                                        const char *utf8_text, uint32_t text_length) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_TEXT;
    command.object_id = object_id;
    command.text = utf8_text;
    command.text_length = text_length;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_tooltip(uint64_t object_id,
                                           const char *utf8_text, uint32_t text_length) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_CONTROL_SET_TOOLTIP;
    command.object_id = object_id;
    command.text = utf8_text;
    command.text_length = text_length;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_context_menu_enabled(uint64_t object_id, int32_t enabled) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_CONTROL_SET_CONTEXT_MENU_ENABLED;
    command.object_id = object_id;
    command.value = enabled ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_show_context_menu(uint64_t object_id,
                                                  const void *items_blob,
                                                  uint32_t blob_length,
                                                  const int32_t *flags,
                                                  uint32_t item_count,
                                                  int32_t screen_x,
                                                  int32_t screen_y) {
    SpwCommand command = {0};
    int32_t status;
    command.opcode = SPW_CMD_CONTROL_SHOW_CONTEXT_MENU;
    command.object_id = object_id;
    command.text = (const char *)items_blob;
    command.text_length = blob_length;
    command.indexes = flags;
    command.item_count = item_count;
    command.x = screen_x;
    command.y = screen_y;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK)
        return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_control_show_popup_menu(uint64_t object_id,
                                                const void *menu_blob,
                                                uint32_t blob_length,
                                                uint32_t item_count,
                                                int32_t screen_x,
                                                int32_t screen_y) {
    SpwCommand command = {0};
    int32_t status;
    command.opcode = SPW_CMD_CONTROL_SHOW_POPUP_MENU;
    command.object_id = object_id;
    command.text = (const char *)menu_blob;
    command.text_length = blob_length;
    command.item_count = item_count;
    command.x = screen_x;
    command.y = screen_y;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK)
        return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_control_get_screen_bounds(uint64_t object_id,
                                                  int32_t *out_x, int32_t *out_y,
                                                  int32_t *out_width, int32_t *out_height) {
    SpwObject *object;
    RECT rect;
    if (out_x == NULL || out_y == NULL || out_width == NULL || out_height == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    object = spw_find_object_by_id(object_id);
    if (object == NULL || object->hwnd == NULL)
        return SPW_STATUS_NOT_FOUND;
    if (!GetWindowRect(object->hwnd, &rect))
        return SPW_STATUS_WIN32_ERROR;
    *out_x = rect.left;
    *out_y = rect.top;
    *out_width = rect.right - rect.left;
    *out_height = rect.bottom - rect.top;
    return SPW_STATUS_OK;
}

SPW_EXPORT int32_t spw_object_get_monitor_work_area(uint64_t object_id,
                                                        int32_t *out_x, int32_t *out_y,
                                                        int32_t *out_width, int32_t *out_height) {
    SpwObject *object;
    HMONITOR monitor;
    MONITORINFO info;
    if (out_x == NULL || out_y == NULL || out_width == NULL || out_height == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    object = spw_find_object_by_id(object_id);
    if (object == NULL || object->hwnd == NULL)
        return SPW_STATUS_NOT_FOUND;
    monitor = MonitorFromWindow(object->hwnd, MONITOR_DEFAULTTONEAREST_VALUE);
    if (monitor == NULL)
        return SPW_STATUS_WIN32_ERROR;
    info.cbSize = (DWORD)sizeof(MONITORINFO);
    if (!GetMonitorInfoW(monitor, &info))
        return SPW_STATUS_WIN32_ERROR;
    *out_x = info.rcWork.left;
    *out_y = info.rcWork.top;
    *out_width = info.rcWork.right - info.rcWork.left;
    *out_height = info.rcWork.bottom - info.rcWork.top;
    return SPW_STATUS_OK;
}

SPW_EXPORT int32_t spw_get_cursor_position(int32_t *out_x, int32_t *out_y) {
    POINT point;
    if (out_x == NULL || out_y == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    if (!GetCursorPos(&point))
        return SPW_STATUS_WIN32_ERROR;
    *out_x = point.x;
    *out_y = point.y;
    return SPW_STATUS_OK;
}

SPW_EXPORT int32_t spw_control_get_text(uint64_t object_id,
                                        char *out_utf8, uint32_t capacity, uint32_t *out_length) {
    SpwCommand command;
    int32_t status;
    if (out_length == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_GET_TEXT;
    command.object_id = object_id;
    command.output_text = out_utf8;
    command.output_capacity = capacity;
    command.output_length = 0u;
    status = spw_dispatch_command(&command);
    *out_length = command.output_length;
    return status;
}

SPW_EXPORT int32_t spw_control_set_checked(uint64_t object_id, int32_t checked) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_CHECKED;
    command.object_id = object_id;
    command.value = checked ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_get_checked(uint64_t object_id) {
    SpwCommand command;
    int32_t status;
    command.opcode = SPW_CMD_CONTROL_GET_CHECKED;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK)
        return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_control_set_editable(uint64_t object_id, int32_t editable) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_EDITABLE;
    command.object_id = object_id;
    command.value = editable ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_max_length(uint64_t object_id, uint32_t max_length) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_MAX_LENGTH;
    command.object_id = object_id;
    command.value = (int32_t)max_length;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_password(uint64_t object_id, int32_t password) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_PASSWORD;
    command.object_id = object_id;
    command.value = password ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_placeholder(uint64_t object_id,
                                               const char *utf8_text, uint32_t text_length) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_PLACEHOLDER;
    command.object_id = object_id;
    command.text = utf8_text;
    command.text_length = text_length;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_image_bgra(uint64_t object_id,
                                              const void *pixels, uint32_t byte_length,
                                              int32_t width, int32_t height) {
    SpwCommand command = {0};
    if ((pixels == NULL || byte_length == 0u || width == 0 || height == 0) &&
        !(pixels == NULL && byte_length == 0u && width == 0 && height == 0))
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_SET_IMAGE_BGRA;
    command.object_id = object_id;
    command.bytes = (const uint8_t *)pixels;
    command.byte_length = byte_length;
    command.width = width;
    command.height = height;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_image_auto_scale(uint64_t object_id, int32_t auto_scale) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_CONTROL_SET_IMAGE_AUTO_SCALE;
    command.object_id = object_id;
    command.value = auto_scale ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_items(uint64_t object_id,
                                         const void *items_blob, uint32_t blob_length,
                                         uint32_t item_count) {
    SpwCommand command;
    if (item_count > 0u && items_blob == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_SET_ITEMS;
    command.object_id = object_id;
    command.text = (const char *)items_blob;
    command.text_length = blob_length;
    command.item_count = item_count;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_list_set_rows(uint64_t object_id,
                                      const void *rows_blob, uint32_t blob_length,
                                      uint32_t row_count) {
    SpwCommand command = {0};
    if (row_count > 0u && rows_blob == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_SET_LIST_ROWS;
    command.object_id = object_id;
    command.bytes = (const uint8_t *)rows_blob;
    command.byte_length = blob_length;
    command.item_count = row_count;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_selected_index(uint64_t object_id, int32_t selected_index) {
    SpwCommand command;
    if (selected_index < 0)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_SET_SELECTED_INDEX;
    command.object_id = object_id;
    command.value = selected_index;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_get_selected_index(uint64_t object_id) {
    SpwCommand command;
    int32_t status;
    command.opcode = SPW_CMD_CONTROL_GET_SELECTED_INDEX;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK)
        return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_control_set_value(uint64_t object_id, int32_t normalized_value) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_VALUE;
    command.object_id = object_id;
    command.value = spw_clamp_normalized_value(normalized_value);
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_get_value(uint64_t object_id) {
    SpwCommand command;
    int32_t status;
    command.opcode = SPW_CMD_CONTROL_GET_VALUE;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK)
        return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_control_set_indeterminate(uint64_t object_id, int32_t indeterminate) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_INDETERMINATE;
    command.object_id = object_id;
    command.value = indeterminate ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_enabled(uint64_t object_id, int32_t enabled) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_ENABLED;
    command.object_id = object_id;
    command.value = enabled ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_show(uint64_t object_id, int32_t visible) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SHOW;
    command.object_id = object_id;
    command.value = visible ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_focus(uint64_t object_id) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_FOCUS;
    command.object_id = object_id;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_has_focus(uint64_t object_id) {
    SpwCommand command;
    int32_t status;
    command.opcode = SPW_CMD_CONTROL_HAS_FOCUS;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK)
        return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_control_set_selection(uint64_t object_id,
                                              int32_t start_offset,
                                              int32_t end_offset) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_SELECTION;
    command.object_id = object_id;
    command.x = start_offset;
    command.y = end_offset;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_get_selection(uint64_t object_id,
                                              int32_t *out_start_offset,
                                              int32_t *out_end_offset) {
    SpwCommand command;
    int32_t status;
    if (out_start_offset == NULL || out_end_offset == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_GET_SELECTION;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    if (status == SPW_STATUS_OK) {
        *out_start_offset = command.x;
        *out_end_offset = command.y;
    }
    return status;
}

SPW_EXPORT int32_t spw_text_configure(uint64_t object_id, int32_t wrap_word,
                                      int32_t scroll_bars, int32_t undo_enabled) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_TEXT_CONFIGURE;
    command.object_id = object_id;
    command.value = wrap_word ? 1 : 0;
    command.aux_value = scroll_bars ? 1 : 0;
    command.x = undo_enabled ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_text_replace_selection(uint64_t object_id,
                                               const char *utf8_text, uint32_t text_length) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_TEXT_REPLACE_SELECTION;
    command.object_id = object_id;
    command.text = utf8_text;
    command.text_length = text_length;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_text_command(uint64_t object_id, int32_t command_code) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_TEXT_COMMAND;
    command.object_id = object_id;
    command.value = command_code;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_text_scroll_to_line(uint64_t object_id, int32_t zero_based_line) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_TEXT_SCROLL_TO_LINE;
    command.object_id = object_id;
    command.value = zero_based_line;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_text_first_visible_line(uint64_t object_id) {
    SpwObject *object = spw_find_object_by_id(object_id);
    if (object == NULL || object->hwnd == NULL)
        return SPW_STATUS_NOT_FOUND;
    if (object->kind != SPW_CONTROL_TEXT_AREA && object->kind != SPW_CONTROL_CODE)
        return SPW_STATUS_BAD_ARGUMENT;
    return (int32_t)SendMessageW(object->hwnd, EM_GETFIRSTVISIBLELINE_VALUE, 0, 0);
}

SPW_EXPORT int32_t spw_code_set_line_numbers(uint64_t object_id, int32_t enabled) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_CODE_SET_LINE_NUMBERS;
    command.object_id = object_id;
    command.value = enabled ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_code_set_styles(uint64_t object_id,
                                       const int32_t *spans, uint32_t span_count,
                                       uint32_t foreground_rgb, uint32_t background_rgb) {
    SpwCommand command = {0};
    if (span_count > 0u && spans == NULL) return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CODE_SET_STYLES;
    command.object_id = object_id;
    command.indexes = spans;
    command.item_count = span_count;
    command.x = (int32_t)(foreground_rgb & 0xffffffu);
    command.y = (int32_t)(background_rgb & 0xffffffu);
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_code_set_line_decorations(uint64_t object_id,
                                                  const int32_t *entries,
                                                  uint32_t entry_count) {
    SpwCommand command = {0};
    if (entry_count > 0u && entries == NULL) return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CODE_SET_LINE_DECORATIONS;
    command.object_id = object_id;
    command.indexes = entries;
    command.item_count = entry_count;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_selected_indexes(uint64_t object_id,
                                                     const int32_t *indexes, uint32_t count) {
    SpwCommand command;
    if (count > 0u && indexes == NULL) return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_SET_SELECTED_INDEXES;
    command.object_id = object_id;
    command.indexes = indexes;
    command.index_count = count;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_get_selected_indexes(uint64_t object_id,
                                                     int32_t *out_indexes, uint32_t capacity,
                                                     uint32_t *out_count) {
    SpwCommand command;
    int32_t status;
    if (out_count == NULL) return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_GET_SELECTED_INDEXES;
    command.object_id = object_id;
    command.output_indexes = out_indexes;
    command.output_index_capacity = capacity;
    command.output_index_count = 0u;
    status = spw_dispatch_command(&command);
    if (status == SPW_STATUS_OK) *out_count = command.output_index_count;
    return status;
}

SPW_EXPORT int32_t spw_control_set_multiple_selection(uint64_t object_id, int32_t multiple) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_MULTIPLE_SELECTION;
    command.object_id = object_id;
    command.value = multiple ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_header(uint64_t object_id,
                                           const char *utf8_text, uint32_t text_length,
                                           int32_t visible) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_SET_HEADER;
    command.object_id = object_id;
    command.text = utf8_text;
    command.text_length = text_length;
    command.value = visible ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_table_columns(uint64_t object_id,
                                                  const void *titles_blob, uint32_t blob_length,
                                                  const int32_t *widths, const int32_t *alignments,
                                                  const int32_t *expandables, uint32_t column_count,
                                                  int32_t headers_visible, int32_t resizable) {
    SpwCommand command;
    if (column_count == 0u || column_count > SPW_MAX_TABLE_COLUMNS ||
        titles_blob == NULL || widths == NULL || alignments == NULL || expandables == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_SET_TABLE_COLUMNS;
    command.object_id = object_id;
    command.text = (const char *)titles_blob;
    command.text_length = blob_length;
    command.column_widths = widths;
    command.column_alignments = alignments;
    command.column_expandables = expandables;
    command.column_count = column_count;
    command.headers_visible = headers_visible ? 1 : 0;
    command.resizable = resizable ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_table_cells(uint64_t object_id,
                                                const void *cells_blob, uint32_t blob_length,
                                                uint32_t row_count, uint32_t column_count) {
    SpwCommand command;
    if (column_count == 0u || (row_count > 0u && cells_blob == NULL)) return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_SET_TABLE_CELLS;
    command.object_id = object_id;
    command.text = (const char *)cells_blob;
    command.text_length = blob_length;
    command.row_count = row_count;
    command.column_count = column_count;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_table_set_cells_with_images(uint64_t object_id,
                                                    const void *cells_blob, uint32_t blob_length,
                                                    uint32_t row_count, uint32_t column_count) {
    SpwCommand command = {0};
    if (column_count == 0u || (row_count > 0u && cells_blob == NULL)) return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_SET_TABLE_CELLS_WITH_IMAGES;
    command.object_id = object_id;
    command.bytes = (const uint8_t *)cells_blob;
    command.byte_length = blob_length;
    command.row_count = row_count;
    command.column_count = column_count;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_ensure_visible(uint64_t object_id, int32_t index) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_ENSURE_VISIBLE;
    command.object_id = object_id;
    command.value = index;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_bounds_batch(const SpwBounds *bounds, uint32_t count) {
    SpwCommand command;
    if (bounds == NULL || count == 0u)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_SET_BOUNDS_BATCH;
    command.bounds = bounds;
    command.bounds_count = count;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_set_z_order(const uint64_t *object_ids, uint32_t count) {
    SpwCommand command = {0};
    if (object_ids == NULL || count == 0u)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_SET_Z_ORDER;
    command.object_ids = object_ids;
    command.object_id_count = count;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_scroll_view_configure(uint64_t viewport_object_id,
                                               uint64_t content_object_id,
                                               int32_t content_width,
                                               int32_t content_height,
                                               int32_t horizontal_enabled,
                                               int32_t vertical_enabled) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_SCROLL_VIEW_CONFIGURE;
    command.object_id = viewport_object_id;
    command.parent_id = content_object_id;
    command.width = content_width;
    command.height = content_height;
    command.x = horizontal_enabled ? 1 : 0;
    command.y = vertical_enabled ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_scroll_view_configure_policies(uint64_t viewport_object_id,
                                                        uint64_t content_object_id,
                                                        int32_t content_width,
                                                        int32_t content_height,
                                                        int32_t horizontal_policy,
                                                        int32_t vertical_policy) {
    SpwCommand command = {0};
    if (horizontal_policy < (int32_t)SPW_SCROLL_POLICY_DISABLED ||
        horizontal_policy > (int32_t)SPW_SCROLL_POLICY_ALWAYS ||
        vertical_policy < (int32_t)SPW_SCROLL_POLICY_DISABLED ||
        vertical_policy > (int32_t)SPW_SCROLL_POLICY_ALWAYS)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_SCROLL_VIEW_CONFIGURE;
    command.object_id = viewport_object_id;
    command.parent_id = content_object_id;
    command.width = content_width;
    command.height = content_height;
    command.x = horizontal_policy;
    command.y = vertical_policy;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_scroll_view_get_page_extent(uint64_t viewport_object_id,
                                                    int32_t *out_width, int32_t *out_height) {
    SpwCommand command = {0};
    int32_t status;
    if (out_width == NULL || out_height == NULL) return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_SCROLL_VIEW_GET_PAGE_EXTENT;
    command.object_id = viewport_object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK) return status;
    *out_width = command.width;
    *out_height = command.height;
    return SPW_STATUS_OK;
}

SPW_EXPORT int32_t spw_scroll_view_set_position(uint64_t viewport_object_id,
                                                  int32_t x, int32_t y) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_SCROLL_VIEW_SET_POSITION;
    command.object_id = viewport_object_id;
    command.x = x;
    command.y = y;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_paned_configure(uint64_t paned_object_id,
                                        uint64_t first_content_object_id,
                                        uint64_t second_content_object_id,
                                        int32_t vertical,
                                        int32_t initial_position,
                                        int32_t first_present,
                                        int32_t second_present,
                                        int32_t first_resize,
                                        int32_t second_resize,
                                        int32_t first_min,
                                        int32_t second_min) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_PANED_CONFIGURE;
    command.object_id = paned_object_id;
    command.parent_id = first_content_object_id;
    command.secondary_id = second_content_object_id;
    command.kind = vertical ? 1u : 0u;
    command.value = initial_position;
    command.x = first_present ? 1 : 0;
    command.y = second_present ? 1 : 0;
    command.resizable = first_resize ? 1 : 0;
    command.headers_visible = second_resize ? 1 : 0;
    command.width = first_min;
    command.height = second_min;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_paned_get_position(uint64_t paned_object_id) {
    SpwCommand command = {0};
    int32_t status;
    command.opcode = SPW_CMD_PANED_GET_POSITION;
    command.object_id = paned_object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK) return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_notebook_get_content_rect(uint64_t notebook_object_id,
                                                  int32_t *out_x, int32_t *out_y,
                                                  int32_t *out_width, int32_t *out_height) {
    SpwCommand command = {0};
    int32_t status;
    if (out_x == NULL || out_y == NULL || out_width == NULL || out_height == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_NOTEBOOK_GET_CONTENT_RECT;
    command.object_id = notebook_object_id;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK) return status;
    *out_x = command.x;
    *out_y = command.y;
    *out_width = command.width;
    *out_height = command.height;
    return SPW_STATUS_OK;
}

SPW_EXPORT int32_t spw_control_destroy(uint64_t object_id) {
    SpwCommand command;
    command.opcode = SPW_CMD_CONTROL_DESTROY;
    command.object_id = object_id;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_control_measure(uint32_t kind, uint64_t parent_object_id,
                                       const char *utf8_text, uint32_t text_length,
                                       int32_t *out_width, int32_t *out_height) {
    SpwCommand command;
    int32_t status;
    if (out_width == NULL || out_height == NULL) return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_CONTROL_MEASURE;
    command.kind = kind;
    command.parent_id = parent_object_id;
    command.text = utf8_text;
    command.text_length = text_length;
    status = spw_dispatch_command(&command);
    if (status == SPW_STATUS_OK) {
        *out_width = command.width;
        *out_height = command.height;
    }
    return status;
}

SPW_EXPORT int32_t spw_runtime_barrier(uint64_t token) {
    SpwCommand command;
    command.opcode = SPW_CMD_BARRIER;
    command.object_id = token;
    return spw_dispatch_command(&command);
}


SPW_EXPORT int32_t spw_control_set_tree_nodes(uint64_t object_id,const void *labels_blob,uint32_t blob_length,const int32_t *parent_tokens,uint32_t count){ SpwCommand c={0}; if(count>0u&&(labels_blob==NULL||parent_tokens==NULL))return SPW_STATUS_BAD_ARGUMENT; c.opcode=SPW_CMD_CONTROL_SET_TREE_NODES;c.object_id=object_id;c.text=(const char*)labels_blob;c.text_length=blob_length;c.parent_tokens=parent_tokens;c.item_count=count;return spw_dispatch_command(&c);}
SPW_EXPORT int32_t spw_tree_set_nodes_with_images(uint64_t object_id,const void *rows_blob,uint32_t blob_length,const int32_t *parent_tokens,uint32_t count){ SpwCommand c={0}; if(count>0u&&(rows_blob==NULL||parent_tokens==NULL))return SPW_STATUS_BAD_ARGUMENT; c.opcode=SPW_CMD_CONTROL_SET_TREE_NODES_WITH_IMAGES;c.object_id=object_id;c.bytes=(const uint8_t*)rows_blob;c.byte_length=blob_length;c.parent_tokens=parent_tokens;c.item_count=count;return spw_dispatch_command(&c);}
SPW_EXPORT int32_t spw_control_set_tree_selected_token(uint64_t object_id,int32_t token){SpwCommand c={0};c.opcode=SPW_CMD_CONTROL_SET_TREE_SELECTED_TOKEN;c.object_id=object_id;c.value=token;return spw_dispatch_command(&c);}
SPW_EXPORT int32_t spw_control_get_tree_selected_token(uint64_t object_id){SpwCommand c={0};int32_t st;c.opcode=SPW_CMD_CONTROL_GET_TREE_SELECTED_TOKEN;c.object_id=object_id;st=spw_dispatch_command(&c);return st<0?st:c.result32;}
SPW_EXPORT int32_t spw_control_tree_set_expanded(uint64_t object_id,int32_t token,int32_t expanded){SpwCommand c={0};c.opcode=SPW_CMD_CONTROL_TREE_SET_EXPANDED;c.object_id=object_id;c.value=token;c.aux_value=expanded;return spw_dispatch_command(&c);}
SPW_EXPORT int32_t spw_control_tree_is_expanded(uint64_t object_id,int32_t token){SpwCommand c={0};int32_t st;c.opcode=SPW_CMD_CONTROL_TREE_IS_EXPANDED;c.object_id=object_id;c.value=token;st=spw_dispatch_command(&c);return st<0?st:c.result32;}
SPW_EXPORT int32_t spw_control_tree_ensure_visible(uint64_t object_id,int32_t token){SpwCommand c={0};c.opcode=SPW_CMD_CONTROL_TREE_ENSURE_VISIBLE;c.object_id=object_id;c.value=token;return spw_dispatch_command(&c);}

SPW_EXPORT int32_t spw_show_message(uint64_t owner_object_id,
                                    int32_t message_kind,
                                    const char *utf8_message, uint32_t message_length,
                                    const char *utf8_title, uint32_t title_length) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_SHOW_MESSAGE;
    command.object_id = owner_object_id;
    command.value = message_kind;
    command.text = utf8_message;
    command.text_length = message_length;
    command.text2 = utf8_title;
    command.text2_length = title_length;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_file_dialog(uint64_t owner_object_id,
                                   int32_t mode,
                                   const char *utf8_title, uint32_t title_length,
                                   const char *utf8_initial_dir, uint32_t initial_dir_length,
                                   const char *utf8_initial_name, uint32_t initial_name_length,
                                   const void *utf8_filter, uint32_t filter_length,
                                   char *out_utf8, uint32_t output_capacity,
                                   uint32_t *out_length) {
    SpwCommand command = {0};
    int32_t status;
    if (out_length == NULL || output_capacity == 0u || out_utf8 == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    command.opcode = SPW_CMD_FILE_DIALOG;
    command.object_id = owner_object_id;
    command.value = mode;
    command.text = utf8_title;
    command.text_length = title_length;
    command.text2 = utf8_initial_dir;
    command.text2_length = initial_dir_length;
    command.text3 = utf8_initial_name;
    command.text3_length = initial_name_length;
    command.bytes = (const uint8_t *)utf8_filter;
    command.byte_length = filter_length;
    command.output_text = out_utf8;
    command.output_capacity = output_capacity;
    command.output_length = 0u;
    status = spw_dispatch_command(&command);
    *out_length = command.output_length;
    return status;
}

SPW_EXPORT int32_t spw_window_set_wait_cursor(uint64_t object_id, int32_t enabled) {
    SpwCommand command = {0};
    command.opcode = SPW_CMD_SET_WAIT_CURSOR;
    command.object_id = object_id;
    command.value = enabled ? 1 : 0;
    return spw_dispatch_command(&command);
}

SPW_EXPORT int32_t spw_window_wait_cursor_active(uint64_t object_id) {
    SpwCommand command = {0};
    int32_t status;
    command.opcode = SPW_CMD_GET_WAIT_CURSOR;
    command.object_id = object_id;
    status = spw_dispatch_command(&command);
    return status < 0 ? status : command.result32;
}

SPW_EXPORT int32_t spw_beep(uint32_t kind) {
    SpwCommand command;
    int32_t status;
    command.opcode = SPW_CMD_BEEP;
    command.value = (int32_t)kind;
    status = spw_dispatch_command(&command);
    if (status != SPW_STATUS_OK)
        return status;
    return command.result32;
}

SPW_EXPORT int32_t spw_show_command_hide(void) { return SW_HIDE_VALUE; }
SPW_EXPORT int32_t spw_show_command_show(void) { return SW_SHOW_VALUE; }
SPW_EXPORT int32_t spw_show_command_minimize(void) { return SW_MINIMIZE_VALUE; }
SPW_EXPORT int32_t spw_show_command_maximize(void) { return SW_MAXIMIZE_VALUE; }
SPW_EXPORT int32_t spw_show_command_restore(void) { return SW_RESTORE_VALUE; }
SPW_EXPORT int32_t spw_cw_use_default(void) { return CW_USEDEFAULT_VALUE; }
