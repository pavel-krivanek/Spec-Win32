/* Object registry ------------------------------------------------------- */
static SpwObject *spw_find_object_by_id(uint64_t id) {
    uint32_t i;
    for (i = 0u; i < SPW_OBJECT_CAPACITY; ++i) {
        if (g_objects[i].alive && g_objects[i].id == id)
            return &g_objects[i];
    }
    return NULL;
}

static SpwObject *spw_find_object_by_hwnd(HWND hwnd) {
    uint32_t i;
    for (i = 0u; i < SPW_OBJECT_CAPACITY; ++i) {
        if (g_objects[i].alive && g_objects[i].hwnd == hwnd)
            return &g_objects[i];
    }
    return NULL;
}

static SpwObject *spw_register_object(HWND hwnd, uint32_t type, uint32_t kind, uint64_t parent_id) {
    uint32_t i;
    for (i = 0u; i < SPW_OBJECT_CAPACITY; ++i) {
        if (!g_objects[i].alive) {
            g_objects[i].id = atomic_fetch_add_explicit(&g_next_id, 1u, memory_order_relaxed);
            g_objects[i].parent_id = parent_id;
            g_objects[i].hwnd = hwnd;
            g_objects[i].tree_hwnd = NULL;
            g_objects[i].table_hwnd = NULL;
            g_objects[i].type = type;
            g_objects[i].kind = kind;
            g_objects[i].alive = 1u;
            g_objects[i].suppress_notifications = 0u;
            g_objects[i].normalized_value = 0;
            g_objects[i].indeterminate = 0u;
            g_objects[i].component_row_selected = 0u;
            g_objects[i].table_column_count = 0u;
            g_objects[i].table_headers_visible = 1u;
            g_objects[i].mouse_inside = 0u;
            g_objects[i].mouse_x = 0;
            g_objects[i].mouse_y = 0;
            g_objects[i].tooltip_hwnd = NULL;
            g_objects[i].tooltip_text[0] = 0;
            g_objects[i].placeholder_text[0] = 0;
            g_objects[i].image_bitmap = NULL;
            g_objects[i].button_image_list = NULL;
            g_objects[i].list_image_list = NULL;
            g_objects[i].tree_column_tree_image_list = NULL;
            g_objects[i].tree_column_table_image_list = NULL;
            g_objects[i].image_width = 0;
            g_objects[i].image_height = 0;
            g_objects[i].image_auto_scale = 0u;
            g_objects[i].text_undo_enabled = 1u;
            g_objects[i].popup_window = 0u;
            g_objects[i].popup_autohide = 0u;
            g_objects[i].scroll_content_id = 0u;
            g_objects[i].scroll_x = 0;
            g_objects[i].scroll_y = 0;
            g_objects[i].scroll_content_width = 0;
            g_objects[i].scroll_content_height = 0;
            g_objects[i].scroll_h_enabled = 1u;
            g_objects[i].scroll_v_enabled = 1u;
            g_objects[i].paned_first_content_id = 0u;
            g_objects[i].paned_second_content_id = 0u;
            g_objects[i].paned_position = 0;
            g_objects[i].paned_last_main_extent = 0;
            g_objects[i].paned_first_min = 0;
            g_objects[i].paned_second_min = 0;
            g_objects[i].paned_drag_offset = 0;
            g_objects[i].paned_vertical = 0u;
            g_objects[i].paned_first_present = 0u;
            g_objects[i].paned_second_present = 0u;
            g_objects[i].paned_first_resize = 1u;
            g_objects[i].paned_second_resize = 1u;
            g_objects[i].paned_dragging = 0u;
            g_objects[i].paned_initialized = 0u;
            return &g_objects[i];
        }
    }
    return NULL;
}

static uint32_t spw_object_slot(const SpwObject *object) {
    return (uint32_t)(object - g_objects);
}

static void spw_release_window_menu(SpwObject *object, int32_t detach) {
    uint32_t slot, i, count;
    HMENU menu;
    if (object == NULL || object->type != SPW_OBJECT_WINDOW)
        return;
    slot = spw_object_slot(object);
    if (slot >= SPW_OBJECT_CAPACITY)
        return;
    menu = g_window_menus[slot];
    count = g_window_menu_item_counts[slot];
    if (detach && object->hwnd != NULL) {
        SetMenu(object->hwnd, NULL);
        DrawMenuBar(object->hwnd);
    }
    if (menu != NULL)
        DestroyMenu(menu);
    g_window_menus[slot] = NULL;
    for (i = 0u; i < count && i < SPW_MAX_MENU_ITEMS; ++i) {
        if (g_window_menu_bitmaps[slot][i] != NULL) {
            DeleteObject((HGDIOBJ)g_window_menu_bitmaps[slot][i]);
            g_window_menu_bitmaps[slot][i] = NULL;
        }
    }
    g_window_menu_item_counts[slot] = 0u;
}

static void spw_unregister_object(SpwObject *object) {
    if (object == NULL)
        return;
    if (object->type == SPW_OBJECT_WINDOW)
        spw_release_window_menu(object, 0);
    if (object->tooltip_hwnd != NULL) {
        DestroyWindow(object->tooltip_hwnd);
        object->tooltip_hwnd = NULL;
    }
    object->tooltip_text[0] = 0;
    if (object->button_image_list != NULL) {
        ImageList_Destroy(object->button_image_list);
        object->button_image_list = NULL;
    }
    if (object->list_image_list != NULL) {
        ImageList_Destroy(object->list_image_list);
        object->list_image_list = NULL;
    }
    if (object->tree_column_tree_image_list != NULL) {
        ImageList_Destroy(object->tree_column_tree_image_list);
        object->tree_column_tree_image_list = NULL;
    }
    if (object->tree_column_table_image_list != NULL) {
        ImageList_Destroy(object->tree_column_table_image_list);
        object->tree_column_table_image_list = NULL;
    }
    if (object->image_bitmap != NULL) {
        DeleteObject((HGDIOBJ)object->image_bitmap);
        object->image_bitmap = NULL;
    }
    object->image_width = 0;
    object->image_height = 0;
    object->image_auto_scale = 0u;
    object->popup_window = 0u;
    object->popup_autohide = 0u;
    object->alive = 0u;
    object->parent_id = 0u;
    object->hwnd = NULL;
    object->tree_hwnd = NULL;
    object->table_hwnd = NULL;
    object->type = 0u;
    object->kind = 0u;
    object->suppress_notifications = 0u;
    object->normalized_value = 0;
    object->indeterminate = 0u;
    object->component_row_selected = 0u;
    object->table_column_count = 0u;
    object->table_headers_visible = 1u;
    object->mouse_inside = 0u;
    object->mouse_x = 0;
    object->mouse_y = 0;
    object->spinner_phase = 0u;
    object->id = 0u;
}

