/* Text conversion ------------------------------------------------------- */
static int spw_utf8_to_wide(const char *text, uint32_t text_length, WCHAR *buffer, int capacity) {
    int count;
    if (buffer == NULL || capacity <= 0)
        return SPW_STATUS_BAD_ARGUMENT;

    if (text == NULL || text_length == 0u) {
        buffer[0] = 0;
        return SPW_STATUS_OK;
    }

    if (text_length > 0x7fffffffu)
        return SPW_STATUS_BAD_ARGUMENT;

    count = MultiByteToWideChar(CP_UTF8_VALUE, 0u, text, (int)text_length, buffer, capacity - 1);
    if (count <= 0)
        return SPW_STATUS_WIN32_ERROR;

    buffer[count] = 0;
    return SPW_STATUS_OK;
}

static int32_t spw_set_control_tooltip(SpwObject *object, const char *text, uint32_t text_length) {
    SpwObject *parent;
    TOOLINFOW tool = {0};
    int conversion_status;

    if (object == NULL || object->type != SPW_OBJECT_CONTROL)
        return SPW_STATUS_NOT_FOUND;

    conversion_status = spw_utf8_to_wide(text, text_length,
        object->tooltip_text, SPW_MAX_TOOLTIP_WCHARS);
    if (conversion_status != SPW_STATUS_OK)
        return conversion_status;

    if (object->tooltip_hwnd != NULL) {
        DestroyWindow(object->tooltip_hwnd);
        object->tooltip_hwnd = NULL;
    }

    if (object->tooltip_text[0] == 0)
        return SPW_STATUS_OK;

    parent = spw_find_object_by_id(object->parent_id);
    if (parent == NULL || parent->type != SPW_OBJECT_WINDOW)
        return SPW_STATUS_NOT_FOUND;

    object->tooltip_hwnd = CreateWindowExW(
        0u, g_tooltip_class_name, L"",
        WS_POPUP_VALUE | TTS_ALWAYSTIP_VALUE | TTS_NOPREFIX_VALUE,
        CW_USEDEFAULT_VALUE, CW_USEDEFAULT_VALUE,
        CW_USEDEFAULT_VALUE, CW_USEDEFAULT_VALUE,
        parent->hwnd, NULL, g_instance, NULL);
    if (object->tooltip_hwnd == NULL)
        return SPW_STATUS_WIN32_ERROR;

    tool.cbSize = (UINT)sizeof(TOOLINFOW);
    tool.uFlags = TTF_IDISHWND_VALUE | TTF_SUBCLASS_VALUE;
    tool.hwnd = parent->hwnd;
    tool.uId = (uintptr_t)object->hwnd;
    tool.lpszText = object->tooltip_text;
    if (SendMessageW(object->tooltip_hwnd, TTM_ADDTOOLW_VALUE, 0u,
            (LPARAM)(uintptr_t)&tool) == 0) {
        DestroyWindow(object->tooltip_hwnd);
        object->tooltip_hwnd = NULL;
        return SPW_STATUS_WIN32_ERROR;
    }

    return SPW_STATUS_OK;
}

