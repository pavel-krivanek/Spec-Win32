/* Window procedure and commands --------------------------------------- */
static void spw_complete_command(SpwCommand *command, int32_t status) {
    command->status = status;
    if (status == SPW_STATUS_WIN32_ERROR)
        command->win32_error = GetLastError();
    SetEvent(command->done);
}

static void spw_destroy_all_windows(void) {
    uint32_t i;
    for (i = 0u; i < SPW_OBJECT_CAPACITY; ++i) {
        if (g_objects[i].alive && g_objects[i].hwnd != NULL)
            DestroyWindow(g_objects[i].hwnd);
    }
}

static int32_t spw_move_window_centered(HWND hwnd, HWND relative_to) {
    RECT window_rect;
    RECT area;
    MONITORINFO monitor_info;
    HMONITOR monitor;
    int32_t width;
    int32_t height;
    int32_t x;
    int32_t y;

    if (hwnd == NULL || !GetWindowRect(hwnd, &window_rect))
        return SPW_STATUS_WIN32_ERROR;

    width = window_rect.right - window_rect.left;
    height = window_rect.bottom - window_rect.top;

    if (relative_to != NULL) {
        if (!GetWindowRect(relative_to, &area))
            return SPW_STATUS_WIN32_ERROR;
    } else {
        monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST_VALUE);
        if (monitor == NULL)
            return SPW_STATUS_WIN32_ERROR;
        monitor_info.cbSize = (DWORD)sizeof(MONITORINFO);
        if (!GetMonitorInfoW(monitor, &monitor_info))
            return SPW_STATUS_WIN32_ERROR;
        area = monitor_info.rcWork;
    }

    x = area.left + ((area.right - area.left - width) / 2);
    y = area.top + ((area.bottom - area.top - height) / 2);

    /* Relative centering should remain visible on the owner's monitor work area. */
    if (relative_to != NULL) {
        monitor = MonitorFromWindow(relative_to, MONITOR_DEFAULTTONEAREST_VALUE);
        if (monitor != NULL) {
            monitor_info.cbSize = (DWORD)sizeof(MONITORINFO);
            if (GetMonitorInfoW(monitor, &monitor_info)) {
                if (width <= monitor_info.rcWork.right - monitor_info.rcWork.left) {
                    if (x < monitor_info.rcWork.left) x = monitor_info.rcWork.left;
                    if (x + width > monitor_info.rcWork.right) x = monitor_info.rcWork.right - width;
                } else {
                    x = monitor_info.rcWork.left;
                }
                if (height <= monitor_info.rcWork.bottom - monitor_info.rcWork.top) {
                    if (y < monitor_info.rcWork.top) y = monitor_info.rcWork.top;
                    if (y + height > monitor_info.rcWork.bottom) y = monitor_info.rcWork.bottom - height;
                } else {
                    y = monitor_info.rcWork.top;
                }
            }
        }
    }

    if (!MoveWindow(hwnd, x, y, width, height, TRUE_VALUE))
        return SPW_STATUS_WIN32_ERROR;
    return SPW_STATUS_OK;
}

static HWND spw_top_level_hwnd_for_object(SpwObject *object) {
    SpwObject *parent;
    if (object == NULL)
        return NULL;
    if (object->type == SPW_OBJECT_WINDOW)
        return object->hwnd;
    parent = spw_find_object_by_id(object->parent_id);
    if (parent != NULL && parent->type == SPW_OBJECT_WINDOW)
        return parent->hwnd;
    return NULL;
}

static int32_t spw_scroll_view_apply(SpwObject *viewport);
static int32_t spw_paned_apply(SpwObject *paned, int push_event);
static int32_t spw_paned_main_extent(SpwObject *paned);

static HWND spw_dialog_owner_hwnd(uint64_t object_id) {
    SpwObject *object;
    HWND foreground;
    uint32_t i;
    if (object_id != 0u) {
        object = spw_find_object_by_id(object_id);
        if (object != NULL && object->hwnd != NULL)
            return object->hwnd;
    }
    foreground = GetForegroundWindow();
    object = spw_find_object_by_hwnd(foreground);
    if (object != NULL && object->type == SPW_OBJECT_WINDOW)
        return foreground;
    for (i = 0u; i < SPW_OBJECT_CAPACITY; ++i) {
        if (g_objects[i].alive && g_objects[i].type == SPW_OBJECT_WINDOW &&
            g_objects[i].hwnd != NULL && IsWindowVisible(g_objects[i].hwnd))
            return g_objects[i].hwnd;
    }
    return NULL;
}