static void spw_unregister_children_of(uint64_t parent_id) {
    uint32_t i;
    for (i = 0u; i < SPW_OBJECT_CAPACITY; ++i) {
        if (g_objects[i].alive && g_objects[i].parent_id == parent_id) {
            uint64_t child_id = g_objects[i].id;
            spw_unregister_children_of(child_id);
            spw_unregister_object(&g_objects[i]);
            spw_push_event(SPW_EVENT_DESTROYED, child_id, 0, 0);
        }
    }
}

static int spw_control_kind_is_valid(uint32_t kind) {
    return kind >= SPW_CONTROL_LABEL && kind <= SPW_CONTROL_CODE;
}

static int32_t spw_tree_selected_token(SpwObject *object);
static void spw_tree_column_layout(SpwObject *object);
static void spw_tree_column_sync_scroll_from_table(SpwObject *object);
static int spw_tree_column_handle_notify(SpwObject *object, NMHDR *header, LPARAM lParam);
static int32_t spw_tree_column_create_children(SpwObject *object);
static LRESULT WINAPI spw_tree_column_child_subclass_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, uintptr_t subclass_id, uintptr_t ref_data);

static uint32_t spw_modifier_flags(void) {
    uint32_t flags = 0u;
    if ((GetKeyState((int32_t)VK_SHIFT_VALUE) & (SHORT)0x8000) != 0) flags |= SPW_MOD_SHIFT;
    if ((GetKeyState((int32_t)VK_CONTROL_VALUE) & (SHORT)0x8000) != 0) flags |= SPW_MOD_CONTROL;
    if ((GetKeyState((int32_t)VK_MENU_VALUE) & (SHORT)0x8000) != 0) flags |= SPW_MOD_ALT;
    if ((GetKeyState((int32_t)VK_LWIN_VALUE) & (SHORT)0x8000) != 0 ||
        (GetKeyState((int32_t)VK_RWIN_VALUE) & (SHORT)0x8000) != 0) flags |= SPW_MOD_META;
    return flags;
}

static SpwObject *spw_component_row_ancestor(SpwObject *object) {
    SpwObject *current;
    uint32_t depth = 0u;
    if (object == NULL) return NULL;
    if (object->kind == SPW_CONTROL_COMPONENT_ROW) return object;
    current = object;
    while (current != NULL && current->parent_id != 0u && depth++ < 32u) {
        current = spw_find_object_by_id(current->parent_id);
        if (current != NULL && current->type == SPW_OBJECT_CONTROL &&
            current->kind == SPW_CONTROL_COMPONENT_ROW)
            return current;
    }
    return NULL;
}

static int32_t spw_signed_low_word(LPARAM value) {
    return (int32_t)(int16_t)((uint32_t)(uintptr_t)value & 0xffffu);
}

static int32_t spw_signed_high_word(LPARAM value) {
    return (int32_t)(int16_t)(((uint32_t)(uintptr_t)value >> 16) & 0xffffu);
}

static int64_t spw_pack_mouse_argument2(int32_t y, uint32_t flags, uint32_t button) {
    uint64_t packed_flags = (uint64_t)((flags & 0xffffu) | ((button & 0xffu) << 16));
    return (int64_t)((packed_flags << 32) | (uint32_t)y);
}

static void spw_push_mouse_event(uint32_t type, SpwObject *object, LPARAM lParam, uint32_t button) {
    int32_t x = spw_signed_low_word(lParam);
    int32_t y = spw_signed_high_word(lParam);
    object->mouse_x = x;
    object->mouse_y = y;
    spw_push_event(type, object->id, (int64_t)x,
        spw_pack_mouse_argument2(y, spw_modifier_flags(), button));
}

static void spw_track_mouse_leave(SpwObject *object) {
    TRACKMOUSEEVENT track;
    if (object->mouse_inside) return;
    object->mouse_inside = 1u;
    track.cbSize = (DWORD)sizeof(TRACKMOUSEEVENT);
    track.dwFlags = TME_LEAVE_VALUE;
    track.hwndTrack = object->hwnd;
    track.dwHoverTime = 0u;
    TrackMouseEvent(&track);
    spw_push_event(SPW_EVENT_MOUSE_ENTER, object->id, (int64_t)object->mouse_x,
        spw_pack_mouse_argument2(object->mouse_y, spw_modifier_flags(), SPW_MOUSE_BUTTON_NONE));
}

static int spw_utf8_to_wide(const char *text, uint32_t text_length, WCHAR *buffer, int capacity);
static uint32_t spw_read_u32_le(const uint8_t *bytes);

static int32_t spw_show_context_menu(SpwObject *object, const char *blob,
                                         uint32_t blob_length, const int32_t *flags,
                                         uint32_t item_count, int32_t screen_x,
                                         int32_t screen_y, int32_t *out_selected) {
    HMENU menu;
    SpwObject *parent;
    HWND owner;
    uint32_t offset = 0u;
    uint32_t i;
    UINT selected;

    if (object == NULL || object->type != SPW_OBJECT_CONTROL || out_selected == NULL)
        return SPW_STATUS_NOT_FOUND;
    if (item_count > 0u && (blob == NULL || flags == NULL))
        return SPW_STATUS_BAD_ARGUMENT;

    menu = CreatePopupMenu();
    if (menu == NULL)
        return SPW_STATUS_WIN32_ERROR;

    for (i = 0u; i < item_count; ++i) {
        uint32_t item_length;
        UINT menu_flags;
        if (blob_length - offset < 4u) {
            DestroyMenu(menu);
            return SPW_STATUS_BAD_ARGUMENT;
        }
        item_length = spw_read_u32_le((const uint8_t *)blob + offset);
        offset += 4u;
        if (item_length > blob_length - offset) {
            DestroyMenu(menu);
            return SPW_STATUS_BAD_ARGUMENT;
        }
        if ((flags[i] & SPW_MENU_SEPARATOR) != 0) {
            if (!AppendMenuW(menu, MF_SEPARATOR_VALUE, 0u, NULL)) {
                DestroyMenu(menu);
                return SPW_STATUS_WIN32_ERROR;
            }
        } else {
            int32_t conversion_status = spw_utf8_to_wide(
                blob + offset, item_length, g_control_text_wide,
                SPW_MAX_CONTROL_TEXT_WCHARS);
            if (conversion_status != SPW_STATUS_OK) {
                DestroyMenu(menu);
                return conversion_status;
            }
            menu_flags = MF_STRING_VALUE;
            if ((flags[i] & SPW_MENU_ENABLED) == 0)
                menu_flags |= MF_GRAYED_VALUE;
            if ((flags[i] & SPW_MENU_CHECKED) != 0)
                menu_flags |= MF_CHECKED_VALUE;
            if (!AppendMenuW(menu, menu_flags, (uintptr_t)(i + 1u), g_control_text_wide)) {
                DestroyMenu(menu);
                return SPW_STATUS_WIN32_ERROR;
            }
        }
        offset += item_length;
    }
    if (offset != blob_length) {
        DestroyMenu(menu);
        return SPW_STATUS_BAD_ARGUMENT;
    }

    parent = spw_find_object_by_id(object->parent_id);
    owner = parent != NULL ? parent->hwnd : object->hwnd;
    SetForegroundWindow(owner);
    selected = TrackPopupMenu(menu,
        TPM_RETURNCMD_VALUE | TPM_RIGHTBUTTON_VALUE,
        screen_x, screen_y, 0, owner, NULL);
    DestroyMenu(menu);
    PostMessageW(owner, WM_NULL_VALUE, 0u, 0);
    *out_selected = (int32_t)selected;
    return SPW_STATUS_OK;
}