static int32_t spw_recreate_text_area(SpwObject *object, DWORD style) {
    HWND old_hwnd;
    HWND parent_hwnd;
    HWND new_hwnd;
    RECT screen_rect;
    POINT top_left;
    POINT bottom_right;
    uint32_t text_units = 0u;
    uint32_t start_units = 0u;
    uint32_t end_units = 0u;
    int32_t first_line;
    int was_enabled;
    int was_visible;
    int had_focus;
    int32_t status;

    if (object == NULL || object->type != SPW_OBJECT_CONTROL ||
        object->kind != SPW_CONTROL_TEXT_AREA || object->hwnd == NULL)
        return SPW_STATUS_NOT_FOUND;

    old_hwnd = object->hwnd;
    parent_hwnd = GetParent(old_hwnd);
    if (parent_hwnd == NULL || !GetWindowRect(old_hwnd, &screen_rect))
        return SPW_STATUS_WIN32_ERROR;
    top_left.x = screen_rect.left;
    top_left.y = screen_rect.top;
    bottom_right.x = screen_rect.right;
    bottom_right.y = screen_rect.bottom;
    if (!ScreenToClient(parent_hwnd, &top_left) ||
        !ScreenToClient(parent_hwnd, &bottom_right))
        return SPW_STATUS_WIN32_ERROR;

    status = spw_control_text_wide(old_hwnd, &text_units);
    if (status != SPW_STATUS_OK)
        return status;
    SendMessageW(old_hwnd, EM_GETSEL_VALUE,
        (WPARAM)(uintptr_t)&start_units, (LPARAM)(uintptr_t)&end_units);
    first_line = (int32_t)SendMessageW(old_hwnd, EM_GETFIRSTVISIBLELINE_VALUE, 0, 0);
    was_enabled = IsWindowEnabled(old_hwnd) ? 1 : 0;
    was_visible = IsWindowVisible(old_hwnd) ? 1 : 0;
    had_focus = GetFocus() == old_hwnd ? 1 : 0;

    if (object->tooltip_hwnd != NULL) {
        DestroyWindow(object->tooltip_hwnd);
        object->tooltip_hwnd = NULL;
    }
    RemoveWindowSubclass(old_hwnd, spw_control_subclass_proc, 1u);
    if (!DestroyWindow(old_hwnd))
        return SPW_STATUS_WIN32_ERROR;

    new_hwnd = CreateWindowExW(
        0u, g_edit_class_name, g_control_text_wide, style,
        top_left.x, top_left.y,
        bottom_right.x - top_left.x, bottom_right.y - top_left.y,
        parent_hwnd, NULL, g_instance, NULL);
    if (new_hwnd == NULL)
        return SPW_STATUS_WIN32_ERROR;
    object->hwnd = new_hwnd;
    SendMessageW(new_hwnd, WM_SETFONT_VALUE,
        (WPARAM)(uintptr_t)GetStockObject(DEFAULT_GUI_FONT_VALUE), (LPARAM)TRUE_VALUE);
    if (!SetWindowSubclass(new_hwnd, spw_control_subclass_proc, 1u, 0u)) {
        DestroyWindow(new_hwnd);
        object->hwnd = NULL;
        return SPW_STATUS_WIN32_ERROR;
    }
    EnableWindow(new_hwnd, was_enabled ? TRUE_VALUE : FALSE_VALUE);
    ShowWindow(new_hwnd, was_visible ? SW_SHOW_VALUE : SW_HIDE_VALUE);
    if (object->placeholder_text[0] != 0)
        SendMessageW(new_hwnd, EM_SETCUEBANNER_VALUE, FALSE_VALUE,
            (LPARAM)(uintptr_t)object->placeholder_text);
    SendMessageW(new_hwnd, EM_SETSEL_VALUE, (WPARAM)start_units, (LPARAM)end_units);
    if (first_line > 0)
        SendMessageW(new_hwnd, EM_LINESCROLL_VALUE, 0, (LPARAM)first_line);
    if (had_focus)
        SetFocus(new_hwnd);

    /* Rebind a stored tooltip, if any, to the replacement HWND. */
    if (object->tooltip_text[0] != 0) {
        SpwObject *parent = spw_find_object_by_id(object->parent_id);
        TOOLINFOW tool = {0};
        if (parent != NULL && parent->type == SPW_OBJECT_WINDOW) {
            object->tooltip_hwnd = CreateWindowExW(
                0u, g_tooltip_class_name, L"",
                WS_POPUP_VALUE | TTS_ALWAYSTIP_VALUE | TTS_NOPREFIX_VALUE,
                CW_USEDEFAULT_VALUE, CW_USEDEFAULT_VALUE,
                CW_USEDEFAULT_VALUE, CW_USEDEFAULT_VALUE,
                parent->hwnd, NULL, g_instance, NULL);
            if (object->tooltip_hwnd != NULL) {
                tool.cbSize = (UINT)sizeof(TOOLINFOW);
                tool.uFlags = TTF_IDISHWND_VALUE | TTF_SUBCLASS_VALUE;
                tool.hwnd = parent->hwnd;
                tool.uId = (uintptr_t)new_hwnd;
                tool.lpszText = object->tooltip_text;
                if (SendMessageW(object->tooltip_hwnd, TTM_ADDTOOLW_VALUE, 0u,
                        (LPARAM)(uintptr_t)&tool) == 0) {
                    DestroyWindow(object->tooltip_hwnd);
                    object->tooltip_hwnd = NULL;
                }
            }
        }
    }
    return SPW_STATUS_OK;
}