static int32_t spw_wide_path_to_utf8(const WCHAR *path, SpwCommand *command) {
    int wide_length = 0;
    int required;
    if (path == NULL || command == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    while (path[wide_length] != 0 && wide_length < SPW_MAX_DIALOG_PATH_WCHARS - 1)
        ++wide_length;
    required = WideCharToMultiByte(CP_UTF8_VALUE, 0u, path, wide_length,
                                    NULL, 0, NULL, NULL);
    if (required < 0)
        return SPW_STATUS_WIN32_ERROR;
    command->output_length = (uint32_t)required;
    if (required == 0)
        return SPW_STATUS_OK;
    if (command->output_text == NULL || command->output_capacity <= (uint32_t)required)
        return SPW_STATUS_CAPACITY;
    if (WideCharToMultiByte(CP_UTF8_VALUE, 0u, path, wide_length,
                            command->output_text, required, NULL, NULL) != required)
        return SPW_STATUS_WIN32_ERROR;
    command->output_text[required] = 0;
    return SPW_STATUS_OK;
}

static int32_t WINAPI spw_browse_callback(HWND hwnd, UINT message, LPARAM lParam, LPARAM data) {
    (void)lParam;
    if (message == BFFM_INITIALIZED_VALUE && data != 0)
        SendMessageW(hwnd, BFFM_SETSELECTIONW_VALUE, (WPARAM)1u, data);
    return 0;
}

static int32_t spw_run_file_dialog(SpwCommand *command) {
    HWND owner;
    int status;
    if (command == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    owner = spw_dialog_owner_hwnd(command->object_id);
    status = spw_utf8_to_wide(command->text, command->text_length,
                              g_dialog_title_wide, SPW_MAX_TITLE_WCHARS);
    if (status != SPW_STATUS_OK) return status;
    status = spw_utf8_to_wide(command->text2, command->text2_length,
                              g_dialog_initial_dir_wide, SPW_MAX_DIALOG_PATH_WCHARS);
    if (status != SPW_STATUS_OK) return status;
    status = spw_utf8_to_wide(command->text3, command->text3_length,
                              g_dialog_path_wide, SPW_MAX_DIALOG_PATH_WCHARS);
    if (status != SPW_STATUS_OK) return status;

    if (command->value == SPW_FILE_DIALOG_DIRECTORY) {
        BROWSEINFOW browse = {0};
        void *pidl;
        BOOL restore_owner = owner != NULL && IsWindowEnabled(owner);
        int32_t result;
        g_dialog_display_wide[0] = 0;
        browse.hwndOwner = owner;
        browse.pszDisplayName = g_dialog_display_wide;
        browse.lpszTitle = g_dialog_title_wide[0] != 0 ? g_dialog_title_wide : NULL;
        browse.ulFlags = BIF_RETURNONLYFSDIRS_VALUE | BIF_NEWDIALOGSTYLE_VALUE | BIF_EDITBOX_VALUE;
        browse.lpfn = spw_browse_callback;
        browse.lParam = g_dialog_initial_dir_wide[0] != 0 ? (LPARAM)(uintptr_t)g_dialog_initial_dir_wide : 0;
        if (restore_owner) EnableWindow(owner, FALSE_VALUE);
        pidl = SHBrowseForFolderW(&browse);
        if (restore_owner) { EnableWindow(owner, TRUE_VALUE); SetForegroundWindow(owner); }
        if (pidl == NULL) {
            command->output_length = 0u;
            if (command->output_text != NULL && command->output_capacity > 0u) command->output_text[0] = 0;
            return SPW_STATUS_OK;
        }
        if (!SHGetPathFromIDListW(pidl, g_dialog_path_wide)) {
            CoTaskMemFree(pidl);
            return SPW_STATUS_WIN32_ERROR;
        }
        CoTaskMemFree(pidl);
        result = spw_wide_path_to_utf8(g_dialog_path_wide, command);
        return result;
    }

    status = spw_utf8_to_wide((const char *)command->bytes, command->byte_length,
                              g_dialog_filter_wide, SPW_MAX_DIALOG_FILTER_WCHARS);
    if (status != SPW_STATUS_OK) return status;
    {
        OPENFILENAMEW ofn = {0};
        BOOL accepted;
        ofn.lStructSize = (DWORD)sizeof(OPENFILENAMEW);
        ofn.hwndOwner = owner;
        ofn.lpstrFilter = g_dialog_filter_wide[0] != 0 ? g_dialog_filter_wide : NULL;
        ofn.nFilterIndex = 1u;
        ofn.lpstrFile = g_dialog_path_wide;
        ofn.nMaxFile = SPW_MAX_DIALOG_PATH_WCHARS;
        ofn.lpstrInitialDir = g_dialog_initial_dir_wide[0] != 0 ? g_dialog_initial_dir_wide : NULL;
        ofn.lpstrTitle = g_dialog_title_wide[0] != 0 ? g_dialog_title_wide : NULL;
        ofn.Flags = OFN_EXPLORER_VALUE | OFN_NOCHANGEDIR_VALUE | OFN_PATHMUSTEXIST_VALUE | OFN_HIDEREADONLY_VALUE;
        if (command->value == SPW_FILE_DIALOG_OPEN)
            ofn.Flags |= OFN_FILEMUSTEXIST_VALUE;
        else if (command->value == SPW_FILE_DIALOG_SAVE)
            ofn.Flags |= OFN_OVERWRITEPROMPT_VALUE;
        else
            return SPW_STATUS_BAD_ARGUMENT;
        accepted = command->value == SPW_FILE_DIALOG_SAVE
            ? GetSaveFileNameW(&ofn)
            : GetOpenFileNameW(&ofn);
        if (!accepted) {
            if (CommDlgExtendedError() != 0u)
                return SPW_STATUS_WIN32_ERROR;
            command->output_length = 0u;
            if (command->output_text != NULL && command->output_capacity > 0u) command->output_text[0] = 0;
            return SPW_STATUS_OK;
        }
        return spw_wide_path_to_utf8(g_dialog_path_wide, command);
    }
}

static void spw_process_command(SpwCommand *command) {
    SpwObject *object;
    WCHAR wide_text[SPW_MAX_TITLE_WCHARS];
    int conversion_status;

    if (command == NULL)
        return;

    switch ((SpwCommandOpcode)command->opcode) {
        case SPW_CMD_CREATE_WINDOW: {
            HWND hwnd;
            conversion_status = spw_utf8_to_wide(command->text, command->text_length,
                                                  wide_text, SPW_MAX_TITLE_WCHARS);
            if (conversion_status != SPW_STATUS_OK) {
                spw_complete_command(command, conversion_status);
                return;
            }

            hwnd = CreateWindowExW(
                0u,
                g_window_class_name,
                wide_text,
                WS_OVERLAPPEDWINDOW_VALUE,
                command->x,
                command->y,
                command->width,
                command->height,
                NULL,
                NULL,
                g_instance,
                NULL);
            if (hwnd == NULL) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }

            object = spw_register_object(hwnd, SPW_OBJECT_WINDOW, 0u, 0u);
            if (object == NULL) {
                DestroyWindow(hwnd);
                spw_complete_command(command, SPW_STATUS_CAPACITY);
                return;
            }

            command->object_id = object->id;
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_CREATE_POPUP_WINDOW: {
            HWND hwnd;
            HWND owner_hwnd = NULL;
            SpwObject *owner = NULL;
            if (command->parent_id != 0u) {
                owner = spw_find_object_by_id(command->parent_id);
                owner_hwnd = spw_top_level_hwnd_for_object(owner);
                if (owner_hwnd == NULL) {
                    spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                    return;
                }
            }
            hwnd = CreateWindowExW(
                WS_EX_TOOLWINDOW_VALUE,
                g_window_class_name,
                L"",
                WS_POPUP_VALUE | WS_BORDER_VALUE,
                command->x,
                command->y,
                command->width,
                command->height,
                owner_hwnd,
                NULL,
                g_instance,
                NULL);
            if (hwnd == NULL) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            object = spw_register_object(hwnd, SPW_OBJECT_WINDOW, 0u, 0u);
            if (object == NULL) {
                DestroyWindow(hwnd);
                spw_complete_command(command, SPW_STATUS_CAPACITY);
                return;
            }
            object->popup_window = 1u;
            object->popup_autohide = command->value ? 1u : 0u;
            command->object_id = object->id;
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_SHOW_WINDOW:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            ShowWindow(object->hwnd, command->value);
            UpdateWindow(object->hwnd);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_SET_TITLE:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            conversion_status = spw_utf8_to_wide(command->text, command->text_length,
                                                  wide_text, SPW_MAX_TITLE_WCHARS);
            if (conversion_status != SPW_STATUS_OK) {
                spw_complete_command(command, conversion_status);
                return;
            }
            if (!SetWindowTextW(object->hwnd, wide_text)) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_SET_WINDOW_MENU: {
            uint32_t slot, i;
            HMENU menu = NULL;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            slot = spw_object_slot(object);
            spw_release_window_menu(object, 1);
            if (command->item_count == 0u) {
                spw_complete_command(command, SPW_STATUS_OK);
                return;
            }
            conversion_status = spw_build_window_menu(
                (const uint8_t *)command->text, command->text_length,
                command->item_count, g_window_menu_bitmaps[slot], &menu);
            if (conversion_status != SPW_STATUS_OK) {
                spw_complete_command(command, conversion_status);
                return;
            }
            if (!SetMenu(object->hwnd, menu)) {
                DestroyMenu(menu);
                for (i = 0u; i < command->item_count; ++i) {
                    if (g_window_menu_bitmaps[slot][i] != NULL) {
                        DeleteObject((HGDIOBJ)g_window_menu_bitmaps[slot][i]);
                        g_window_menu_bitmaps[slot][i] = NULL;
                    }
                }
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            g_window_menus[slot] = menu;
            g_window_menu_item_counts[slot] = command->item_count;
            DrawMenuBar(object->hwnd);
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_SET_BOUNDS:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (!MoveWindow(object->hwnd, command->x, command->y,
                            command->width, command->height, TRUE_VALUE)) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_DESTROY_WINDOW:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (!DestroyWindow(object->hwnd)) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_IS_VISIBLE:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = IsWindowVisible(object->hwnd) ? 1 : 0;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_ACTIVATE_WINDOW:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = SetForegroundWindow(object->hwnd) ? 1 : 0;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_GET_CLIENT_SIZE: {
            RECT rect;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (!GetClientRect(object->hwnd, &rect)) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            command->width = rect.right - rect.left;
            command->height = rect.bottom - rect.top;
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_GET_WINDOW_BOUNDS: {
            RECT rect;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (!GetWindowRect(object->hwnd, &rect)) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            command->x = rect.left;
            command->y = rect.top;
            command->width = rect.right - rect.left;
            command->height = rect.bottom - rect.top;
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_IS_MINIMIZED:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = IsIconic(object->hwnd) ? 1 : 0;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_IS_MAXIMIZED:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = IsZoomed(object->hwnd) ? 1 : 0;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_IS_FOREGROUND:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = GetForegroundWindow() == object->hwnd ? 1 : 0;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CENTER_WINDOW:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            spw_complete_command(command, spw_move_window_centered(object->hwnd, NULL));
            return;

        case SPW_CMD_CENTER_WINDOW_RELATIVE: {
            SpwObject *relative;
            object = spw_find_object_by_id(command->object_id);
            relative = spw_find_object_by_id(command->parent_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW ||
                relative == NULL || relative->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            spw_complete_command(command, spw_move_window_centered(object->hwnd, relative->hwnd));
            return;
        }

        case SPW_CMD_SET_WINDOW_OWNER: {
            SpwObject *owner = NULL;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (command->parent_id != 0u) {
                owner = spw_find_object_by_id(command->parent_id);
                if (spw_top_level_hwnd_for_object(owner) == NULL) {
                    spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                    return;
                }
            }
            {
                HWND owner_hwnd = spw_top_level_hwnd_for_object(owner);
                SetWindowLongPtrW(object->hwnd, GWLP_HWNDPARENT_VALUE,
                    owner_hwnd == NULL ? (LONG_PTR)0 : (LONG_PTR)(uintptr_t)owner_hwnd);
                if ((HWND)(uintptr_t)GetWindowLongPtrW(object->hwnd, GWLP_HWNDPARENT_VALUE) != owner_hwnd) {
                    spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                    return;
                }
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_SET_WINDOW_ENABLED:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            EnableWindow(object->hwnd, command->value ? TRUE_VALUE : FALSE_VALUE);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_IS_WINDOW_ENABLED:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = IsWindowEnabled(object->hwnd) ? 1 : 0;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CREATE_CONTROL: {
            HWND hwnd;
            SpwObject *parent;
            const WCHAR *class_name = spw_control_class_name(command->kind);
            DWORD style = spw_control_style(command->kind);
            if (command->kind == SPW_CONTROL_CODE) {
                int32_t code_status = spw_code_ensure_loaded();
                if (code_status != SPW_STATUS_OK) {
                    spw_complete_command(command, code_status);
                    return;
                }
            }
            if (class_name == NULL || style == 0u) {
                spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
                return;
            }
            parent = spw_find_object_by_id(command->parent_id);
            if (parent == NULL || parent->hwnd == NULL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            conversion_status = spw_utf8_to_wide(
                command->text, command->text_length,
                spw_control_is_text_input(command->kind) ? g_control_text_wide : wide_text,
                spw_control_is_text_input(command->kind) ? SPW_MAX_CONTROL_TEXT_WCHARS : SPW_MAX_TITLE_WCHARS);
            if (conversion_status != SPW_STATUS_OK) {
                spw_complete_command(command, conversion_status);
                return;
            }
            if (spw_control_is_multiline_text(command->kind)) {
                conversion_status = spw_text_area_expand_newlines(g_control_text_wide, g_text_area_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
                if (conversion_status != SPW_STATUS_OK) {
                    spw_complete_command(command, conversion_status);
                    return;
                }
            }
            if (command->kind == SPW_CONTROL_LINK) {
                conversion_status = spw_link_markup(wide_text, g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
                if (conversion_status != SPW_STATUS_OK) {
                    spw_complete_command(command, conversion_status);
                    return;
                }
            }
            hwnd = CreateWindowExW(
                0u, class_name,
                command->kind == SPW_CONTROL_LINK ? g_control_text_wide :
                    (spw_control_is_multiline_text(command->kind) ? g_text_area_wide :
                        (spw_control_is_text_input(command->kind) ? g_control_text_wide : wide_text)), style,
                command->x, command->y, command->width,
                spw_control_is_drop_list(command->kind) && command->height < 160 ? 160 : command->height,
                parent->hwnd, NULL, g_instance, NULL);
            if (hwnd == NULL) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            /* Predefined controls otherwise use an old system font rather than
               the normal Windows GUI font. WM_SETFONT keeps native controls
               visually consistent with ordinary Windows applications. */
            SendMessageW(hwnd, WM_SETFONT_VALUE,
                (WPARAM)(uintptr_t)GetStockObject(DEFAULT_GUI_FONT_VALUE),
                (LPARAM)TRUE_VALUE);
            if (command->kind == SPW_CONTROL_NUMBER_INPUT || command->kind == SPW_CONTROL_SEARCH_INPUT)
                SendMessageW(hwnd, EM_SETMARGINS_VALUE, EC_RIGHTMARGIN_VALUE,
                    (LPARAM)(uintptr_t)SPW_EDIT_AFFORDANCE_WIDTH);
            if (spw_control_is_list(command->kind)) {
                LVCOLUMNW column;
                column.mask = LVCF_FMT_VALUE | LVCF_WIDTH_VALUE | LVCF_TEXT_VALUE | LVCF_SUBITEM_VALUE;
                column.fmt = LVCFMT_LEFT_VALUE;
                column.cx = command->width > 4 ? command->width - 4 : 1;
                column.pszText = (WCHAR *)L"";
                column.cchTextMax = 0;
                column.iSubItem = 0;
                if (SendMessageW(hwnd, LVM_INSERTCOLUMNW_VALUE, 0u, (LPARAM)(uintptr_t)&column) < 0) {
                    DestroyWindow(hwnd);
                    spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                    return;
                }
                SendMessageW(hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE_VALUE,
                    (WPARAM)(LVS_EX_FULLROWSELECT_VALUE | LVS_EX_DOUBLEBUFFER_VALUE),
                    (LPARAM)(LVS_EX_FULLROWSELECT_VALUE | LVS_EX_DOUBLEBUFFER_VALUE));
            } else if (spw_control_is_table(command->kind)) {
                SendMessageW(hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE_VALUE,
                    (WPARAM)(LVS_EX_FULLROWSELECT_VALUE | LVS_EX_DOUBLEBUFFER_VALUE),
                    (LPARAM)(LVS_EX_FULLROWSELECT_VALUE | LVS_EX_DOUBLEBUFFER_VALUE));
            }
            if (spw_control_is_slider(command->kind)) {
                SendMessageW(hwnd, TBM_SETRANGEMIN_VALUE, (WPARAM)FALSE_VALUE, (LPARAM)0);
                SendMessageW(hwnd, TBM_SETRANGEMAX_VALUE, (WPARAM)TRUE_VALUE, (LPARAM)SPW_NORMALIZED_MAX);
                SendMessageW(hwnd, TBM_SETTICFREQ_VALUE, (WPARAM)(SPW_NORMALIZED_MAX / 10), 0);
                SendMessageW(hwnd, TBM_SETPOS_VALUE, (WPARAM)TRUE_VALUE, (LPARAM)0);
            } else if (spw_control_is_progress(command->kind)) {
                SendMessageW(hwnd, PBM_SETRANGE32_VALUE, (WPARAM)0, (LPARAM)SPW_NORMALIZED_MAX);
                SendMessageW(hwnd, PBM_SETPOS_VALUE, (WPARAM)0, 0);
            }
            object = spw_register_object(hwnd, SPW_OBJECT_CONTROL, command->kind, command->parent_id);
            if (object == NULL) {
                DestroyWindow(hwnd);
                spw_complete_command(command, SPW_STATUS_CAPACITY);
                return;
            }
            if (!SetWindowSubclass(hwnd, spw_control_subclass_proc, 1u, 0u)) {
                spw_unregister_object(object);
                DestroyWindow(hwnd);
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            if (command->kind == SPW_CONTROL_CODE) {
                int32_t code_status = spw_code_initialize_control(object);
                if (code_status != SPW_STATUS_OK) {
                    RemoveWindowSubclass(hwnd, spw_control_subclass_proc, 1u);
                    spw_unregister_object(object);
                    DestroyWindow(hwnd);
                    spw_complete_command(command, code_status);
                    return;
                }
            }
            if (command->kind == SPW_CONTROL_TREE_COLUMN) {
                int32_t tree_column_status = spw_tree_column_create_children(object);
                if (tree_column_status != SPW_STATUS_OK) {
                    RemoveWindowSubclass(hwnd, spw_control_subclass_proc, 1u);
                    spw_unregister_object(object);
                    DestroyWindow(hwnd);
                    spw_complete_command(command, tree_column_status);
                    return;
                }
            }
            if (command->kind == SPW_CONTROL_SPINNER) {
                if (SetTimer(hwnd, 1u, 80u, NULL) == 0u) {
                    RemoveWindowSubclass(hwnd, spw_control_subclass_proc, 1u);
                    spw_unregister_object(object);
                    DestroyWindow(hwnd);
                    spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                    return;
                }
            }
            command->object_id = object->id;
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_CONTROL_SET_TEXT:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            conversion_status = spw_utf8_to_wide(
                command->text,
                command->text_length,
                spw_control_is_text_input(object->kind) ? g_control_text_wide : wide_text,
                spw_control_is_text_input(object->kind) ? SPW_MAX_CONTROL_TEXT_WCHARS : SPW_MAX_TITLE_WCHARS);
            if (conversion_status != SPW_STATUS_OK) {
                spw_complete_command(command, conversion_status);
                return;
            }
            if (spw_control_is_multiline_text(object->kind)) {
                conversion_status = spw_text_area_expand_newlines(g_control_text_wide, g_text_area_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
                if (conversion_status != SPW_STATUS_OK) {
                    spw_complete_command(command, conversion_status);
                    return;
                }
            }
            if (object->kind == SPW_CONTROL_LINK) {
                conversion_status = spw_link_markup(wide_text, g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
                if (conversion_status != SPW_STATUS_OK) {
                    spw_complete_command(command, conversion_status);
                    return;
                }
            }
            object->suppress_notifications += 1u;
            if (!SetWindowTextW(
                    object->hwnd,
                    object->kind == SPW_CONTROL_LINK
                        ? g_control_text_wide
                        : (spw_control_is_multiline_text(object->kind)
                            ? g_text_area_wide
                            : (spw_control_is_text_input(object->kind) ? g_control_text_wide : wide_text)))) {
                object->suppress_notifications -= 1u;
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            object->suppress_notifications -= 1u;
            if ((object->kind == SPW_CONTROL_BUTTON || object->kind == SPW_CONTROL_TOGGLE_BUTTON) &&
                object->image_bitmap != NULL) {
                conversion_status = spw_refresh_button_image_list(object);
                if (conversion_status != SPW_STATUS_OK) {
                    spw_complete_command(command, conversion_status);
                    return;
                }
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_TOOLTIP:
            object = spw_find_object_by_id(command->object_id);
            conversion_status = spw_set_control_tooltip(object, command->text, command->text_length);
            spw_complete_command(command, conversion_status);
            return;

        case SPW_CMD_CONTROL_SET_CONTEXT_MENU_ENABLED:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            object->context_menu_enabled = command->value ? 1u : 0u;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SHOW_CONTEXT_MENU:
            object = spw_find_object_by_id(command->object_id);
            conversion_status = spw_show_context_menu(
                object, command->text, command->text_length,
                command->indexes, command->item_count,
                command->x, command->y, &command->result32);
            spw_complete_command(command, conversion_status);
            return;

        case SPW_CMD_CONTROL_SHOW_POPUP_MENU:
            object = command->object_id == 0u ? NULL : spw_find_object_by_id(command->object_id);
            conversion_status = spw_show_popup_menu(
                object, (const uint8_t *)command->text, command->text_length,
                command->item_count, command->x, command->y, &command->result32);
            spw_complete_command(command, conversion_status);
            return;

        case SPW_CMD_CONTROL_GET_TEXT: {
            int wide_length;
            int required;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            wide_length = GetWindowTextLengthW(object->hwnd);
            if (wide_length < 0 || wide_length >= SPW_MAX_CONTROL_TEXT_WCHARS) {
                spw_complete_command(command, SPW_STATUS_CAPACITY);
                return;
            }
            if (wide_length > 0 &&
                GetWindowTextW(object->hwnd, g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS) == 0) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            if (wide_length == 0)
                g_control_text_wide[0] = 0;
            if (spw_control_is_multiline_text(object->kind))
                wide_length = (int)spw_text_area_normalize_newlines(g_control_text_wide, (uint32_t)wide_length);
            required = WideCharToMultiByte(
                CP_UTF8_VALUE, 0u, g_control_text_wide, wide_length,
                NULL, 0, NULL, NULL);
            if (wide_length > 0 && required == 0) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            command->output_length = (uint32_t)required;
            if (command->output_text == NULL || command->output_capacity == 0u) {
                spw_complete_command(command, SPW_STATUS_OK);
                return;
            }
            if (command->output_capacity <= (uint32_t)required) {
                spw_complete_command(command, SPW_STATUS_CAPACITY);
                return;
            }
            if (required > 0) {
                int converted = WideCharToMultiByte(
                    CP_UTF8_VALUE, 0u, g_control_text_wide, wide_length,
                    command->output_text, (int)command->output_capacity - 1,
                    NULL, NULL);
                if (converted != required) {
                    spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                    return;
                }
            }
            command->output_text[required] = 0;
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_CONTROL_SET_CHECKED:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_checkable(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (object->kind == SPW_CONTROL_COMPONENT_ROW) {
                object->component_row_selected = command->value ? 1u : 0u;
                InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
            } else {
                SendMessageW(object->hwnd, BM_SETCHECK_VALUE,
                    command->value ? BST_CHECKED_VALUE : BST_UNCHECKED_VALUE, 0);
                if (object->kind == SPW_CONTROL_SWITCH)
                    InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_GET_CHECKED:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_checkable(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = object->kind == SPW_CONTROL_COMPONENT_ROW
                ? (object->component_row_selected ? 1 : 0)
                : (SendMessageW(object->hwnd, BM_GETCHECK_VALUE, 0, 0) == BST_CHECKED_VALUE ? 1 : 0);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_EDITABLE:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_text_input(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            SendMessageW(object->hwnd, EM_SETREADONLY_VALUE, command->value ? FALSE_VALUE : TRUE_VALUE, 0);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_MAX_LENGTH:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_text_input(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            SendMessageW(object->hwnd, EM_SETLIMITTEXT_VALUE, (WPARAM)(uintptr_t)(uint32_t)command->value, 0);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_PASSWORD:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_text_input(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            SendMessageW(object->hwnd, EM_SETPASSWORDCHAR_VALUE,
                command->value ? (WPARAM)0x2022u : 0u, 0);
            InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_PLACEHOLDER:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_text_input(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            conversion_status = spw_utf8_to_wide(command->text, command->text_length,
                                                  object->placeholder_text, SPW_MAX_PLACEHOLDER_WCHARS);
            if (conversion_status != SPW_STATUS_OK) {
                spw_complete_command(command, conversion_status);
                return;
            }
            SendMessageW(object->hwnd, EM_SETCUEBANNER_VALUE, FALSE_VALUE,
                         (LPARAM)(uintptr_t)object->placeholder_text);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_IMAGE_BGRA:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command, spw_set_control_image_bgra(
                object, command->bytes, command->byte_length, command->width, command->height));
            return;

        case SPW_CMD_CONTROL_SET_IMAGE_AUTO_SCALE:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            object->image_auto_scale = command->value ? 1u : 0u;
            InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_ITEMS:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL ||
                (!spw_control_has_selection(object->kind) && !spw_control_is_list_view(object->kind))) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            spw_complete_command(command, spw_control_is_list(object->kind)
                ? spw_list_set_items(object, command->text, command->text_length, command->item_count)
                : spw_selection_control_set_items(object, command->text, command->text_length, command->item_count));
            return;

        case SPW_CMD_CONTROL_SET_LIST_ROWS:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command, spw_list_apply_rows(
                object, command->bytes, command->byte_length, command->item_count));
            return;

        case SPW_CMD_CONTROL_SET_SELECTED_INDEX: {
            LRESULT result;
            int32_t native_index;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_has_selection(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (command->value < 0) {
                spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
                return;
            }
            native_index = command->value == 0 ? -1 : command->value - 1;
            object->suppress_notifications += 1u;
            result = spw_control_is_drop_list(object->kind)
                ? SendMessageW(object->hwnd, CB_SETCURSEL_VALUE, (WPARAM)(intptr_t)native_index, 0)
                : SendMessageW(object->hwnd, TCM_SETCURSEL_VALUE, (WPARAM)(intptr_t)native_index, 0);
            object->suppress_notifications -= 1u;
            if (command->value > 0 && result < 0) {
                spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
                return;
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_CONTROL_GET_SELECTED_INDEX: {
            LRESULT result;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_drop_list(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            result = spw_control_is_drop_list(object->kind)
                ? SendMessageW(object->hwnd, CB_GETCURSEL_VALUE, 0u, 0)
                : SendMessageW(object->hwnd, TCM_GETCURSEL_VALUE, 0u, 0);
            command->result32 = result < 0 ? 0 : (int32_t)result + 1;
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_CONTROL_SET_VALUE:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL ||
                (!spw_control_is_slider(object->kind) && !spw_control_is_progress(object->kind))) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->value = spw_clamp_normalized_value(command->value);
            object->normalized_value = command->value;
            if (spw_control_is_slider(object->kind))
                SendMessageW(object->hwnd, TBM_SETPOS_VALUE, (WPARAM)TRUE_VALUE, (LPARAM)command->value);
            else if (!object->indeterminate)
                SendMessageW(object->hwnd, PBM_SETPOS_VALUE, (WPARAM)command->value, 0);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_GET_VALUE:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_slider(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = (int32_t)SendMessageW(object->hwnd, TBM_GETPOS_VALUE, 0, 0);
            object->normalized_value = spw_clamp_normalized_value(command->result32);
            command->result32 = object->normalized_value;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_INDETERMINATE: {
            LONG_PTR style;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_progress(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            style = GetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE);
            if (command->value) {
                if (!object->indeterminate) {
                    SetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE, style | (LONG_PTR)PBS_MARQUEE_VALUE);
                    SendMessageW(object->hwnd, PBM_SETMARQUEE_VALUE, (WPARAM)TRUE_VALUE, (LPARAM)30);
                    object->indeterminate = 1u;
                }
            } else {
                if (object->indeterminate) {
                    SendMessageW(object->hwnd, PBM_SETMARQUEE_VALUE, (WPARAM)FALSE_VALUE, 0);
                    SetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE, style & ~((LONG_PTR)PBS_MARQUEE_VALUE));
                    object->indeterminate = 0u;
                    SendMessageW(object->hwnd, PBM_SETRANGE32_VALUE, (WPARAM)0, (LPARAM)SPW_NORMALIZED_MAX);
                    SendMessageW(object->hwnd, PBM_SETPOS_VALUE, (WPARAM)object->normalized_value, 0);
                    InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
                }
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_CONTROL_SET_ENABLED:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            EnableWindow(object->hwnd, command->value ? TRUE_VALUE : FALSE_VALUE);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SHOW:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            ShowWindow(object->hwnd, command->value ? SW_SHOW_VALUE : SW_HIDE_VALUE);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_FOCUS:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            SetFocus(object->kind == SPW_CONTROL_TREE_COLUMN && object->tree_hwnd != NULL ? object->tree_hwnd : object->hwnd);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_HAS_FOCUS:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = (GetFocus() == object->hwnd ||
                (object->kind == SPW_CONTROL_TREE_COLUMN && (GetFocus() == object->tree_hwnd || GetFocus() == object->table_hwnd))) ? 1 : 0;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_SELECTION: {
            uint32_t units;
            uint32_t start_units;
            uint32_t end_units;
            int32_t text_status;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL ||
                !spw_control_is_text_input(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (command->x < 0 || command->y < 0) {
                spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
                return;
            }
            text_status = spw_control_text_wide(object->hwnd, &units);
            if (text_status != SPW_STATUS_OK) {
                spw_complete_command(command, text_status);
                return;
            }
            start_units = spw_control_is_multiline_text(object->kind)
                ? spw_text_area_utf16_units_for_offset(g_control_text_wide, units, (uint32_t)command->x)
                : spw_utf16_units_for_codepoint_offset(g_control_text_wide, units, (uint32_t)command->x);
            end_units = spw_control_is_multiline_text(object->kind)
                ? spw_text_area_utf16_units_for_offset(g_control_text_wide, units, (uint32_t)command->y)
                : spw_utf16_units_for_codepoint_offset(g_control_text_wide, units, (uint32_t)command->y);
            if (end_units < start_units)
                end_units = start_units;
            SendMessageW(object->hwnd, EM_SETSEL_VALUE, (WPARAM)start_units, (LPARAM)end_units);
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_CONTROL_GET_SELECTION: {
            uint32_t units;
            uint32_t start_units = 0u;
            uint32_t end_units = 0u;
            int32_t text_status;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL ||
                !spw_control_is_text_input(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            text_status = spw_control_text_wide(object->hwnd, &units);
            if (text_status != SPW_STATUS_OK) {
                spw_complete_command(command, text_status);
                return;
            }
            SendMessageW(object->hwnd, EM_GETSEL_VALUE,
                (WPARAM)(uintptr_t)&start_units, (LPARAM)(uintptr_t)&end_units);
            command->x = (int32_t)(spw_control_is_multiline_text(object->kind)
                ? spw_text_area_offset_for_utf16_units(g_control_text_wide, units, start_units)
                : spw_codepoint_offset_for_utf16_units(g_control_text_wide, units, start_units));
            command->y = (int32_t)(spw_control_is_multiline_text(object->kind)
                ? spw_text_area_offset_for_utf16_units(g_control_text_wide, units, end_units)
                : spw_codepoint_offset_for_utf16_units(g_control_text_wide, units, end_units));
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_TEXT_CONFIGURE: {
            LONG_PTR style;
            int old_wrap;
            int new_wrap;
            int32_t recreate_status;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_multiline_text(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (object->kind == SPW_CONTROL_CODE) {
                spw_complete_command(command, spw_code_configure(object, command->value, command->aux_value, command->x));
                return;
            }
            style = GetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE);
            old_wrap = (style & (LONG_PTR)ES_AUTOHSCROLL_VALUE) == 0;
            new_wrap = command->value ? 1 : 0;
            style |= (LONG_PTR)(ES_MULTILINE_VALUE | ES_AUTOVSCROLL_VALUE | ES_WANTRETURN_VALUE);
            if (command->aux_value) style |= (LONG_PTR)WS_VSCROLL_VALUE;
            else style &= ~((LONG_PTR)WS_VSCROLL_VALUE);
            if (new_wrap) {
                style &= ~((LONG_PTR)ES_AUTOHSCROLL_VALUE);
                style &= ~((LONG_PTR)WS_HSCROLL_VALUE);
            } else {
                style |= (LONG_PTR)ES_AUTOHSCROLL_VALUE;
                if (command->aux_value) style |= (LONG_PTR)WS_HSCROLL_VALUE;
                else style &= ~((LONG_PTR)WS_HSCROLL_VALUE);
            }
            if (old_wrap != new_wrap) {
                /* ES_AUTOHSCROLL is creation-sensitive for multiline EDITs,
                   notably under Wine. Keep the stable SpwObject id while replacing
                   only the native HWND so dynamic Spec wrapping is visually real. */
                recreate_status = spw_recreate_text_area(object, (DWORD)style);
                if (recreate_status != SPW_STATUS_OK) {
                    spw_complete_command(command, recreate_status);
                    return;
                }
            } else {
                SetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE, style);
                SetWindowPos(object->hwnd, NULL, 0, 0, 0, 0,
                    SWP_NOMOVE_VALUE | SWP_NOSIZE_VALUE | SWP_NOZORDER_VALUE |
                    SWP_NOACTIVATE_VALUE | SWP_FRAMECHANGED_VALUE);
            }
            spw_text_area_apply_format_rect(object);
            object->text_undo_enabled = command->x ? 1u : 0u;
            if (!object->text_undo_enabled)
                SendMessageW(object->hwnd, EM_EMPTYUNDOBUFFER_VALUE, 0, 0);
            InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_TEXT_REPLACE_SELECTION:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_multiline_text(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            conversion_status = spw_utf8_to_wide(command->text, command->text_length,
                g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
            if (conversion_status != SPW_STATUS_OK) {
                spw_complete_command(command, conversion_status);
                return;
            }
            conversion_status = spw_text_area_expand_newlines(g_control_text_wide, g_text_area_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
            if (conversion_status != SPW_STATUS_OK) {
                spw_complete_command(command, conversion_status);
                return;
            }
            SendMessageW(object->hwnd, EM_REPLACESEL_VALUE,
                object->text_undo_enabled ? TRUE_VALUE : FALSE_VALUE,
                (LPARAM)(uintptr_t)g_text_area_wide);
            if (!object->text_undo_enabled)
                SendMessageW(object->hwnd, EM_EMPTYUNDOBUFFER_VALUE, 0, 0);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_TEXT_COMMAND:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_multiline_text(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            switch (command->value) {
                case SPW_TEXT_COMMAND_COPY: SendMessageW(object->hwnd, WM_COPY_VALUE, 0, 0); break;
                case SPW_TEXT_COMMAND_CUT: SendMessageW(object->hwnd, WM_CUT_VALUE, 0, 0); break;
                case SPW_TEXT_COMMAND_PASTE: SendMessageW(object->hwnd, WM_PASTE_VALUE, 0, 0); break;
                case SPW_TEXT_COMMAND_UNDO: SendMessageW(object->hwnd, EM_UNDO_VALUE, 0, 0); break;
                case SPW_TEXT_COMMAND_CLEAR_UNDO: SendMessageW(object->hwnd, EM_EMPTYUNDOBUFFER_VALUE, 0, 0); break;
                default: spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT); return;
            }
            if (!object->text_undo_enabled)
                SendMessageW(object->hwnd, EM_EMPTYUNDOBUFFER_VALUE, 0, 0);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_TEXT_SCROLL_TO_LINE:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_multiline_text(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            {
                int32_t first_line = (int32_t)SendMessageW(object->hwnd, EM_GETFIRSTVISIBLELINE_VALUE, 0, 0);
                int32_t target_line = command->value < 0 ? 0 : command->value;
                SendMessageW(object->hwnd, EM_LINESCROLL_VALUE, 0, (LPARAM)(target_line - first_line));
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CODE_SET_LINE_NUMBERS:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command, spw_code_set_line_numbers_impl(object, command->value));
            return;

        case SPW_CMD_CODE_SET_STYLES:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command, spw_code_set_styles_impl(
                object, command->indexes, command->item_count,
                (uint32_t)command->x, (uint32_t)command->y));
            return;

        case SPW_CMD_CODE_SET_LINE_DECORATIONS:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command, spw_code_set_line_decorations_impl(
                object, command->indexes, command->item_count));
            return;

        case SPW_CMD_CONTROL_SET_SELECTED_INDEXES:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command,
                spw_list_set_selected_indexes(object, command->indexes, command->index_count));
            return;

        case SPW_CMD_CONTROL_GET_SELECTED_INDEXES:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_list_view(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->output_index_count = spw_list_get_selected_indexes(
                object, command->output_indexes, command->output_index_capacity);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_MULTIPLE_SELECTION:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command, spw_list_set_multiple_selection(object, command->value));
            return;

        case SPW_CMD_CONTROL_SET_HEADER:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command,
                spw_list_set_header(object, command->text, command->text_length, command->value));
            return;

        case SPW_CMD_CONTROL_SET_TABLE_COLUMNS:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command, spw_table_set_columns(
                object, command->text, command->text_length,
                command->column_widths, command->column_alignments, command->column_expandables,
                command->column_count, command->headers_visible, command->resizable));
            return;

        case SPW_CMD_CONTROL_SET_TABLE_CELLS:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command, spw_table_set_cells(
                object, command->text, command->text_length, command->row_count, command->column_count));
            return;

        case SPW_CMD_CONTROL_SET_TABLE_CELLS_WITH_IMAGES:
            object = spw_find_object_by_id(command->object_id);
            spw_complete_command(command, spw_table_apply_cells_with_images(
                object, command->bytes, command->byte_length, command->row_count, command->column_count));
            return;

        case SPW_CMD_CONTROL_ENSURE_VISIBLE:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL || !spw_control_is_list_view(object->kind)) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (command->value <= 0) {
                spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
                return;
            }
            SendMessageW(object->hwnd, LVM_ENSUREVISIBLE_VALUE, (WPARAM)(command->value - 1), (LPARAM)FALSE_VALUE);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_CONTROL_SET_TREE_NODES: {
            SpwObject *obj=spw_find_object_by_id(command->object_id); command->status=spw_tree_set_nodes(obj,command->text,command->text_length,command->parent_tokens,command->item_count); spw_complete_command(command,command->status); return; }
        case SPW_CMD_CONTROL_SET_TREE_NODES_WITH_IMAGES: {
            SpwObject *obj=spw_find_object_by_id(command->object_id); command->status=spw_tree_apply_nodes_with_images(obj,command->bytes,command->byte_length,command->parent_tokens,command->item_count); spw_complete_command(command,command->status); return; }
        case SPW_CMD_CONTROL_SET_TREE_SELECTED_TOKEN: {
            SpwObject *obj = spw_find_object_by_id(command->object_id);
            HTREEITEM item;
            if (obj == NULL || !spw_control_is_tree(obj->kind)) { spw_complete_command(command, SPW_STATUS_NOT_FOUND); return; }
            item = command->value <= 0 ? NULL : spw_tree_find_token(obj, command->value);
            obj->suppress_notifications += 1u;
            SendMessageW(spw_tree_hwnd(obj), TVM_SELECTITEM_VALUE, TVGN_CARET_VALUE, (LPARAM)(uintptr_t)item);
            obj->suppress_notifications -= 1u;
            if (obj->kind == SPW_CONTROL_TREE_COLUMN) spw_tree_column_sync_selection_from_tree(obj);
            spw_complete_command(command, SPW_STATUS_OK); return; }
        case SPW_CMD_CONTROL_GET_TREE_SELECTED_TOKEN: { SpwObject *obj=spw_find_object_by_id(command->object_id); int32_t v=spw_tree_selected_token(obj); if(v<0){spw_complete_command(command,v);return;} command->result32=v; spw_complete_command(command,SPW_STATUS_OK); return; }
        case SPW_CMD_CONTROL_TREE_SET_EXPANDED: {
            SpwObject *obj = spw_find_object_by_id(command->object_id);
            HTREEITEM item = spw_tree_find_token(obj, command->value);
            if (item == NULL) { spw_complete_command(command, SPW_STATUS_NOT_FOUND); return; }
            obj->suppress_notifications += 1u;
            SendMessageW(spw_tree_hwnd(obj), TVM_EXPAND_VALUE,
                command->aux_value ? TVE_EXPAND_VALUE : TVE_COLLAPSE_VALUE, (LPARAM)(uintptr_t)item);
            obj->suppress_notifications -= 1u;
            if (obj->kind == SPW_CONTROL_TREE_COLUMN) spw_tree_column_layout(obj);
            spw_complete_command(command, SPW_STATUS_OK); return; }
        case SPW_CMD_CONTROL_TREE_IS_EXPANDED: {
            SpwObject *obj=spw_find_object_by_id(command->object_id); HTREEITEM item=spw_tree_find_token(obj,command->value); TVITEMW q;
            if(item==NULL){spw_complete_command(command,SPW_STATUS_NOT_FOUND);return;}
            q.mask=0x0008u; q.hItem=item; q.state=0; q.stateMask=TVIS_EXPANDED_VALUE; q.pszText=NULL;q.cchTextMax=0;q.iImage=0;q.iSelectedImage=0;q.cChildren=0;q.lParam=0;
            if(!SendMessageW(spw_tree_hwnd(obj),TVM_GETITEMW_VALUE,0,(LPARAM)(uintptr_t)&q)){spw_complete_command(command,SPW_STATUS_WIN32_ERROR);return;}
            command->result32=(q.state&TVIS_EXPANDED_VALUE)?1:0; spw_complete_command(command,SPW_STATUS_OK); return; }
        case SPW_CMD_CONTROL_TREE_ENSURE_VISIBLE: {
            SpwObject *obj=spw_find_object_by_id(command->object_id); HTREEITEM item=spw_tree_find_token(obj,command->value);
            if(item==NULL){spw_complete_command(command,SPW_STATUS_NOT_FOUND);return;}
            if (obj->kind == SPW_CONTROL_TREE_COLUMN) {
                int32_t index = spw_tree_column_visible_index_for_item(obj, item);
                if (index >= 0) SendMessageW(obj->table_hwnd, LVM_ENSUREVISIBLE_VALUE, (WPARAM)index, (LPARAM)FALSE_VALUE);
                spw_tree_column_sync_scroll_from_table(obj);
            } else {
                SendMessageW(spw_tree_hwnd(obj),TVM_ENSUREVISIBLE_VALUE,0,(LPARAM)(uintptr_t)item);
            }
            spw_complete_command(command,SPW_STATUS_OK); return; }
        case SPW_CMD_CONTROL_SET_BOUNDS_BATCH: {
            uint32_t i;
            HDWP defer;
            if (command->bounds == NULL || command->bounds_count == 0u) {
                spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
                return;
            }
            defer = BeginDeferWindowPos((int32_t)command->bounds_count);
            if (defer == NULL) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            for (i = 0u; i < command->bounds_count; ++i) {
                const SpwBounds *entry = &command->bounds[i];
                object = spw_find_object_by_id(entry->object_id);
                if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                    EndDeferWindowPos(defer);
                    spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                    return;
                }
                defer = DeferWindowPos(defer, object->hwnd, NULL,
                    entry->x, entry->y, entry->width,
                    spw_control_is_drop_list(object->kind) && entry->height < 160 ? 160 : entry->height,
                    SWP_NOZORDER_VALUE | SWP_NOACTIVATE_VALUE |
                    (object->kind == SPW_CONTROL_FRAME ? SWP_NOCOPYBITS_VALUE : 0u));
                if (defer == NULL) {
                    spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                    return;
                }
                if (spw_control_is_list(object->kind))
                    SendMessageW(object->hwnd, LVM_SETCOLUMNWIDTH_VALUE, 0u,
                                 (LPARAM)(entry->width > 4 ? entry->width - 4 : 1));
                else if (spw_control_is_table(object->kind))
                    spw_table_apply_column_widths(object, entry->width);
            }
            if (!EndDeferWindowPos(defer)) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            /* Wine and some native themes may preserve copied child-window pixels
             * across deferred moves. Repaint each moved control explicitly so a
             * dynamic relayout cannot leave stale captions or icon fragments. */
            for (i = 0u; i < command->bounds_count; ++i) {
                object = spw_find_object_by_id(command->bounds[i].object_id);
                if (object != NULL && object->type == SPW_OBJECT_CONTROL) {
                    if (object->kind == SPW_CONTROL_TEXT_AREA)
                        spw_text_area_apply_format_rect(object);
                    InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
                    UpdateWindow(object->hwnd);
                }
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_CONTROL_SET_Z_ORDER: {
            uint32_t i;
            HWND expected_parent = NULL;
            if (command->object_ids == NULL || command->object_id_count == 0u) {
                spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
                return;
            }
            for (i = 0u; i < command->object_id_count; ++i) {
                object = spw_find_object_by_id(command->object_ids[i]);
                if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                    spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                    return;
                }
                if (i == 0u)
                    expected_parent = GetParent(object->hwnd);
                else if (GetParent(object->hwnd) != expected_parent) {
                    spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
                    return;
                }
            }
            /* The input order is bottom-to-top. Bringing each HWND to the top in
             * sequence yields exactly that relative ordering while leaving bounds
             * and activation unchanged. */
            for (i = 0u; i < command->object_id_count; ++i) {
                object = spw_find_object_by_id(command->object_ids[i]);
                if (!SetWindowPos(object->hwnd, NULL, 0, 0, 0, 0,
                                  SWP_NOMOVE_VALUE | SWP_NOSIZE_VALUE | SWP_NOACTIVATE_VALUE)) {
                    spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                    return;
                }
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_SCROLL_VIEW_CONFIGURE: {
            SpwObject *content;
            object = spw_find_object_by_id(command->object_id);
            content = spw_find_object_by_id(command->parent_id);
            if (object == NULL || object->kind != SPW_CONTROL_SCROLL_VIEWPORT ||
                content == NULL || content->kind != SPW_CONTROL_SCROLL_CONTENT ||
                content->parent_id != object->id) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            object->scroll_content_id = content->id;
            object->scroll_content_width = command->width > 0 ? command->width : 1;
            object->scroll_content_height = command->height > 0 ? command->height : 1;
            if (command->x < (int32_t)SPW_SCROLL_POLICY_DISABLED ||
                command->x > (int32_t)SPW_SCROLL_POLICY_ALWAYS ||
                command->y < (int32_t)SPW_SCROLL_POLICY_DISABLED ||
                command->y > (int32_t)SPW_SCROLL_POLICY_ALWAYS) {
                spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
                return;
            }
            object->scroll_h_policy = (uint32_t)command->x;
            object->scroll_v_policy = (uint32_t)command->y;
            object->scroll_h_enabled = command->x != (int32_t)SPW_SCROLL_POLICY_DISABLED ? 1u : 0u;
            object->scroll_v_enabled = command->y != (int32_t)SPW_SCROLL_POLICY_DISABLED ? 1u : 0u;
            spw_complete_command(command, spw_scroll_view_apply(object));
            return;
        }

        case SPW_CMD_SCROLL_VIEW_GET_PAGE_EXTENT: {
            RECT client;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->kind != SPW_CONTROL_SCROLL_VIEWPORT) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (!GetClientRect(object->hwnd, &client)) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            command->width = client.right - client.left;
            command->height = client.bottom - client.top;
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_SCROLL_VIEW_SET_POSITION:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->kind != SPW_CONTROL_SCROLL_VIEWPORT) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            object->scroll_x = command->x;
            object->scroll_y = command->y;
            spw_complete_command(command, spw_scroll_view_apply(object));
            return;

        case SPW_CMD_PANED_CONFIGURE: {
            SpwObject *first;
            SpwObject *second;
            object = spw_find_object_by_id(command->object_id);
            first = spw_find_object_by_id(command->parent_id);
            second = spw_find_object_by_id(command->secondary_id);
            if (object == NULL || object->kind != SPW_CONTROL_PANED ||
                first == NULL || first->kind != SPW_CONTROL_PANED_CONTENT || first->parent_id != object->id ||
                second == NULL || second->kind != SPW_CONTROL_PANED_CONTENT || second->parent_id != object->id) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            {
                int was_both_present = object->paned_first_present && object->paned_second_present;

                object->paned_first_content_id = first->id;
                object->paned_second_content_id = second->id;
                object->paned_vertical = command->kind ? 1u : 0u;
                object->paned_first_present = command->x ? 1u : 0u;
                object->paned_second_present = command->y ? 1u : 0u;
                object->paned_first_resize = command->resizable ? 1u : 0u;
                object->paned_second_resize = command->headers_visible ? 1u : 0u;
                object->paned_first_min = command->width > 0 ? command->width : 0;
                object->paned_second_min = command->height > 0 ? command->height : 0;
                if (!object->paned_initialized ||
                    (!was_both_present && object->paned_first_present && object->paned_second_present)) {
                    object->paned_position = command->value >= 0 ? command->value : 0;
                    object->paned_initialized = 1u;
                    object->paned_last_main_extent = spw_paned_main_extent(object);
                }
            }
            spw_complete_command(command, spw_paned_apply(object, 0));
            return;
        }

        case SPW_CMD_PANED_GET_POSITION:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->kind != SPW_CONTROL_PANED) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = object->paned_position;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_NOTEBOOK_GET_CONTENT_RECT: {
            RECT rect;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->kind != SPW_CONTROL_NOTEBOOK || object->hwnd == NULL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (!GetClientRect(object->hwnd, &rect)) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            SendMessageW(object->hwnd, TCM_ADJUSTRECT_VALUE, (WPARAM)FALSE_VALUE,
                         (LPARAM)(uintptr_t)&rect);
            command->x = rect.left;
            command->y = rect.top;
            command->width = rect.right - rect.left;
            command->height = rect.bottom - rect.top;
            if (command->width < 1) command->width = 1;
            if (command->height < 1) command->height = 1;
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_CONTROL_DESTROY: {
            uint64_t dead_id;
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_CONTROL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            dead_id = object->id;
            if (!DestroyWindow(object->hwnd)) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            object = spw_find_object_by_id(dead_id);
            if (object != NULL) {
                spw_unregister_children_of(dead_id);
                spw_unregister_object(object);
                spw_push_event(SPW_EVENT_DESTROYED, dead_id, 0, 0);
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_CONTROL_MEASURE: {
            SpwObject *parent;
            HDC dc;
            HGDIOBJ old_font;
            SIZE size;
            parent = spw_find_object_by_id(command->parent_id);
            if (parent == NULL || parent->hwnd == NULL) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            if (!spw_control_kind_is_valid(command->kind)) {
                spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
                return;
            }
            conversion_status = spw_utf8_to_wide(command->text, command->text_length,
                                                  wide_text, SPW_MAX_TITLE_WCHARS);
            if (conversion_status != SPW_STATUS_OK) {
                spw_complete_command(command, conversion_status);
                return;
            }
            dc = GetDC(parent->hwnd);
            if (dc == NULL) {
                spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                return;
            }
            old_font = SelectObject(dc, GetStockObject(DEFAULT_GUI_FONT_VALUE));
            size.cx = 0; size.cy = 0;
            if (wide_text[0] != 0) {
                int32_t length = 0;
                while (wide_text[length] != 0 && length < SPW_MAX_TITLE_WCHARS - 1) ++length;
                if (!GetTextExtentPoint32W(dc, wide_text, length, &size)) {
                    if (old_font != NULL) SelectObject(dc, old_font);
                    ReleaseDC(parent->hwnd, dc);
                    spw_complete_command(command, SPW_STATUS_WIN32_ERROR);
                    return;
                }
            } else {
                size.cx = 0;
                size.cy = 16;
            }
            if (old_font != NULL) SelectObject(dc, old_font);
            ReleaseDC(parent->hwnd, dc);
            if (command->kind == SPW_CONTROL_LABEL) {
                command->width = size.cx + 4;
                command->height = size.cy + 4;
            } else if (command->kind == SPW_CONTROL_FRAME) {
                command->width = size.cx + 24;
                if (command->width < 24) command->width = 24;
                command->height = size.cy + 8;
                if (command->height < 22) command->height = 22;
            } else if (command->kind == SPW_CONTROL_BUTTON) {
                command->width = size.cx + 24;
                if (command->width < 75) command->width = 75;
                command->height = size.cy + 12;
                if (command->height < 28) command->height = 28;
            } else if (command->kind == SPW_CONTROL_CHECKBOX ||
                       command->kind == SPW_CONTROL_RADIOBUTTON ||
                       command->kind == SPW_CONTROL_TOGGLE_BUTTON) {
                command->width = size.cx + 24;
                if (command->width < 24) command->width = 24;
                command->height = size.cy + 8;
                if (command->height < 22) command->height = 22;
            } else if (command->kind == SPW_CONTROL_SWITCH) {
                command->width = 46;
                command->height = 24;
            } else if (command->kind == SPW_CONTROL_SPINNER) {
                command->width = 32;
                command->height = 32;
            } else if (command->kind == SPW_CONTROL_LINK) {
                command->width = size.cx + 6;
                if (command->width < 12) command->width = 12;
                command->height = size.cy + 6;
                if (command->height < 22) command->height = 22;
            } else if (command->kind == SPW_CONTROL_TEXT_INPUT) {
                /* A single-line EDIT should not grow with its current text.
                   Its natural width is a form-field policy; only height depends
                   on the selected GUI font. */
                command->width = 120;
                command->height = size.cy + 10;
                if (command->height < 24) command->height = 24;
            } else if (command->kind == SPW_CONTROL_TEXT_AREA) {
                command->width = 320;
                command->height = 160;
            } else if (command->kind == SPW_CONTROL_CODE) {
                command->width = 520;
                command->height = 320;
            } else if (command->kind == SPW_CONTROL_DROP_LIST) {
                /* DropList measurement text is the widest candidate chosen by
                   the Pharo adapter. Reserve room for the native arrow button. */
                command->width = size.cx + 32;
                if (command->width < 80) command->width = 80;
                command->height = size.cy + 10;
                if (command->height < 24) command->height = 24;
            } else if (command->kind == SPW_CONTROL_SLIDER_HORIZONTAL) {
                command->width = 160;
                command->height = 32;
            } else if (command->kind == SPW_CONTROL_SLIDER_VERTICAL) {
                command->width = 32;
                command->height = 160;
            } else if (command->kind == SPW_CONTROL_LIST) {
                command->width = 180;
                command->height = 120;
            } else if (command->kind == SPW_CONTROL_TABLE) {
                command->width = 320;
                command->height = 160;
            } else if (command->kind == SPW_CONTROL_TREE) {
                command->width = 240;
                command->height = 180;
            } else if (command->kind == SPW_CONTROL_TREE_COLUMN) {
                command->width = 420;
                command->height = 200;
            } else if (command->kind == SPW_CONTROL_IMAGE) {
                command->width = 64;
                command->height = 64;
            } else {
                command->width = 160;
                command->height = 22;
            }
            spw_complete_command(command, SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_BARRIER:
            spw_push_event(SPW_EVENT_BARRIER, 0u, (int64_t)command->object_id, 0);
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_BEEP:
            command->result32 = MessageBeep((UINT)command->value) ? 1 : 0;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_SHOW_MESSAGE: {
            HWND owner = spw_dialog_owner_hwnd(command->object_id);
            UINT flags = MB_OK_VALUE | MB_SETFOREGROUND_VALUE;
            conversion_status = spw_utf8_to_wide(command->text, command->text_length,
                                                  g_message_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
            if (conversion_status != SPW_STATUS_OK) { spw_complete_command(command, conversion_status); return; }
            conversion_status = spw_utf8_to_wide(command->text2, command->text2_length,
                                                  g_dialog_title_wide, SPW_MAX_TITLE_WCHARS);
            if (conversion_status != SPW_STATUS_OK) { spw_complete_command(command, conversion_status); return; }
            flags |= command->value == SPW_MESSAGE_ERROR ? MB_ICONERROR_VALUE : MB_ICONINFORMATION_VALUE;
            command->result32 = MessageBoxW(owner, g_message_wide, g_dialog_title_wide, flags);
            spw_complete_command(command, command->result32 == 0 ? SPW_STATUS_WIN32_ERROR : SPW_STATUS_OK);
            return;
        }

        case SPW_CMD_FILE_DIALOG:
            spw_complete_command(command, spw_run_file_dialog(command));
            return;

        case SPW_CMD_SET_WAIT_CURSOR:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            object->wait_cursor = command->value ? 1u : 0u;
            SetCursor(LoadCursorW(NULL, object->wait_cursor ? IDC_WAIT_VALUE : IDC_ARROW_VALUE));
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_GET_WAIT_CURSOR:
            object = spw_find_object_by_id(command->object_id);
            if (object == NULL || object->type != SPW_OBJECT_WINDOW) {
                spw_complete_command(command, SPW_STATUS_NOT_FOUND);
                return;
            }
            command->result32 = (object->wait_cursor &&
                GetCursor() == LoadCursorW(NULL, IDC_WAIT_VALUE)) ? 1 : 0;
            spw_complete_command(command, SPW_STATUS_OK);
            return;

        case SPW_CMD_SHUTDOWN:
            spw_destroy_all_windows();
            spw_complete_command(command, SPW_STATUS_OK);
            PostQuitMessage(0);
            return;

        default:
            spw_complete_command(command, SPW_STATUS_BAD_ARGUMENT);
            return;
    }
}