static int32_t spw_create_bgra_bitmap(const uint8_t *pixels, uint32_t byte_length,
                                      int32_t width, int32_t height, HBITMAP *out_bitmap) {
    BITMAPINFO info = {0};
    void *bits = NULL;
    HBITMAP bitmap;
    uint64_t expected;
    uint32_t i;

    if (out_bitmap == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    *out_bitmap = NULL;
    if (pixels == NULL || width <= 0 || height <= 0 || width > 16384 || height > 16384)
        return SPW_STATUS_BAD_ARGUMENT;
    expected = (uint64_t)(uint32_t)width * (uint64_t)(uint32_t)height * 4u;
    if (expected != (uint64_t)byte_length)
        return SPW_STATUS_BAD_ARGUMENT;

    info.bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1u;
    info.bmiHeader.biBitCount = 32u;
    info.bmiHeader.biCompression = BI_RGB_VALUE;
    info.bmiHeader.biSizeImage = byte_length;
    bitmap = CreateDIBSection(NULL, &info, DIB_RGB_COLORS_VALUE, &bits, NULL, 0u);
    if (bitmap == NULL || bits == NULL) {
        if (bitmap != NULL) DeleteObject((HGDIOBJ)bitmap);
        return SPW_STATUS_WIN32_ERROR;
    }
    for (i = 0u; i < byte_length; ++i)
        ((uint8_t *)bits)[i] = pixels[i];
    *out_bitmap = bitmap;
    return SPW_STATUS_OK;
}

/* Rich popup-menu blob. Each depth-first record is:
 * u32 parent-index (0=root, otherwise 1-based parent record),
 * u32 flags, u32 label-length, u32 icon-width, u32 icon-height,
 * u32 icon-byte-length, followed by UTF-8 label and premultiplied BGRA icon bytes.
 */
static int32_t spw_show_popup_menu(SpwObject *object, const uint8_t *blob,
                                   uint32_t blob_length, uint32_t item_count,
                                   int32_t screen_x, int32_t screen_y,
                                   int32_t *out_selected) {
    HMENU root = NULL;
    uint32_t root_count = 0u;
    uint32_t offset = 0u;
    uint32_t i;
    HWND owner = NULL;
    UINT selected = 0u;
    int32_t status = SPW_STATUS_OK;

    if (out_selected == NULL || item_count > SPW_MAX_MENU_ITEMS)
        return SPW_STATUS_BAD_ARGUMENT;
    for (i = 0u; i < SPW_MAX_MENU_ITEMS; ++i) {
        g_popup_child_menus[i] = NULL;
        g_popup_child_counts[i] = 0u;
        g_popup_bitmaps[i] = NULL;
    }
    if (item_count > 0u && blob == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    if (object != NULL) {
        SpwObject *parent;
        if (!object->alive)
            return SPW_STATUS_NOT_FOUND;
        if (object->type == SPW_OBJECT_CONTROL) {
            parent = spw_find_object_by_id(object->parent_id);
            owner = parent != NULL ? parent->hwnd : object->hwnd;
        } else if (object->type == SPW_OBJECT_WINDOW) {
            owner = object->hwnd;
        }
    }
    if (owner == NULL)
        owner = GetForegroundWindow();
    if (owner == NULL)
        return SPW_STATUS_NOT_FOUND;

    root = CreatePopupMenu();
    if (root == NULL)
        return SPW_STATUS_WIN32_ERROR;

    for (i = 0u; i < item_count; ++i) {
        uint32_t parent_index, flags, label_length, icon_width, icon_height, icon_length;
        HMENU target;
        uint32_t position;
        UINT menu_flags;
        uintptr_t command_or_submenu;
        HMENU submenu = NULL;

        if (blob_length - offset < 24u) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        parent_index = spw_read_u32_le(blob + offset); offset += 4u;
        flags = spw_read_u32_le(blob + offset); offset += 4u;
        label_length = spw_read_u32_le(blob + offset); offset += 4u;
        icon_width = spw_read_u32_le(blob + offset); offset += 4u;
        icon_height = spw_read_u32_le(blob + offset); offset += 4u;
        icon_length = spw_read_u32_le(blob + offset); offset += 4u;
        if (label_length > blob_length - offset) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        if (icon_length > blob_length - offset - label_length) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        if (parent_index > i) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        if (parent_index == 0u) {
            target = root;
            position = root_count++;
        } else {
            target = g_popup_child_menus[parent_index - 1u];
            if (target == NULL) { status = SPW_STATUS_BAD_ARGUMENT; break; }
            position = g_popup_child_counts[parent_index - 1u]++;
        }

        if ((flags & SPW_MENU_SEPARATOR) != 0u) {
            if (!AppendMenuW(target, MF_SEPARATOR_VALUE, 0u, NULL)) {
                status = SPW_STATUS_WIN32_ERROR; break;
            }
            offset += label_length + icon_length;
            continue;
        }

        status = spw_utf8_to_wide((const char *)(blob + offset), label_length,
                                  g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
        if (status != SPW_STATUS_OK) break;
        offset += label_length;

        menu_flags = MF_STRING_VALUE;
        if ((flags & SPW_MENU_ENABLED) == 0u) menu_flags |= MF_GRAYED_VALUE;
        if ((flags & SPW_MENU_CHECKED) != 0u) menu_flags |= MF_CHECKED_VALUE;
        if ((flags & SPW_MENU_SUBMENU) != 0u) {
            submenu = CreatePopupMenu();
            if (submenu == NULL) { status = SPW_STATUS_WIN32_ERROR; break; }
            g_popup_child_menus[i] = submenu;
            menu_flags |= MF_POPUP_VALUE;
            command_or_submenu = (uintptr_t)submenu;
        } else {
            command_or_submenu = (uintptr_t)(i + 1u);
        }
        if (!AppendMenuW(target, menu_flags, command_or_submenu, g_control_text_wide)) {
            if (submenu != NULL) DestroyMenu(submenu);
            g_popup_child_menus[i] = NULL;
            status = SPW_STATUS_WIN32_ERROR; break;
        }

        if (icon_length > 0u) {
            status = spw_create_bgra_bitmap(blob + offset, icon_length,
                                            (int32_t)icon_width, (int32_t)icon_height,
                                            &g_popup_bitmaps[i]);
            if (status != SPW_STATUS_OK) break;
            if (!SetMenuItemBitmaps(target, position, MF_BYPOSITION_VALUE,
                                    g_popup_bitmaps[i], g_popup_bitmaps[i])) {
                status = SPW_STATUS_WIN32_ERROR; break;
            }
        } else if (icon_width != 0u || icon_height != 0u) {
            status = SPW_STATUS_BAD_ARGUMENT; break;
        }
        offset += icon_length;
    }

    if (status == SPW_STATUS_OK && offset != blob_length)
        status = SPW_STATUS_BAD_ARGUMENT;
    if (status == SPW_STATUS_OK) {
        SetForegroundWindow(owner);
        selected = TrackPopupMenu(root, TPM_RETURNCMD_VALUE | TPM_RIGHTBUTTON_VALUE,
                                  screen_x, screen_y, 0, owner, NULL);
        PostMessageW(owner, WM_NULL_VALUE, 0u, 0);
        *out_selected = (int32_t)selected;
    }
    if (root != NULL) DestroyMenu(root);
    for (i = 0u; i < item_count; ++i)
        if (g_popup_bitmaps[i] != NULL) DeleteObject((HGDIOBJ)g_popup_bitmaps[i]);
    return status;
}


static int32_t spw_build_window_menu(const uint8_t *blob, uint32_t blob_length,
                                     uint32_t item_count, HBITMAP *bitmaps,
                                     HMENU *out_menu) {
    HMENU root = NULL;
    uint32_t root_count = 0u;
    uint32_t offset = 0u;
    uint32_t i;
    int32_t status = SPW_STATUS_OK;

    if (out_menu == NULL || bitmaps == NULL || item_count > SPW_MAX_MENU_ITEMS)
        return SPW_STATUS_BAD_ARGUMENT;
    *out_menu = NULL;
    for (i = 0u; i < SPW_MAX_MENU_ITEMS; ++i) {
        g_popup_child_menus[i] = NULL;
        g_popup_child_counts[i] = 0u;
        bitmaps[i] = NULL;
    }
    if (item_count > 0u && blob == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    root = CreateMenu();
    if (root == NULL)
        return SPW_STATUS_WIN32_ERROR;

    for (i = 0u; i < item_count; ++i) {
        uint32_t parent_index, flags, label_length, icon_width, icon_height, icon_length;
        HMENU target;
        uint32_t position;
        UINT menu_flags;
        uintptr_t command_or_submenu;
        HMENU submenu = NULL;

        if (blob_length - offset < 24u) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        parent_index = spw_read_u32_le(blob + offset); offset += 4u;
        flags = spw_read_u32_le(blob + offset); offset += 4u;
        label_length = spw_read_u32_le(blob + offset); offset += 4u;
        icon_width = spw_read_u32_le(blob + offset); offset += 4u;
        icon_height = spw_read_u32_le(blob + offset); offset += 4u;
        icon_length = spw_read_u32_le(blob + offset); offset += 4u;
        if (label_length > blob_length - offset) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        if (icon_length > blob_length - offset - label_length) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        if (parent_index > i) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        if (parent_index == 0u) {
            target = root;
            position = root_count++;
        } else {
            target = g_popup_child_menus[parent_index - 1u];
            if (target == NULL) { status = SPW_STATUS_BAD_ARGUMENT; break; }
            position = g_popup_child_counts[parent_index - 1u]++;
        }
        if ((flags & SPW_MENU_SEPARATOR) != 0u) {
            if (!AppendMenuW(target, MF_SEPARATOR_VALUE, 0u, NULL)) {
                status = SPW_STATUS_WIN32_ERROR; break;
            }
            offset += label_length + icon_length;
            continue;
        }
        status = spw_utf8_to_wide((const char *)(blob + offset), label_length,
                                  g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
        if (status != SPW_STATUS_OK) break;
        offset += label_length;
        menu_flags = MF_STRING_VALUE;
        if ((flags & SPW_MENU_ENABLED) == 0u) menu_flags |= MF_GRAYED_VALUE;
        if ((flags & SPW_MENU_CHECKED) != 0u) menu_flags |= MF_CHECKED_VALUE;
        if ((flags & SPW_MENU_SUBMENU) != 0u) {
            submenu = CreatePopupMenu();
            if (submenu == NULL) { status = SPW_STATUS_WIN32_ERROR; break; }
            g_popup_child_menus[i] = submenu;
            menu_flags |= MF_POPUP_VALUE;
            command_or_submenu = (uintptr_t)submenu;
        } else {
            command_or_submenu = (uintptr_t)(i + 1u);
        }
        if (!AppendMenuW(target, menu_flags, command_or_submenu, g_control_text_wide)) {
            if (submenu != NULL) DestroyMenu(submenu);
            g_popup_child_menus[i] = NULL;
            status = SPW_STATUS_WIN32_ERROR; break;
        }
        if (icon_length > 0u) {
            status = spw_create_bgra_bitmap(blob + offset, icon_length,
                                            (int32_t)icon_width, (int32_t)icon_height,
                                            &bitmaps[i]);
            if (status != SPW_STATUS_OK) break;
            if (!SetMenuItemBitmaps(target, position, MF_BYPOSITION_VALUE,
                                    bitmaps[i], bitmaps[i])) {
                status = SPW_STATUS_WIN32_ERROR; break;
            }
        } else if (icon_width != 0u || icon_height != 0u) {
            status = SPW_STATUS_BAD_ARGUMENT; break;
        }
        offset += icon_length;
    }
    if (status == SPW_STATUS_OK && offset != blob_length)
        status = SPW_STATUS_BAD_ARGUMENT;
    if (status != SPW_STATUS_OK) {
        if (root != NULL) DestroyMenu(root);
        for (i = 0u; i < item_count; ++i) {
            if (bitmaps[i] != NULL) {
                DeleteObject((HGDIOBJ)bitmaps[i]);
                bitmaps[i] = NULL;
            }
        }
        return status;
    }
    *out_menu = root;
    return SPW_STATUS_OK;
}

static int32_t spw_link_markup(const WCHAR *plain, WCHAR *out, uint32_t capacity) {
    static const WCHAR open_tag[] = { '<','a','>',0 };
    static const WCHAR close_tag[] = { '<','/','a','>',0 };
    static const WCHAR amp[] = { '&','a','m','p',';',0 };
    static const WCHAR lt[] = { '&','l','t',';',0 };
    static const WCHAR gt[] = { '&','g','t',';',0 };
    uint32_t out_index = 0u;
    uint32_t i;

#define SPW_APPEND_LITERAL(literal) \
    do { \
        for (i = 0u; (literal)[i] != 0; ++i) { \
            if (out_index + 1u >= capacity) return SPW_STATUS_CAPACITY; \
            out[out_index++] = (literal)[i]; \
        } \
    } while (0)

    if (plain == NULL || out == NULL || capacity < 8u)
        return SPW_STATUS_BAD_ARGUMENT;
    SPW_APPEND_LITERAL(open_tag);
    while (*plain != 0) {
        if (*plain == (WCHAR)'&') {
            SPW_APPEND_LITERAL(amp);
        } else if (*plain == (WCHAR)'<') {
            SPW_APPEND_LITERAL(lt);
        } else if (*plain == (WCHAR)'>') {
            SPW_APPEND_LITERAL(gt);
        } else {
            if (out_index + 1u >= capacity) return SPW_STATUS_CAPACITY;
            out[out_index++] = *plain;
        }
        ++plain;
    }
    SPW_APPEND_LITERAL(close_tag);
    out[out_index] = 0;
#undef SPW_APPEND_LITERAL
    return SPW_STATUS_OK;
}

static void spw_draw_spinner(SpwObject *object) {
    static const int8_t offsets[8][2] = {
        { 0, -10 }, { 7, -7 }, { 10, 0 }, { 7, 7 },
        { 0, 10 }, { -7, 7 }, { -10, 0 }, { -7, -7 }
    };
    RECT rect;
    HDC dc;
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;
    int32_t width;
    int32_t height;
    int32_t cx;
    int32_t cy;
    uint32_t i;

    if (object == NULL || object->hwnd == NULL || object->kind != SPW_CONTROL_SPINNER)
        return;
    if (!GetClientRect(object->hwnd, &rect))
        return;
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    if (width < 16 || height < 16)
        return;

    dc = GetDC(object->hwnd);
    if (dc == NULL)
        return;
    cx = width / 2;
    cy = height / 2;
    old_pen = SelectObject(dc, GetStockObject(NULL_PEN_VALUE));
    old_brush = SelectObject(dc, GetSysColorBrush(COLOR_3DSHADOW_VALUE));
    for (i = 0u; i < 8u; ++i) {
        int32_t x = cx + offsets[i][0];
        int32_t y = cy + offsets[i][1];
        HBRUSH brush = GetSysColorBrush(i == object->spinner_phase
            ? COLOR_HIGHLIGHT_VALUE
            : COLOR_3DSHADOW_VALUE);
        SelectObject(dc, brush);
        RoundRect(dc, x - 2, y - 2, x + 3, y + 3, 5, 5);
    }
    if (old_brush != NULL) SelectObject(dc, old_brush);
    if (old_pen != NULL) SelectObject(dc, old_pen);
    ReleaseDC(object->hwnd, dc);
}

static void spw_draw_switch(SpwObject *object) {
    RECT rect;
    HDC dc;
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;
    HBRUSH track_brush;
    HBRUSH thumb_brush;
    int32_t width;
    int32_t height;
    int32_t visual_width;
    int32_t track_height;
    int32_t track_top;
    int32_t radius;
    int32_t thumb_size;
    int32_t thumb_left;
    int checked;

    if (object == NULL || object->hwnd == NULL || object->kind != SPW_CONTROL_SWITCH)
        return;
    if (!GetClientRect(object->hwnd, &rect))
        return;
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    if (width <= 4 || height <= 4)
        return;
    visual_width = width > 46 ? 46 : width;

    dc = GetDC(object->hwnd);
    if (dc == NULL)
        return;
    checked = SendMessageW(object->hwnd, BM_GETCHECK_VALUE, 0, 0) == BST_CHECKED_VALUE;
    track_height = height - 6;
    if (track_height > 20) track_height = 20;
    if (track_height < 10) track_height = 10;
    track_top = (height - track_height) / 2;
    radius = track_height;
    track_brush = GetSysColorBrush(IsWindowEnabled(object->hwnd)
        ? (checked ? COLOR_HIGHLIGHT_VALUE : COLOR_3DSHADOW_VALUE)
        : COLOR_BTNFACE_VALUE);
    thumb_brush = GetSysColorBrush(COLOR_WINDOW_VALUE);
    old_pen = SelectObject(dc, GetStockObject(NULL_PEN_VALUE));
    old_brush = SelectObject(dc, track_brush);
    RoundRect(dc, 1, track_top, visual_width - 1, track_top + track_height,
              radius, radius);

    thumb_size = track_height - 4;
    if (thumb_size < 6) thumb_size = 6;
    thumb_left = checked ? visual_width - 3 - thumb_size : 3;
    SelectObject(dc, thumb_brush);
    RoundRect(dc, thumb_left, track_top + 2,
              thumb_left + thumb_size, track_top + 2 + thumb_size,
              thumb_size, thumb_size);
    if (old_brush != NULL) SelectObject(dc, old_brush);
    if (old_pen != NULL) SelectObject(dc, old_pen);
    ReleaseDC(object->hwnd, dc);
}

static int spw_edit_affordance_hit(SpwObject *object, LPARAM lParam, int *upper_half) {
    RECT rect;
    int32_t x;
    int32_t y;
    if (object == NULL || !GetClientRect(object->hwnd, &rect)) return 0;
    x = spw_signed_low_word(lParam);
    y = spw_signed_high_word(lParam);
    if (x < rect.right - SPW_EDIT_AFFORDANCE_WIDTH) return 0;
    if (upper_half != NULL) *upper_half = y < ((rect.bottom - rect.top) / 2);
    return 1;
}

static void spw_draw_number_affordance(SpwObject *object) {
    RECT rect;
    HDC dc;
    HGDIOBJ old_pen;
    int32_t left;
    int32_t middle;
    int32_t cx;
    if (object == NULL || !GetClientRect(object->hwnd, &rect)) return;
    dc = GetDC(object->hwnd);
    if (dc == NULL) return;
    old_pen = SelectObject(dc, GetStockObject(BLACK_PEN_VALUE));
    left = rect.right - SPW_EDIT_AFFORDANCE_WIDTH;
    middle = (rect.bottom - rect.top) / 2;
    cx = left + SPW_EDIT_AFFORDANCE_WIDTH / 2;
    MoveToEx(dc, left, rect.top + 1, NULL);
    LineTo(dc, left, rect.bottom - 1);
    MoveToEx(dc, left, middle, NULL);
    LineTo(dc, rect.right - 1, middle);
    MoveToEx(dc, cx - 3, middle - 3, NULL);
    LineTo(dc, cx, middle - 6);
    LineTo(dc, cx + 3, middle - 3);
    MoveToEx(dc, cx - 3, middle + 3, NULL);
    LineTo(dc, cx, middle + 6);
    LineTo(dc, cx + 3, middle + 3);
    if (old_pen != NULL) SelectObject(dc, old_pen);
    ReleaseDC(object->hwnd, dc);
}

static void spw_draw_search_affordance(SpwObject *object) {
    RECT rect;
    HDC dc;
    HGDIOBJ old_pen;
    int32_t left;
    int32_t cy;
    int32_t cx;
    if (object == NULL || GetWindowTextLengthW(object->hwnd) <= 0) return;
    if (!GetClientRect(object->hwnd, &rect)) return;
    dc = GetDC(object->hwnd);
    if (dc == NULL) return;
    old_pen = SelectObject(dc, GetStockObject(BLACK_PEN_VALUE));
    left = rect.right - SPW_EDIT_AFFORDANCE_WIDTH;
    cx = left + SPW_EDIT_AFFORDANCE_WIDTH / 2;
    cy = (rect.bottom - rect.top) / 2;
    MoveToEx(dc, cx - 3, cy - 3, NULL);
    LineTo(dc, cx + 4, cy + 4);
    MoveToEx(dc, cx + 3, cy - 3, NULL);
    LineTo(dc, cx - 4, cy + 4);
    if (old_pen != NULL) SelectObject(dc, old_pen);
    ReleaseDC(object->hwnd, dc);
}

static int32_t spw_refresh_button_image_list(SpwObject *object) {
    BUTTON_IMAGELIST config = {0};
    HIMAGELIST new_list = NULL;
    int32_t add_index;

    if (object == NULL || object->hwnd == NULL)
        return SPW_STATUS_NOT_FOUND;
    if (object->kind != SPW_CONTROL_BUTTON && object->kind != SPW_CONTROL_TOGGLE_BUTTON)
        return SPW_STATUS_OK;

    if (object->image_bitmap != NULL && object->image_width > 0 && object->image_height > 0) {
        new_list = ImageList_Create(object->image_width, object->image_height,
            ILC_COLOR32_VALUE | ILC_MASK_VALUE, 1, 1);
        if (new_list == NULL)
            return SPW_STATUS_WIN32_ERROR;
        add_index = ImageList_Add(new_list, object->image_bitmap, NULL);
        if (add_index < 0) {
            ImageList_Destroy(new_list);
            return SPW_STATUS_WIN32_ERROR;
        }
        config.himl = new_list;
        config.margin.left = 4;
        config.margin.top = 2;
        config.margin.right = 4;
        config.margin.bottom = 2;
        config.uAlign = GetWindowTextLengthW(object->hwnd) > 0
            ? BUTTON_IMAGELIST_ALIGN_LEFT_VALUE
            : BUTTON_IMAGELIST_ALIGN_CENTER_VALUE;
    }

    SendMessageW(object->hwnd, BCM_SETIMAGELIST_VALUE, 0u, (LPARAM)(uintptr_t)&config);
    if (object->button_image_list != NULL)
        ImageList_Destroy(object->button_image_list);
    object->button_image_list = new_list;
    InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
    return SPW_STATUS_OK;
}

static int32_t spw_set_control_image_bgra(SpwObject *object, const uint8_t *pixels,
                                              uint32_t byte_length, int32_t width, int32_t height) {
    HBITMAP bitmap;
    BITMAPINFO info = {0};
    void *bits = NULL;
    uint64_t expected;
    uint32_t i;

    if (object == NULL || object->type != SPW_OBJECT_CONTROL)
        return SPW_STATUS_NOT_FOUND;
    if (pixels == NULL || byte_length == 0u || width == 0 || height == 0) {
        if (!(pixels == NULL && byte_length == 0u && width == 0 && height == 0))
            return SPW_STATUS_BAD_ARGUMENT;
        if (object->button_image_list != NULL) {
            BUTTON_IMAGELIST config = {0};
            SendMessageW(object->hwnd, BCM_SETIMAGELIST_VALUE, 0u, (LPARAM)(uintptr_t)&config);
            ImageList_Destroy(object->button_image_list);
            object->button_image_list = NULL;
        }
        if (object->image_bitmap != NULL) {
            DeleteObject((HGDIOBJ)object->image_bitmap);
            object->image_bitmap = NULL;
        }
        object->image_width = 0;
        object->image_height = 0;
        InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
        return SPW_STATUS_OK;
    }
    if (width < 0 || height < 0 || width > 16384 || height > 16384)
        return SPW_STATUS_BAD_ARGUMENT;
    expected = (uint64_t)(uint32_t)width * (uint64_t)(uint32_t)height * 4u;
    if (expected != (uint64_t)byte_length)
        return SPW_STATUS_BAD_ARGUMENT;

    info.bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height; /* top-down DIB: row 0 is Form row 0 */
    info.bmiHeader.biPlanes = 1u;
    info.bmiHeader.biBitCount = 32u;
    info.bmiHeader.biCompression = BI_RGB_VALUE;
    info.bmiHeader.biSizeImage = byte_length;
    bitmap = CreateDIBSection(NULL, &info, DIB_RGB_COLORS_VALUE, &bits, NULL, 0u);
    if (bitmap == NULL || bits == NULL) {
        if (bitmap != NULL) DeleteObject((HGDIOBJ)bitmap);
        return SPW_STATUS_WIN32_ERROR;
    }
    for (i = 0u; i < byte_length; ++i)
        ((uint8_t *)bits)[i] = pixels[i];

    if (object->image_bitmap != NULL)
        DeleteObject((HGDIOBJ)object->image_bitmap);
    object->image_bitmap = bitmap;
    object->image_width = width;
    object->image_height = height;
    if (object->kind == SPW_CONTROL_BUTTON || object->kind == SPW_CONTROL_TOGGLE_BUTTON) {
        int32_t status = spw_refresh_button_image_list(object);
        if (status != SPW_STATUS_OK)
            return status;
    }
    InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
    return SPW_STATUS_OK;
}

static void spw_draw_control_image(SpwObject *object) {
    RECT rect;
    HDC dc;
    HDC memory_dc;
    HGDIOBJ old_bitmap;
    BLENDFUNCTION blend;
    int32_t client_width;
    int32_t client_height;
    int32_t draw_width;
    int32_t draw_height;
    int32_t x;
    int32_t y;

    if (object == NULL || object->hwnd == NULL || object->image_bitmap == NULL ||
        object->image_width <= 0 || object->image_height <= 0)
        return;
    if (!GetClientRect(object->hwnd, &rect)) return;
    client_width = rect.right - rect.left;
    client_height = rect.bottom - rect.top;
    if (client_width <= 0 || client_height <= 0) return;

    draw_width = object->image_width;
    draw_height = object->image_height;
    if (object->image_auto_scale) {
        if ((int64_t)object->image_width * client_height >
            (int64_t)object->image_height * client_width) {
            draw_width = client_width;
            draw_height = (int32_t)(((int64_t)object->image_height * client_width) /
                                    object->image_width);
        } else {
            draw_height = client_height;
            draw_width = (int32_t)(((int64_t)object->image_width * client_height) /
                                   object->image_height);
        }
        if (draw_width < 1) draw_width = 1;
        if (draw_height < 1) draw_height = 1;
    }
    x = (client_width - draw_width) / 2;
    y = (client_height - draw_height) / 2;

    dc = GetDC(object->hwnd);
    if (dc == NULL) return;
    memory_dc = CreateCompatibleDC(dc);
    if (memory_dc == NULL) {
        ReleaseDC(object->hwnd, dc);
        return;
    }
    old_bitmap = SelectObject(memory_dc, (HGDIOBJ)object->image_bitmap);
    blend.BlendOp = AC_SRC_OVER_VALUE;
    blend.BlendFlags = 0u;
    blend.SourceConstantAlpha = 255u;
    blend.AlphaFormat = AC_SRC_ALPHA_VALUE;
    AlphaBlend(dc, x, y, draw_width, draw_height,
               memory_dc, 0, 0, object->image_width, object->image_height, blend);
    if (old_bitmap != NULL) SelectObject(memory_dc, old_bitmap);
    DeleteDC(memory_dc);
    ReleaseDC(object->hwnd, dc);
}

static LRESULT WINAPI spw_control_subclass_proc(HWND hwnd, UINT message, WPARAM wParam,
                                                  LPARAM lParam, uintptr_t subclass_id,
                                                  uintptr_t ref_data) {
    SpwObject *object = spw_find_object_by_hwnd(hwnd);
    (void)ref_data;

    if (object != NULL) {
        if (object->kind == SPW_CONTROL_TREE_COLUMN && message == WM_SIZE_VALUE)
            spw_tree_column_layout(object);
        if (object->kind == SPW_CONTROL_CODE && message == WM_SIZE_VALUE)
            spw_code_apply_margin(object);
        if (object->kind == SPW_CONTROL_TREE_COLUMN && message == WM_NOTIFY_VALUE) {
            NMHDR *header = (NMHDR *)(uintptr_t)lParam;
            if (header != NULL && spw_tree_column_handle_notify(object, header, lParam))
                return 0;
        }
        if (object->kind == SPW_CONTROL_COMPONENT_ROW && message == WM_ERASEBKGND_VALUE) {
            RECT client;
            if (GetClientRect(hwnd, &client)) {
                HBRUSH brush = GetSysColorBrush(object->component_row_selected
                    ? COLOR_HIGHLIGHT_VALUE : COLOR_WINDOW_VALUE);
                FillRect((HDC)(uintptr_t)wParam, &client, brush);
                return 1;
            }
        }
        if (object->kind == SPW_CONTROL_FRAME &&
            (message == WM_COMMAND_VALUE || message == WM_NOTIFY_VALUE ||
             message == WM_HSCROLL_VALUE || message == WM_VSCROLL_VALUE)) {
            HWND parent = GetParent(hwnd);
            if (parent != NULL)
                return SendMessageW(parent, message, wParam, lParam);
        }
        if (object->kind == SPW_CONTROL_FRAME && message == WM_ERASEBKGND_VALUE) {
            RECT client;
            HBRUSH brush = NULL;
            HWND parent = GetParent(hwnd);
            if (GetClientRect(hwnd, &client)) {
                if (parent != NULL)
                    brush = (HBRUSH)(uintptr_t)SendMessageW(parent, WM_CTLCOLORBTN_VALUE, wParam, (LPARAM)(uintptr_t)hwnd);
                if (brush == NULL) brush = GetSysColorBrush(COLOR_BTNFACE_VALUE);
                FillRect((HDC)(uintptr_t)wParam, &client, brush);
                return 1;
            }
        }
        if (message == WM_PAINT_VALUE && object->kind == SPW_CONTROL_SWITCH) {
            LRESULT result = DefSubclassProc(hwnd, message, wParam, lParam);
            spw_draw_switch(object);
            return result;
        }
        if (message == WM_PAINT_VALUE && object->kind == SPW_CONTROL_SPINNER) {
            LRESULT result = DefSubclassProc(hwnd, message, wParam, lParam);
            spw_draw_spinner(object);
            return result;
        }
        if (message == WM_PAINT_VALUE && object->kind == SPW_CONTROL_NUMBER_INPUT) {
            LRESULT result = DefSubclassProc(hwnd, message, wParam, lParam);
            spw_draw_number_affordance(object);
            return result;
        }
        if (message == WM_PAINT_VALUE && object->kind == SPW_CONTROL_SEARCH_INPUT) {
            LRESULT result = DefSubclassProc(hwnd, message, wParam, lParam);
            spw_draw_search_affordance(object);
            return result;
        }
        if (message == WM_PAINT_VALUE && object->kind == SPW_CONTROL_IMAGE) {
            LRESULT result = DefSubclassProc(hwnd, message, wParam, lParam);
            spw_draw_control_image(object);
            return result;
        }
        if (message == WM_PAINT_VALUE && object->kind == SPW_CONTROL_CODE) {
            LRESULT result = DefSubclassProc(hwnd, message, wParam, lParam);
            spw_code_draw_gutter(object);
            return result;
        }
        if (message == WM_TIMER_VALUE && object->kind == SPW_CONTROL_SPINNER) {
            object->spinner_phase = (object->spinner_phase + 1u) & 7u;
            InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
            return 0;
        }
        if (message == WM_SETFOCUS_VALUE) {
            SpwObject *row = spw_component_row_ancestor(object);
            spw_push_event(SPW_EVENT_FOCUS_RECEIVED, object->id, 0, 0);
            if (row != NULL && row != object)
                spw_push_event(SPW_EVENT_SELECTION_CHANGED, row->id, 1, 0);
        } else if (message == WM_KILLFOCUS_VALUE)
            spw_push_event(SPW_EVENT_FOCUS_LOST, object->id, 0, 0);
        else if (message == WM_KEYDOWN_VALUE || message == WM_SYSKEYDOWN_VALUE) {
            if (message == WM_KEYDOWN_VALUE && (uint32_t)wParam == 0x1bu) {
                SpwObject *parent = spw_find_object_by_id(object->parent_id);
                if (parent != NULL && parent->type == SPW_OBJECT_WINDOW && parent->popup_window) {
                    spw_push_event(SPW_EVENT_POPUP_DISMISS_REQUESTED, parent->id, 1, 0);
                    return 0;
                }
            }
            spw_push_event(SPW_EVENT_KEY_DOWN, object->id, (int64_t)(uint32_t)wParam,
                (int64_t)spw_modifier_flags());
            if (message == WM_KEYDOWN_VALUE && object->kind == SPW_CONTROL_NUMBER_INPUT &&
                ((uint32_t)wParam == VK_UP_VALUE || (uint32_t)wParam == VK_DOWN_VALUE)) {
                spw_push_event(SPW_EVENT_NUMBER_STEP, object->id,
                    (uint32_t)wParam == VK_UP_VALUE ? 1 : -1, 0);
                return 0;
            }
            if (message == WM_KEYDOWN_VALUE && (uint32_t)wParam == VK_RETURN_VALUE) {
                if (object->kind == SPW_CONTROL_LIST || object->kind == SPW_CONTROL_TABLE) {
                    LRESULT selected = SendMessageW(object->hwnd, LVM_GETNEXTITEM_VALUE,
                        (WPARAM)(intptr_t)-1, (LPARAM)LVNI_SELECTED_VALUE);
                    if (selected >= 0)
                        spw_push_event(SPW_EVENT_ACTIVATED, object->id, (int64_t)selected + 1, 2);
                } else if (object->kind == SPW_CONTROL_TREE) {
                    int32_t token = spw_tree_selected_token(object);
                    if (token > 0)
                        spw_push_event(SPW_EVENT_ACTIVATED, object->id, token, 2);
                }
            }
        } else if (message == WM_CHAR_VALUE && object->kind == SPW_CONTROL_NUMBER_INPUT) {
            uint32_t ch = (uint32_t)wParam;
            if (ch >= 0x20u && !((ch >= (uint32_t)'0' && ch <= (uint32_t)'9') ||
                ch == (uint32_t)'-' || ch == (uint32_t)'+' || ch == (uint32_t)'.' || ch == (uint32_t)','))
                return 0;
        } else if (message == WM_KEYUP_VALUE || message == WM_SYSKEYUP_VALUE) {
            spw_push_event(SPW_EVENT_KEY_UP, object->id, (int64_t)(uint32_t)wParam,
                (int64_t)spw_modifier_flags());
        } else if (message == WM_CONTEXTMENU_VALUE && object->context_menu_enabled) {
            int32_t x = spw_signed_low_word(lParam);
            int32_t y = spw_signed_high_word(lParam);
            if (x == -1 && y == -1) {
                RECT rect;
                if (GetWindowRect(object->hwnd, &rect)) {
                    x = rect.left + (rect.right - rect.left) / 2;
                    y = rect.top + (rect.bottom - rect.top) / 2;
                } else {
                    x = 0;
                    y = 0;
                }
            }
            spw_push_event(SPW_EVENT_CONTEXT_MENU_REQUESTED, object->id, x, y);
            return 0;
        } else if (message == WM_MOUSEMOVE_VALUE) {
            object->mouse_x = spw_signed_low_word(lParam);
            object->mouse_y = spw_signed_high_word(lParam);
            spw_track_mouse_leave(object);
            spw_push_mouse_event(SPW_EVENT_MOUSE_MOVE, object, lParam, SPW_MOUSE_BUTTON_NONE);
        } else if (message == WM_MOUSELEAVE_VALUE) {
            if (object->mouse_inside) {
                object->mouse_inside = 0u;
                spw_push_event(SPW_EVENT_MOUSE_LEAVE, object->id, (int64_t)object->mouse_x,
                    spw_pack_mouse_argument2(object->mouse_y, spw_modifier_flags(), SPW_MOUSE_BUTTON_NONE));
            }
        } else if (message == WM_LBUTTONDOWN_VALUE) {
            SpwObject *row = spw_component_row_ancestor(object);
            if (row != NULL) {
                spw_push_event(SPW_EVENT_CLICKED, row->id, (int64_t)spw_modifier_flags(), 0);
                if (row == object) SetFocus(row->hwnd);
            }
            if ((object->kind == SPW_CONTROL_NUMBER_INPUT || object->kind == SPW_CONTROL_SEARCH_INPUT) &&
                spw_edit_affordance_hit(object, lParam, NULL)) {
                SetFocus(object->hwnd);
                return 0;
            }
            spw_push_mouse_event(SPW_EVENT_MOUSE_DOWN, object, lParam, SPW_MOUSE_BUTTON_PRIMARY);
        } else if (message == WM_RBUTTONDOWN_VALUE) {
            spw_push_mouse_event(SPW_EVENT_MOUSE_DOWN, object, lParam, SPW_MOUSE_BUTTON_SECONDARY);
        } else if (message == WM_MBUTTONDOWN_VALUE) {
            spw_push_mouse_event(SPW_EVENT_MOUSE_DOWN, object, lParam, SPW_MOUSE_BUTTON_MIDDLE);
        } else if (message == WM_LBUTTONUP_VALUE) {
            int upper_half = 0;
            if (object->kind == SPW_CONTROL_NUMBER_INPUT && spw_edit_affordance_hit(object, lParam, &upper_half)) {
                spw_push_event(SPW_EVENT_NUMBER_STEP, object->id, upper_half ? 1 : -1, 0);
                return 0;
            }
            if (object->kind == SPW_CONTROL_SEARCH_INPUT && spw_edit_affordance_hit(object, lParam, NULL)) {
                if (GetWindowTextLengthW(object->hwnd) > 0)
                    SetWindowTextW(object->hwnd, L"");
                InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
                return 0;
            }
            spw_push_mouse_event(SPW_EVENT_MOUSE_UP, object, lParam, SPW_MOUSE_BUTTON_PRIMARY);
            if (object->kind == SPW_CONTROL_IMAGE)
                spw_push_event(SPW_EVENT_CLICKED, object->id, 0, 0);
        } else if (message == WM_RBUTTONUP_VALUE) {
            spw_push_mouse_event(SPW_EVENT_MOUSE_UP, object, lParam, SPW_MOUSE_BUTTON_SECONDARY);
        } else if (message == WM_MBUTTONUP_VALUE) {
            spw_push_mouse_event(SPW_EVENT_MOUSE_UP, object, lParam, SPW_MOUSE_BUTTON_MIDDLE);
        } else if (message == WM_LBUTTONDBLCLK_VALUE) {
            SpwObject *row = spw_component_row_ancestor(object);
            spw_push_mouse_event(SPW_EVENT_MOUSE_DOUBLE_CLICK, object, lParam, SPW_MOUSE_BUTTON_PRIMARY);
            if (row != NULL)
                spw_push_event(SPW_EVENT_ACTIVATED, row->id, 0, 1);
        } else if (message == WM_RBUTTONDBLCLK_VALUE) {
            spw_push_mouse_event(SPW_EVENT_MOUSE_DOUBLE_CLICK, object, lParam, SPW_MOUSE_BUTTON_SECONDARY);
        } else if (message == WM_MBUTTONDBLCLK_VALUE) {
            spw_push_mouse_event(SPW_EVENT_MOUSE_DOUBLE_CLICK, object, lParam, SPW_MOUSE_BUTTON_MIDDLE);
        }
    }

    if (message == WM_NCDESTROY_VALUE) {
        if (object != NULL && object->kind == SPW_CONTROL_SPINNER)
            KillTimer(hwnd, 1u);
        RemoveWindowSubclass(hwnd, spw_control_subclass_proc, subclass_id);
    }

    return DefSubclassProc(hwnd, message, wParam, lParam);
}

