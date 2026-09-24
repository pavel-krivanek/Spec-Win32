static const WCHAR *spw_control_class_name(uint32_t kind) {
    if (kind == SPW_CONTROL_LABEL) return g_static_class_name;
    if (kind == SPW_CONTROL_BUTTON ||
        kind == SPW_CONTROL_CHECKBOX ||
        kind == SPW_CONTROL_RADIOBUTTON ||
        kind == SPW_CONTROL_TOGGLE_BUTTON ||
        kind == SPW_CONTROL_SWITCH ||
        kind == SPW_CONTROL_FRAME) return g_button_class_name;
    if (kind == SPW_CONTROL_CODE) return g_rich_edit_class_name;
    if (kind == SPW_CONTROL_TEXT_INPUT || kind == SPW_CONTROL_NUMBER_INPUT || kind == SPW_CONTROL_SEARCH_INPUT || kind == SPW_CONTROL_TEXT_AREA) return g_edit_class_name;
    if (kind == SPW_CONTROL_DROP_LIST) return g_combo_box_class_name;
    if (kind == SPW_CONTROL_SLIDER_HORIZONTAL || kind == SPW_CONTROL_SLIDER_VERTICAL) return g_trackbar_class_name;
    if (kind == SPW_CONTROL_PROGRESS) return g_progress_class_name;
    if (kind == SPW_CONTROL_NOTEBOOK) return g_tab_class_name;
    if (kind == SPW_CONTROL_LIST || kind == SPW_CONTROL_TABLE) return g_list_view_class_name;
    if (kind == SPW_CONTROL_TREE) return g_tree_view_class_name;
    if (kind == SPW_CONTROL_TREE_COLUMN || kind == SPW_CONTROL_COMPONENT_ROW) return g_window_class_name;
    if (kind == SPW_CONTROL_SPINNER) return g_static_class_name;
    if (kind == SPW_CONTROL_LINK) return g_link_class_name;
    if (kind == SPW_CONTROL_IMAGE) return g_static_class_name;
    if (kind == SPW_CONTROL_PANED) return g_paned_class_name;
    if (kind == SPW_CONTROL_SCROLL_VIEWPORT || kind == SPW_CONTROL_SCROLL_CONTENT ||
        kind == SPW_CONTROL_PANED_CONTENT || kind == SPW_CONTROL_TAB_CONTENT) return g_window_class_name;
    return NULL;
}

static DWORD spw_control_style(uint32_t kind) {
    DWORD style = WS_CHILD_VALUE | WS_VISIBLE_VALUE;
    if (kind == SPW_CONTROL_LABEL) return style | SS_LEFT_VALUE;
    if (kind == SPW_CONTROL_BUTTON) return style | WS_TABSTOP_VALUE | BS_PUSHBUTTON_VALUE;
    if (kind == SPW_CONTROL_FRAME)
        return style | BS_GROUPBOX_VALUE | WS_CLIPCHILDREN_VALUE | WS_CLIPSIBLINGS_VALUE;
    if (kind == SPW_CONTROL_CHECKBOX) return style | WS_TABSTOP_VALUE | BS_AUTOCHECKBOX_VALUE;
    if (kind == SPW_CONTROL_RADIOBUTTON) return style | WS_TABSTOP_VALUE | BS_RADIOBUTTON_VALUE;
    if (kind == SPW_CONTROL_TOGGLE_BUTTON) return style | WS_TABSTOP_VALUE | BS_AUTOCHECKBOX_VALUE | BS_PUSHLIKE_VALUE;
    if (kind == SPW_CONTROL_SWITCH) return style | WS_TABSTOP_VALUE | BS_AUTOCHECKBOX_VALUE;
    if (kind == SPW_CONTROL_TEXT_INPUT || kind == SPW_CONTROL_NUMBER_INPUT || kind == SPW_CONTROL_SEARCH_INPUT) return style | WS_TABSTOP_VALUE | WS_BORDER_VALUE | ES_AUTOHSCROLL_VALUE;
    if (kind == SPW_CONTROL_TEXT_AREA) return style | WS_TABSTOP_VALUE | WS_BORDER_VALUE | ES_MULTILINE_VALUE | ES_AUTOVSCROLL_VALUE | ES_WANTRETURN_VALUE | WS_VSCROLL_VALUE;
    if (kind == SPW_CONTROL_CODE) return style | WS_TABSTOP_VALUE | WS_BORDER_VALUE | ES_MULTILINE_VALUE | ES_AUTOVSCROLL_VALUE | ES_WANTRETURN_VALUE | WS_VSCROLL_VALUE | WS_HSCROLL_VALUE | ES_AUTOHSCROLL_VALUE;
    if (kind == SPW_CONTROL_DROP_LIST) return style | WS_TABSTOP_VALUE | WS_VSCROLL_VALUE | CBS_DROPDOWNLIST_VALUE | CBS_HASSTRINGS_VALUE;
    if (kind == SPW_CONTROL_SLIDER_HORIZONTAL) return style | WS_TABSTOP_VALUE | TBS_HORZ_VALUE | TBS_AUTOTICKS_VALUE;
    if (kind == SPW_CONTROL_SLIDER_VERTICAL) return style | WS_TABSTOP_VALUE | TBS_VERT_VALUE | TBS_AUTOTICKS_VALUE;
    if (kind == SPW_CONTROL_PROGRESS) return style | PBS_SMOOTH_VALUE;
    if (kind == SPW_CONTROL_NOTEBOOK) return style | WS_TABSTOP_VALUE | WS_CLIPSIBLINGS_VALUE;
    if (kind == SPW_CONTROL_LIST) return style | WS_TABSTOP_VALUE | WS_BORDER_VALUE | LVS_REPORT_VALUE | LVS_SINGLESEL_VALUE | LVS_SHOWSELALWAYS_VALUE | LVS_NOCOLUMNHEADER_VALUE;
    if (kind == SPW_CONTROL_TABLE) return style | WS_TABSTOP_VALUE | WS_BORDER_VALUE | LVS_REPORT_VALUE | LVS_SINGLESEL_VALUE | LVS_SHOWSELALWAYS_VALUE;
    if (kind == SPW_CONTROL_TREE) return style | WS_TABSTOP_VALUE | WS_BORDER_VALUE | TVS_HASBUTTONS_VALUE | TVS_HASLINES_VALUE | TVS_LINESATROOT_VALUE | TVS_SHOWSELALWAYS_VALUE;
    if (kind == SPW_CONTROL_TREE_COLUMN) return style | WS_TABSTOP_VALUE | WS_BORDER_VALUE | WS_CLIPCHILDREN_VALUE | WS_CLIPSIBLINGS_VALUE;
    if (kind == SPW_CONTROL_COMPONENT_ROW) return style | WS_TABSTOP_VALUE | WS_CLIPCHILDREN_VALUE | WS_CLIPSIBLINGS_VALUE;
    if (kind == SPW_CONTROL_SPINNER) return style;
    if (kind == SPW_CONTROL_LINK) return style | WS_TABSTOP_VALUE;
    if (kind == SPW_CONTROL_IMAGE) return style | SS_NOTIFY_VALUE;
    if (kind == SPW_CONTROL_SCROLL_VIEWPORT)
        return style | WS_CLIPCHILDREN_VALUE | WS_CLIPSIBLINGS_VALUE | WS_HSCROLL_VALUE | WS_VSCROLL_VALUE;
    if (kind == SPW_CONTROL_SCROLL_CONTENT)
        return style | WS_CLIPCHILDREN_VALUE | WS_CLIPSIBLINGS_VALUE;
    if (kind == SPW_CONTROL_PANED)
        return style | WS_CLIPCHILDREN_VALUE | WS_CLIPSIBLINGS_VALUE;
    if (kind == SPW_CONTROL_PANED_CONTENT || kind == SPW_CONTROL_TAB_CONTENT)
        return style | WS_CLIPCHILDREN_VALUE | WS_CLIPSIBLINGS_VALUE;
    return 0u;
}

static int spw_control_is_checkable(uint32_t kind) {
    return kind == SPW_CONTROL_CHECKBOX || kind == SPW_CONTROL_RADIOBUTTON ||
           kind == SPW_CONTROL_TOGGLE_BUTTON || kind == SPW_CONTROL_SWITCH ||
           kind == SPW_CONTROL_COMPONENT_ROW;
}

static int spw_control_is_text_input(uint32_t kind) {
    return kind == SPW_CONTROL_TEXT_INPUT || kind == SPW_CONTROL_NUMBER_INPUT || kind == SPW_CONTROL_SEARCH_INPUT || kind == SPW_CONTROL_TEXT_AREA || kind == SPW_CONTROL_CODE;
}

static int spw_control_is_multiline_text(uint32_t kind) {
    return kind == SPW_CONTROL_TEXT_AREA || kind == SPW_CONTROL_CODE;
}

static int spw_control_is_drop_list(uint32_t kind) {
    return kind == SPW_CONTROL_DROP_LIST;
}

static int spw_control_is_slider(uint32_t kind) {
    return kind == SPW_CONTROL_SLIDER_HORIZONTAL || kind == SPW_CONTROL_SLIDER_VERTICAL;
}

static int spw_control_is_progress(uint32_t kind) {
    return kind == SPW_CONTROL_PROGRESS;
}

static int spw_control_is_notebook(uint32_t kind) {
    return kind == SPW_CONTROL_NOTEBOOK;
}

static int spw_control_is_list(uint32_t kind) {
    return kind == SPW_CONTROL_LIST;
}

static int spw_control_is_table(uint32_t kind) {
    return kind == SPW_CONTROL_TABLE;
}

static int spw_control_is_tree(uint32_t kind) {
    return kind == SPW_CONTROL_TREE || kind == SPW_CONTROL_TREE_COLUMN;
}

static int spw_control_has_table_surface(uint32_t kind) {
    return kind == SPW_CONTROL_TABLE || kind == SPW_CONTROL_TREE_COLUMN;
}

static HWND spw_tree_hwnd(SpwObject *object) {
    return object != NULL && object->kind == SPW_CONTROL_TREE_COLUMN ? object->tree_hwnd : (object != NULL ? object->hwnd : NULL);
}

static HWND spw_table_hwnd(SpwObject *object) {
    return object != NULL && object->kind == SPW_CONTROL_TREE_COLUMN ? object->table_hwnd : (object != NULL ? object->hwnd : NULL);
}

static int spw_control_is_list_view(uint32_t kind) {
    return spw_control_is_list(kind) || spw_control_is_table(kind);
}

static int spw_control_has_selection(uint32_t kind) {
    return spw_control_is_drop_list(kind) || spw_control_is_notebook(kind);
}

static int32_t spw_clamp_normalized_value(int32_t value) {
    if (value < 0) return 0;
    if (value > SPW_NORMALIZED_MAX) return SPW_NORMALIZED_MAX;
    return value;
}

static int spw_utf8_to_wide(const char *text, uint32_t text_length, WCHAR *buffer, int capacity);

static uint32_t spw_read_u32_le(const uint8_t *bytes) {
    return ((uint32_t)bytes[0]) |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

static int32_t spw_selection_control_set_items(SpwObject *object, const char *blob,
                                              uint32_t blob_length, uint32_t item_count) {
    const uint8_t *bytes = (const uint8_t *)blob;
    uint32_t offset = 0u;
    uint32_t i;
    int32_t status = SPW_STATUS_OK;

    if (object == NULL || !spw_control_has_selection(object->kind))
        return SPW_STATUS_NOT_FOUND;
    if (item_count > 0u && blob == NULL)
        return SPW_STATUS_BAD_ARGUMENT;

    object->suppress_notifications += 1u;
    if (spw_control_is_drop_list(object->kind))
        SendMessageW(object->hwnd, CB_RESETCONTENT_VALUE, 0u, 0);
    else
        SendMessageW(object->hwnd, TCM_DELETEALLITEMS_VALUE, 0u, 0);
    for (i = 0u; i < item_count; ++i) {
        uint32_t item_length;
        LRESULT added;
        if (blob_length - offset < 4u) {
            status = SPW_STATUS_BAD_ARGUMENT;
            break;
        }
        item_length = spw_read_u32_le(bytes + offset);
        offset += 4u;
        if (item_length > blob_length - offset) {
            status = SPW_STATUS_BAD_ARGUMENT;
            break;
        }
        status = spw_utf8_to_wide((const char *)(bytes + offset), item_length,
                                  g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
        if (status != SPW_STATUS_OK)
            break;
        if (spw_control_is_drop_list(object->kind)) {
            added = SendMessageW(object->hwnd, CB_ADDSTRING_VALUE, 0u,
                                 (LPARAM)(uintptr_t)g_control_text_wide);
            if (added == CB_ERR_VALUE || added == CB_ERRSPACE_VALUE) {
                status = SPW_STATUS_WIN32_ERROR;
                break;
            }
        } else {
            TCITEMW item;
            item.mask = TCIF_TEXT_VALUE;
            item.dwState = 0u;
            item.dwStateMask = 0u;
            item.pszText = g_control_text_wide;
            item.cchTextMax = 0;
            item.iImage = -1;
            item.lParam = 0;
            added = SendMessageW(object->hwnd, TCM_INSERTITEMW_VALUE, (WPARAM)i,
                                 (LPARAM)(uintptr_t)&item);
            if (added < 0) {
                status = SPW_STATUS_WIN32_ERROR;
                break;
            }
        }
        offset += item_length;
    }
    if (status == SPW_STATUS_OK && offset != blob_length)
        status = SPW_STATUS_BAD_ARGUMENT;
    object->suppress_notifications -= 1u;
    return status;
}

static void spw_list_release_image_list(SpwObject *object) {
    if (object == NULL)
        return;
    if (object->hwnd != NULL) {
        if (object->kind == SPW_CONTROL_TREE)
            SendMessageW(object->hwnd, TVM_SETIMAGELIST_VALUE, (WPARAM)TVSIL_NORMAL_VALUE, 0);
        else
            SendMessageW(object->hwnd, LVM_SETIMAGELIST_VALUE, (WPARAM)LVSIL_SMALL_VALUE, 0);
    }
    if (object->list_image_list != NULL) {
        ImageList_Destroy(object->list_image_list);
        object->list_image_list = NULL;
    }
}

static void spw_tree_release_image_list(SpwObject *object) {
    HIMAGELIST *slot;
    HWND hwnd;
    if (object == NULL) return;
    hwnd = spw_tree_hwnd(object);
    slot = object->kind == SPW_CONTROL_TREE_COLUMN
        ? &object->tree_column_tree_image_list : &object->list_image_list;
    if (hwnd != NULL)
        SendMessageW(hwnd, TVM_SETIMAGELIST_VALUE, (WPARAM)TVSIL_NORMAL_VALUE, 0);
    if (*slot != NULL) {
        ImageList_Destroy(*slot);
        *slot = NULL;
    }
}

static void spw_table_release_image_list(SpwObject *object) {
    HIMAGELIST *slot;
    HWND hwnd;
    if (object == NULL) return;
    hwnd = spw_table_hwnd(object);
    slot = object->kind == SPW_CONTROL_TREE_COLUMN
        ? &object->tree_column_table_image_list : &object->list_image_list;
    if (hwnd != NULL)
        SendMessageW(hwnd, LVM_SETIMAGELIST_VALUE, (WPARAM)LVSIL_SMALL_VALUE, 0);
    if (*slot != NULL) {
        ImageList_Destroy(*slot);
        *slot = NULL;
    }
}


static HBITMAP spw_create_padded_bitmap_bgra(const uint8_t *pixels, uint32_t byte_length,
                                              int32_t width, int32_t height,
                                              int32_t target_width, int32_t target_height) {
    BITMAPINFO info = {0};
    HBITMAP bitmap;
    void *raw_bits = NULL;
    uint8_t *bits;
    int32_t x_offset, y_offset, y, x;
    uint64_t expected;
    uint64_t target_bytes;
    if (pixels == NULL || width <= 0 || height <= 0 || target_width <= 0 || target_height <= 0)
        return NULL;
    expected = (uint64_t)(uint32_t)width * (uint64_t)(uint32_t)height * 4u;
    if (expected != (uint64_t)byte_length || width > target_width || height > target_height)
        return NULL;
    target_bytes = (uint64_t)(uint32_t)target_width * (uint64_t)(uint32_t)target_height * 4u;
    if (target_bytes > 0xffffffffu)
        return NULL;
    info.bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = target_width;
    info.bmiHeader.biHeight = -target_height;
    info.bmiHeader.biPlanes = 1u;
    info.bmiHeader.biBitCount = 32u;
    info.bmiHeader.biCompression = BI_RGB_VALUE;
    info.bmiHeader.biSizeImage = (DWORD)target_bytes;
    bitmap = CreateDIBSection(NULL, &info, DIB_RGB_COLORS_VALUE, &raw_bits, NULL, 0u);
    if (bitmap == NULL || raw_bits == NULL) {
        if (bitmap != NULL) DeleteObject((HGDIOBJ)bitmap);
        return NULL;
    }
    bits = (uint8_t *)raw_bits;
    memset(bits, 0, (size_t)target_bytes);
    x_offset = (target_width - width) / 2;
    y_offset = (target_height - height) / 2;
    for (y = 0; y < height; ++y) {
        const uint8_t *source = pixels + ((uint32_t)y * (uint32_t)width * 4u);
        uint8_t *destination = bits + (((uint32_t)(y + y_offset) * (uint32_t)target_width + (uint32_t)x_offset) * 4u);
        for (x = 0; x < width * 4; ++x)
            destination[x] = source[x];
    }
    return bitmap;
}

/* Packed row format: u32 UTF-8 length, i32 image width, i32 image height,
 * u32 BGRA byte length, followed by UTF-8 bytes and then top-down BGRA bytes. */
static int32_t spw_list_apply_rows(SpwObject *object, const uint8_t *blob,
                                 uint32_t blob_length, uint32_t row_count) {
    uint32_t offset = 0u, i;
    int32_t max_width = 0, max_height = 0;
    int32_t status = SPW_STATUS_OK;
    HIMAGELIST new_list = NULL;

    if (object == NULL || !spw_control_is_list(object->kind))
        return SPW_STATUS_NOT_FOUND;
    if (row_count > 0u && blob == NULL)
        return SPW_STATUS_BAD_ARGUMENT;

    /* Validate first and determine one stable small-image-list cell size. */
    for (i = 0u; i < row_count; ++i) {
        uint32_t text_length, image_length;
        int32_t width, height;
        uint64_t expected;
        if (blob_length - offset < 16u) return SPW_STATUS_BAD_ARGUMENT;
        text_length = spw_read_u32_le(blob + offset);
        width = (int32_t)spw_read_u32_le(blob + offset + 4u);
        height = (int32_t)spw_read_u32_le(blob + offset + 8u);
        image_length = spw_read_u32_le(blob + offset + 12u);
        offset += 16u;
        if (text_length > blob_length - offset) return SPW_STATUS_BAD_ARGUMENT;
        offset += text_length;
        if (image_length > blob_length - offset) return SPW_STATUS_BAD_ARGUMENT;
        if (image_length == 0u) {
            if (width != 0 || height != 0) return SPW_STATUS_BAD_ARGUMENT;
        } else {
            if (width <= 0 || height <= 0 || width > 256 || height > 256) return SPW_STATUS_BAD_ARGUMENT;
            expected = (uint64_t)(uint32_t)width * (uint64_t)(uint32_t)height * 4u;
            if (expected != (uint64_t)image_length) return SPW_STATUS_BAD_ARGUMENT;
            if (width > max_width) max_width = width;
            if (height > max_height) max_height = height;
        }
        offset += image_length;
    }
    if (offset != blob_length) return SPW_STATUS_BAD_ARGUMENT;

    if (max_width > 0 && max_height > 0) {
        new_list = ImageList_Create(max_width, max_height, ILC_COLOR32_VALUE | ILC_MASK_VALUE,
                                    (int32_t)row_count, 1);
        if (new_list == NULL) return SPW_STATUS_WIN32_ERROR;
    }

    object->suppress_notifications += 1u;
    spw_list_release_image_list(object);
    if (new_list != NULL) {
        SendMessageW(object->hwnd, LVM_SETIMAGELIST_VALUE, (WPARAM)LVSIL_SMALL_VALUE,
                     (LPARAM)(uintptr_t)new_list);
        object->list_image_list = new_list;
    }
    SendMessageW(object->hwnd, LVM_DELETEALLITEMS_VALUE, 0u, 0);

    offset = 0u;
    for (i = 0u; i < row_count; ++i) {
        uint32_t text_length, image_length;
        int32_t width, height, image_index = -1;
        LVITEMW item;
        LRESULT inserted;
        const uint8_t *image_bytes;
        HBITMAP bitmap;
        text_length = spw_read_u32_le(blob + offset);
        width = (int32_t)spw_read_u32_le(blob + offset + 4u);
        height = (int32_t)spw_read_u32_le(blob + offset + 8u);
        image_length = spw_read_u32_le(blob + offset + 12u);
        offset += 16u;
        status = spw_utf8_to_wide((const char *)(blob + offset), text_length,
                                  g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
        if (status != SPW_STATUS_OK) break;
        offset += text_length;
        image_bytes = blob + offset;
        if (image_length > 0u) {
            bitmap = spw_create_padded_bitmap_bgra(image_bytes, image_length, width, height,
                                                    max_width, max_height);
            if (bitmap == NULL) { status = SPW_STATUS_WIN32_ERROR; break; }
            image_index = ImageList_Add(new_list, bitmap, NULL);
            DeleteObject((HGDIOBJ)bitmap);
            if (image_index < 0) { status = SPW_STATUS_WIN32_ERROR; break; }
        }
        item.mask = LVIF_TEXT_VALUE | (image_index >= 0 ? LVIF_IMAGE_VALUE : 0u);
        item.iItem = (int32_t)i;
        item.iSubItem = 0;
        item.state = 0u;
        item.stateMask = 0u;
        item.pszText = g_control_text_wide;
        item.cchTextMax = 0;
        item.iImage = image_index;
        item.lParam = 0;
        inserted = SendMessageW(object->hwnd, LVM_INSERTITEMW_VALUE, 0u, (LPARAM)(uintptr_t)&item);
        if (inserted < 0) { status = SPW_STATUS_WIN32_ERROR; break; }
        offset += image_length;
    }
    object->suppress_notifications -= 1u;
    if (status != SPW_STATUS_OK) {
        SendMessageW(object->hwnd, LVM_DELETEALLITEMS_VALUE, 0u, 0);
        spw_list_release_image_list(object);
    }
    return status;
}

static int32_t spw_list_set_items(SpwObject *object, const char *blob,
                                  uint32_t blob_length, uint32_t item_count) {
    const uint8_t *bytes = (const uint8_t *)blob;
    uint32_t offset = 0u;
    uint32_t i;
    int32_t status = SPW_STATUS_OK;

    if (object == NULL || !spw_control_is_list(object->kind))
        return SPW_STATUS_NOT_FOUND;
    if (item_count > 0u && blob == NULL)
        return SPW_STATUS_BAD_ARGUMENT;

    object->suppress_notifications += 1u;
    spw_list_release_image_list(object);
    SendMessageW(object->hwnd, LVM_DELETEALLITEMS_VALUE, 0u, 0);
    for (i = 0u; i < item_count; ++i) {
        uint32_t item_length;
        LVITEMW item;
        LRESULT inserted;
        if (blob_length - offset < 4u) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        item_length = spw_read_u32_le(bytes + offset);
        offset += 4u;
        if (item_length > blob_length - offset) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        status = spw_utf8_to_wide((const char *)(bytes + offset), item_length,
                                  g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
        if (status != SPW_STATUS_OK) break;
        item.mask = LVIF_TEXT_VALUE;
        item.iItem = (int32_t)i;
        item.iSubItem = 0;
        item.state = 0u;
        item.stateMask = 0u;
        item.pszText = g_control_text_wide;
        item.cchTextMax = 0;
        item.iImage = -1;
        item.lParam = 0;
        inserted = SendMessageW(object->hwnd, LVM_INSERTITEMW_VALUE, 0u, (LPARAM)(uintptr_t)&item);
        if (inserted < 0) { status = SPW_STATUS_WIN32_ERROR; break; }
        offset += item_length;
    }
    if (status == SPW_STATUS_OK && offset != blob_length)
        status = SPW_STATUS_BAD_ARGUMENT;
    object->suppress_notifications -= 1u;
    return status;
}


static HTREEITEM spw_tree_find_token_from(SpwObject *object, HTREEITEM item, int32_t token) {
    while (item != NULL) {
        TVITEMW query; HTREEITEM child; HTREEITEM found;
        query.mask = TVIF_PARAM_VALUE; query.hItem = item; query.state = 0; query.stateMask = 0;
        query.pszText = NULL; query.cchTextMax = 0; query.iImage = 0; query.iSelectedImage = 0; query.cChildren = 0; query.lParam = 0;
        if (SendMessageW(spw_tree_hwnd(object), TVM_GETITEMW_VALUE, 0, (LPARAM)(uintptr_t)&query) && (int32_t)query.lParam == token) return item;
        child = (HTREEITEM)(uintptr_t)SendMessageW(spw_tree_hwnd(object), TVM_GETNEXTITEM_VALUE, TVGN_CHILD_VALUE, (LPARAM)(uintptr_t)item);
        found = spw_tree_find_token_from(object, child, token);
        if (found != NULL) return found;
        item = (HTREEITEM)(uintptr_t)SendMessageW(spw_tree_hwnd(object), TVM_GETNEXTITEM_VALUE, TVGN_NEXT_VALUE, (LPARAM)(uintptr_t)item);
    }
    return NULL;
}

static HTREEITEM spw_tree_find_token(SpwObject *object, int32_t token) {
    HTREEITEM root;
    if (object == NULL || !spw_control_is_tree(object->kind) || token <= 0) return NULL;
    root = (HTREEITEM)(uintptr_t)SendMessageW(spw_tree_hwnd(object), TVM_GETNEXTITEM_VALUE, TVGN_ROOT_VALUE, 0);
    return spw_tree_find_token_from(object, root, token);
}

static int32_t spw_tree_set_nodes(SpwObject *object, const char *blob, uint32_t blob_length, const int32_t *parents, uint32_t count) {
    const uint8_t *bytes=(const uint8_t*)blob; uint32_t offset=0u,i; int32_t status=SPW_STATUS_OK;
    if (object==NULL || !spw_control_is_tree(object->kind)) return SPW_STATUS_NOT_FOUND;
    if (count>0u && (blob==NULL || parents==NULL)) return SPW_STATUS_BAD_ARGUMENT;
    object->suppress_notifications += 1u;
    spw_tree_release_image_list(object);
    SendMessageW(spw_tree_hwnd(object), TVM_DELETEITEM_VALUE, 0, (LPARAM)(uintptr_t)TVI_ROOT_VALUE);
    for(i=0u;i<count;++i){ uint32_t len; TVINSERTSTRUCTW ins; HTREEITEM parent=TVI_ROOT_VALUE; LRESULT added;
        if(blob_length-offset<4u){status=SPW_STATUS_BAD_ARGUMENT;break;} len=spw_read_u32_le(bytes+offset); offset+=4u;
        if(len>blob_length-offset){status=SPW_STATUS_BAD_ARGUMENT;break;}
        status=spw_utf8_to_wide((const char*)(bytes+offset),len,g_control_text_wide,SPW_MAX_CONTROL_TEXT_WCHARS); if(status!=SPW_STATUS_OK)break;
        if(parents[i]>0){ parent=spw_tree_find_token(object,parents[i]); if(parent==NULL){status=SPW_STATUS_BAD_ARGUMENT;break;} }
        ins.hParent=parent; ins.hInsertAfter=TVI_LAST_VALUE;
        ins.item.mask=TVIF_TEXT_VALUE|TVIF_PARAM_VALUE|TVIF_CHILDREN_VALUE; ins.item.hItem=NULL; ins.item.state=0; ins.item.stateMask=0; ins.item.pszText=g_control_text_wide; ins.item.cchTextMax=0; ins.item.iImage=0; ins.item.iSelectedImage=0; ins.item.cChildren=0; ins.item.lParam=(LPARAM)(i+1);
        added=SendMessageW(spw_tree_hwnd(object),TVM_INSERTITEMW_VALUE,0,(LPARAM)(uintptr_t)&ins); if(added==0){status=SPW_STATUS_WIN32_ERROR;break;} offset+=len;
    }
    if(status==SPW_STATUS_OK && offset!=blob_length) status=SPW_STATUS_BAD_ARGUMENT;
    if (object->kind == SPW_CONTROL_TREE_COLUMN) spw_tree_column_layout(object);
    object->suppress_notifications -= 1u; return status;
}

static int32_t spw_tree_apply_nodes_with_images(SpwObject *object, const uint8_t *blob,
                                                   uint32_t blob_length, const int32_t *parents,
                                                   uint32_t count) {
    uint32_t offset = 0u, i;
    int32_t max_width = 0, max_height = 0;
    int32_t status = SPW_STATUS_OK;
    HIMAGELIST new_list = NULL;

    if (object == NULL || !spw_control_is_tree(object->kind)) return SPW_STATUS_NOT_FOUND;
    if (count > 0u && (blob == NULL || parents == NULL)) return SPW_STATUS_BAD_ARGUMENT;

    for (i = 0u; i < count; ++i) {
        uint32_t text_length, image_length;
        int32_t width, height;
        uint64_t expected;
        if (blob_length - offset < 16u) return SPW_STATUS_BAD_ARGUMENT;
        text_length = spw_read_u32_le(blob + offset);
        width = (int32_t)spw_read_u32_le(blob + offset + 4u);
        height = (int32_t)spw_read_u32_le(blob + offset + 8u);
        image_length = spw_read_u32_le(blob + offset + 12u);
        offset += 16u;
        if (text_length > blob_length - offset) return SPW_STATUS_BAD_ARGUMENT;
        offset += text_length;
        if (image_length > blob_length - offset) return SPW_STATUS_BAD_ARGUMENT;
        if (image_length == 0u) {
            if (width != 0 || height != 0) return SPW_STATUS_BAD_ARGUMENT;
        } else {
            if (width <= 0 || height <= 0 || width > 256 || height > 256) return SPW_STATUS_BAD_ARGUMENT;
            expected = (uint64_t)(uint32_t)width * (uint64_t)(uint32_t)height * 4u;
            if (expected != (uint64_t)image_length) return SPW_STATUS_BAD_ARGUMENT;
            if (width > max_width) max_width = width;
            if (height > max_height) max_height = height;
        }
        offset += image_length;
    }
    if (offset != blob_length) return SPW_STATUS_BAD_ARGUMENT;

    if (max_width > 0 && max_height > 0) {
        new_list = ImageList_Create(max_width, max_height, ILC_COLOR32_VALUE | ILC_MASK_VALUE,
                                    (int32_t)count, 1);
        if (new_list == NULL) return SPW_STATUS_WIN32_ERROR;
    }

    object->suppress_notifications += 1u;
    spw_tree_release_image_list(object);
    if (new_list != NULL) {
        SendMessageW(spw_tree_hwnd(object), TVM_SETIMAGELIST_VALUE, (WPARAM)TVSIL_NORMAL_VALUE,
                     (LPARAM)(uintptr_t)new_list);
        if (object->kind == SPW_CONTROL_TREE_COLUMN) object->tree_column_tree_image_list = new_list;
        else object->list_image_list = new_list;
    }
    SendMessageW(spw_tree_hwnd(object), TVM_DELETEITEM_VALUE, 0, (LPARAM)(uintptr_t)TVI_ROOT_VALUE);

    offset = 0u;
    for (i = 0u; i < count; ++i) {
        uint32_t text_length, image_length;
        int32_t width, height, image_index = I_IMAGENONE_VALUE;
        HTREEITEM parent = TVI_ROOT_VALUE;
        TVINSERTSTRUCTW ins;
        LRESULT added;
        const uint8_t *image_bytes;
        HBITMAP bitmap;

        text_length = spw_read_u32_le(blob + offset);
        width = (int32_t)spw_read_u32_le(blob + offset + 4u);
        height = (int32_t)spw_read_u32_le(blob + offset + 8u);
        image_length = spw_read_u32_le(blob + offset + 12u);
        offset += 16u;
        status = spw_utf8_to_wide((const char *)(blob + offset), text_length,
                                  g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
        if (status != SPW_STATUS_OK) break;
        offset += text_length;
        image_bytes = blob + offset;

        if (parents[i] > 0) {
            parent = spw_tree_find_token(object, parents[i]);
            if (parent == NULL) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        }
        if (image_length > 0u) {
            bitmap = spw_create_padded_bitmap_bgra(image_bytes, image_length, width, height,
                                                    max_width, max_height);
            if (bitmap == NULL) { status = SPW_STATUS_WIN32_ERROR; break; }
            image_index = ImageList_Add(new_list, bitmap, NULL);
            DeleteObject((HGDIOBJ)bitmap);
            if (image_index < 0) { status = SPW_STATUS_WIN32_ERROR; break; }
        }

        ins.hParent = parent;
        ins.hInsertAfter = TVI_LAST_VALUE;
        ins.item.mask = TVIF_TEXT_VALUE | TVIF_PARAM_VALUE | TVIF_CHILDREN_VALUE;
        if (new_list != NULL)
            ins.item.mask |= TVIF_IMAGE_VALUE | TVIF_SELECTEDIMAGE_VALUE;
        ins.item.hItem = NULL;
        ins.item.state = 0u;
        ins.item.stateMask = 0u;
        ins.item.pszText = g_control_text_wide;
        ins.item.cchTextMax = 0;
        ins.item.iImage = image_index;
        ins.item.iSelectedImage = image_index;
        ins.item.cChildren = 0;
        ins.item.lParam = (LPARAM)(i + 1);
        added = SendMessageW(spw_tree_hwnd(object), TVM_INSERTITEMW_VALUE, 0, (LPARAM)(uintptr_t)&ins);
        if (added == 0) { status = SPW_STATUS_WIN32_ERROR; break; }
        offset += image_length;
    }
    object->suppress_notifications -= 1u;
    if (status != SPW_STATUS_OK) {
        SendMessageW(spw_tree_hwnd(object), TVM_DELETEITEM_VALUE, 0, (LPARAM)(uintptr_t)TVI_ROOT_VALUE);
        spw_tree_release_image_list(object);
    }
    if (object->kind == SPW_CONTROL_TREE_COLUMN) spw_tree_column_layout(object);
    return status;
}

static int32_t spw_tree_token_for_item(SpwObject *object, HTREEITEM item) {
    TVITEMW query;
    if (object == NULL || !spw_control_is_tree(object->kind)) return SPW_STATUS_NOT_FOUND;
    if (item == NULL) return 0;
    query.mask = TVIF_PARAM_VALUE;
    query.hItem = item;
    query.state = 0u;
    query.stateMask = 0u;
    query.pszText = NULL;
    query.cchTextMax = 0;
    query.iImage = 0;
    query.iSelectedImage = 0;
    query.cChildren = 0;
    query.lParam = 0;
    if (!SendMessageW(spw_tree_hwnd(object), TVM_GETITEMW_VALUE, 0, (LPARAM)(uintptr_t)&query))
        return SPW_STATUS_WIN32_ERROR;
    return (int32_t)query.lParam;
}

static int32_t spw_tree_selected_token(SpwObject *object) {
    HTREEITEM item;
    if (object == NULL || !spw_control_is_tree(object->kind)) return SPW_STATUS_NOT_FOUND;
    item = (HTREEITEM)(uintptr_t)SendMessageW(spw_tree_hwnd(object), TVM_GETNEXTITEM_VALUE,
                                              TVGN_CARET_VALUE, 0);
    return spw_tree_token_for_item(object, item);
}
static HTREEITEM spw_tree_column_visible_item_at(SpwObject *object, int32_t index) {
    HTREEITEM item;
    int32_t current = 0;
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN || index < 0 || object->tree_hwnd == NULL)
        return NULL;
    item = (HTREEITEM)(uintptr_t)SendMessageW(object->tree_hwnd, TVM_GETNEXTITEM_VALUE, TVGN_ROOT_VALUE, 0);
    while (item != NULL && current < index) {
        item = (HTREEITEM)(uintptr_t)SendMessageW(object->tree_hwnd, TVM_GETNEXTITEM_VALUE,
                                                  TVGN_NEXTVISIBLE_VALUE, (LPARAM)(uintptr_t)item);
        current += 1;
    }
    return item;
}

static int32_t spw_tree_column_visible_index_for_item(SpwObject *object, HTREEITEM sought) {
    HTREEITEM item;
    int32_t index = 0;
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN || sought == NULL || object->tree_hwnd == NULL)
        return -1;
    item = (HTREEITEM)(uintptr_t)SendMessageW(object->tree_hwnd, TVM_GETNEXTITEM_VALUE, TVGN_ROOT_VALUE, 0);
    while (item != NULL) {
        if (item == sought) return index;
        item = (HTREEITEM)(uintptr_t)SendMessageW(object->tree_hwnd, TVM_GETNEXTITEM_VALUE,
                                                  TVGN_NEXTVISIBLE_VALUE, (LPARAM)(uintptr_t)item);
        index += 1;
    }
    return -1;
}

static void spw_tree_column_select_table_row(SpwObject *object, int32_t index) {
    LVITEMW state;
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN || object->table_hwnd == NULL) return;
    state.mask = LVIF_STATE_VALUE;
    state.iItem = 0;
    state.iSubItem = 0;
    state.stateMask = LVIS_SELECTED_VALUE;
    state.pszText = NULL;
    state.cchTextMax = 0;
    state.iImage = 0;
    state.lParam = 0;
    object->suppress_notifications += 1u;
    state.state = 0u;
    SendMessageW(object->table_hwnd, LVM_SETITEMSTATE_VALUE, (WPARAM)(intptr_t)-1, (LPARAM)(uintptr_t)&state);
    if (index >= 0) {
        state.state = LVIS_SELECTED_VALUE;
        state.iItem = index;
        SendMessageW(object->table_hwnd, LVM_SETITEMSTATE_VALUE, (WPARAM)index, (LPARAM)(uintptr_t)&state);
    }
    object->suppress_notifications -= 1u;
}

static void spw_tree_column_sync_selection_from_tree(SpwObject *object) {
    HTREEITEM item;
    int32_t index;
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN) return;
    item = (HTREEITEM)(uintptr_t)SendMessageW(object->tree_hwnd, TVM_GETNEXTITEM_VALUE, TVGN_CARET_VALUE, 0);
    index = spw_tree_column_visible_index_for_item(object, item);
    spw_tree_column_select_table_row(object, index);
}

static void spw_tree_column_sync_tree_from_table_row(SpwObject *object, int32_t index) {
    HTREEITEM item;
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN) return;
    item = spw_tree_column_visible_item_at(object, index);
    if (item == NULL) return;
    object->suppress_notifications += 1u;
    SendMessageW(object->tree_hwnd, TVM_SELECTITEM_VALUE, TVGN_CARET_VALUE, (LPARAM)(uintptr_t)item);
    object->suppress_notifications -= 1u;
}

static void spw_tree_column_sync_row_height(SpwObject *object) {
    RECT item_rect;
    int32_t height;
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN || object->table_hwnd == NULL || object->tree_hwnd == NULL)
        return;
    item_rect.left = (int32_t)LVIR_BOUNDS_VALUE;
    if (SendMessageW(object->table_hwnd, LVM_GETITEMRECT_VALUE, 0u, (LPARAM)(uintptr_t)&item_rect)) {
        height = item_rect.bottom - item_rect.top;
        if (height > 0) SendMessageW(object->tree_hwnd, TVM_SETITEMHEIGHT_VALUE, (WPARAM)height, 0);
    }
}

static void spw_tree_column_sync_scroll_from_table(SpwObject *object) {
    LRESULT top;
    HTREEITEM item;
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN || object->table_hwnd == NULL || object->tree_hwnd == NULL)
        return;
    top = SendMessageW(object->table_hwnd, LVM_GETTOPINDEX_VALUE, 0u, 0);
    if (top < 0) return;
    item = spw_tree_column_visible_item_at(object, (int32_t)top);
    if (item != NULL)
        SendMessageW(object->tree_hwnd, TVM_SELECTITEM_VALUE, TVGN_FIRSTVISIBLE_VALUE, (LPARAM)(uintptr_t)item);
}

static void spw_tree_column_layout(SpwObject *object) {
    RECT client, header_rect, item_rect;
    HWND header;
    int32_t width, height, header_height = 0, first_width = 120;
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN || object->hwnd == NULL ||
        object->tree_hwnd == NULL || object->table_hwnd == NULL) return;
    if (!GetClientRect(object->hwnd, &client)) return;
    width = client.right - client.left;
    height = client.bottom - client.top;
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    MoveWindow(object->table_hwnd, 0, 0, width, height, TRUE_VALUE);
    if (object->table_headers_visible) {
        item_rect.left = (int32_t)LVIR_BOUNDS_VALUE;
        if (SendMessageW(object->table_hwnd, LVM_GETITEMRECT_VALUE, 0u, (LPARAM)(uintptr_t)&item_rect) && item_rect.top > 0) {
            header_height = item_rect.top;
        } else {
            header = (HWND)(uintptr_t)SendMessageW(object->table_hwnd, LVM_GETHEADER_VALUE, 0u, 0);
            if (header != NULL && GetWindowRect(header, &header_rect))
                header_height = header_rect.bottom - header_rect.top;
        }
    }
    first_width = (int32_t)SendMessageW(object->table_hwnd, LVM_GETCOLUMNWIDTH_VALUE, 0u, 0);
    if (first_width < 1) first_width = width;
    if (first_width > width) first_width = width;
    MoveWindow(object->tree_hwnd, 0, header_height, first_width,
               height > header_height ? height - header_height : 1, TRUE_VALUE);
    SetWindowPos(object->tree_hwnd, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE_VALUE | SWP_NOSIZE_VALUE | SWP_NOACTIVATE_VALUE);
    spw_tree_column_sync_row_height(object);
    spw_tree_column_sync_scroll_from_table(object);
}

static int spw_tree_column_handle_notify(SpwObject *object, NMHDR *header, LPARAM lParam) {
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN || header == NULL || object->suppress_notifications != 0u)
        return 0;
    if (header->hwndFrom == object->tree_hwnd) {
        if ((int32_t)header->code == TVN_SELCHANGEDW_VALUE) {
            int32_t token = spw_tree_selected_token(object);
            spw_tree_column_sync_selection_from_tree(object);
            if (token > 0) spw_push_event(SPW_EVENT_TREE_SELECTION_CHANGED, object->id, token, 0);
            return 1;
        }
        if ((int32_t)header->code == TVN_ITEMEXPANDEDW_VALUE) {
            NMTREEVIEWW *tv = (NMTREEVIEWW *)(uintptr_t)lParam;
            int32_t token = spw_tree_token_for_item(object, tv->itemNew.hItem);
            int64_t expanded = (tv->itemNew.state & TVIS_EXPANDED_VALUE) ? 1 : 0;
            if (token > 0) spw_push_event(SPW_EVENT_TREE_EXPANSION_CHANGED, object->id, token, expanded);
            return 1;
        }
        if ((int32_t)header->code == NM_DBLCLK_VALUE) {
            int32_t token = spw_tree_selected_token(object);
            if (token > 0) spw_push_event(SPW_EVENT_ACTIVATED, object->id, token, 1);
            return 1;
        }
    }
    if (header->hwndFrom == object->table_hwnd) {
        if ((int32_t)header->code == LVN_ITEMCHANGED_VALUE) {
            NMLISTVIEW *change = (NMLISTVIEW *)(uintptr_t)lParam;
            if ((change->uChanged & LVIF_STATE_VALUE) != 0u &&
                ((change->uOldState ^ change->uNewState) & LVIS_SELECTED_VALUE) != 0u &&
                (change->uNewState & LVIS_SELECTED_VALUE) != 0u && change->iItem >= 0) {
                HTREEITEM item = spw_tree_column_visible_item_at(object, change->iItem);
                int32_t token = spw_tree_token_for_item(object, item);
                spw_tree_column_sync_tree_from_table_row(object, change->iItem);
                if (token > 0) spw_push_event(SPW_EVENT_TREE_SELECTION_CHANGED, object->id, token, 0);
            }
            return 1;
        }
        if ((int32_t)header->code == NM_DBLCLK_VALUE) {
            NMITEMACTIVATE *activation = (NMITEMACTIVATE *)(uintptr_t)lParam;
            if (activation->iItem >= 0) {
                HTREEITEM item = spw_tree_column_visible_item_at(object, activation->iItem);
                int32_t token = spw_tree_token_for_item(object, item);
                if (token > 0) spw_push_event(SPW_EVENT_ACTIVATED, object->id, token, 1);
            }
            return 1;
        }
        if ((int32_t)header->code == LVN_COLUMNCLICK_VALUE) {
            NMLISTVIEW *click = (NMLISTVIEW *)(uintptr_t)lParam;
            spw_push_event(SPW_EVENT_COLUMN_CLICKED, object->id, (int64_t)click->iSubItem + 1, 0);
            return 1;
        }
    }
    return 0;
}

static LRESULT WINAPI spw_tree_column_child_subclass_proc(HWND hwnd, UINT message, WPARAM wParam,
                                                            LPARAM lParam, uintptr_t subclass_id,
                                                            uintptr_t ref_data) {
    SpwObject *object = spw_find_object_by_id((uint64_t)ref_data);
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN)
        return DefSubclassProc(hwnd, message, wParam, lParam);
    if (message == WM_SETFOCUS_VALUE) {
        spw_push_event(SPW_EVENT_FOCUS_RECEIVED, object->id, 0, 0);
    } else if (message == WM_KILLFOCUS_VALUE) {
        HWND next = (HWND)(uintptr_t)wParam;
        if (next != object->tree_hwnd && next != object->table_hwnd)
            spw_push_event(SPW_EVENT_FOCUS_LOST, object->id, 0, 0);
    } else if (message == WM_KEYDOWN_VALUE || message == WM_SYSKEYDOWN_VALUE) {
        spw_push_event(SPW_EVENT_KEY_DOWN, object->id, (int64_t)(uint32_t)wParam, (int64_t)spw_modifier_flags());
        if (message == WM_KEYDOWN_VALUE && (uint32_t)wParam == VK_RETURN_VALUE) {
            int32_t token = spw_tree_selected_token(object);
            if (token > 0) spw_push_event(SPW_EVENT_ACTIVATED, object->id, token, 2);
        }
    } else if (message == WM_KEYUP_VALUE || message == WM_SYSKEYUP_VALUE) {
        spw_push_event(SPW_EVENT_KEY_UP, object->id, (int64_t)(uint32_t)wParam, (int64_t)spw_modifier_flags());
    }
    if (hwnd == object->tree_hwnd && message == WM_MOUSEWHEEL_VALUE && object->table_hwnd != NULL) {
        LRESULT result = SendMessageW(object->table_hwnd, message, wParam, lParam);
        spw_tree_column_sync_scroll_from_table(object);
        return result;
    }
    if (hwnd == object->table_hwnd && (message == WM_VSCROLL_VALUE || message == WM_MOUSEWHEEL_VALUE)) {
        LRESULT result = DefSubclassProc(hwnd, message, wParam, lParam);
        spw_tree_column_sync_scroll_from_table(object);
        return result;
    }
    if (hwnd == object->table_hwnd && message == WM_NOTIFY_VALUE) {
        LRESULT result = DefSubclassProc(hwnd, message, wParam, lParam);
        spw_tree_column_layout(object);
        return result;
    }
    if (message == WM_NCDESTROY_VALUE)
        RemoveWindowSubclass(hwnd, spw_tree_column_child_subclass_proc, subclass_id);
    return DefSubclassProc(hwnd, message, wParam, lParam);
}

static int32_t spw_tree_column_create_children(SpwObject *object) {
    RECT client;
    DWORD table_style, tree_style;
    if (object == NULL || object->kind != SPW_CONTROL_TREE_COLUMN || object->hwnd == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    if (!GetClientRect(object->hwnd, &client)) return SPW_STATUS_WIN32_ERROR;
    table_style = WS_CHILD_VALUE | WS_VISIBLE_VALUE | WS_TABSTOP_VALUE |
                  LVS_REPORT_VALUE | LVS_SINGLESEL_VALUE | LVS_SHOWSELALWAYS_VALUE;
    tree_style = WS_CHILD_VALUE | WS_VISIBLE_VALUE | WS_TABSTOP_VALUE |
                 TVS_HASBUTTONS_VALUE | TVS_HASLINES_VALUE | TVS_LINESATROOT_VALUE |
                 TVS_SHOWSELALWAYS_VALUE | TVS_NOSCROLL_VALUE;
    object->table_hwnd = CreateWindowExW(0u, g_list_view_class_name, L"", table_style,
        0, 0, client.right - client.left, client.bottom - client.top,
        object->hwnd, NULL, g_instance, NULL);
    if (object->table_hwnd == NULL) return SPW_STATUS_WIN32_ERROR;
    object->tree_hwnd = CreateWindowExW(0u, g_tree_view_class_name, L"", tree_style,
        0, 0, client.right - client.left, client.bottom - client.top,
        object->hwnd, NULL, g_instance, NULL);
    if (object->tree_hwnd == NULL) {
        DestroyWindow(object->table_hwnd); object->table_hwnd = NULL;
        return SPW_STATUS_WIN32_ERROR;
    }
    SendMessageW(object->table_hwnd, WM_SETFONT_VALUE,
        (WPARAM)(uintptr_t)GetStockObject(DEFAULT_GUI_FONT_VALUE), (LPARAM)TRUE_VALUE);
    SendMessageW(object->tree_hwnd, WM_SETFONT_VALUE,
        (WPARAM)(uintptr_t)GetStockObject(DEFAULT_GUI_FONT_VALUE), (LPARAM)TRUE_VALUE);
    SendMessageW(object->table_hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE_VALUE,
        (WPARAM)(LVS_EX_FULLROWSELECT_VALUE | LVS_EX_DOUBLEBUFFER_VALUE),
        (LPARAM)(LVS_EX_FULLROWSELECT_VALUE | LVS_EX_DOUBLEBUFFER_VALUE));
    if (!SetWindowSubclass(object->table_hwnd, spw_tree_column_child_subclass_proc, 1u, (uintptr_t)object->id) ||
        !SetWindowSubclass(object->tree_hwnd, spw_tree_column_child_subclass_proc, 2u, (uintptr_t)object->id)) {
        DestroyWindow(object->tree_hwnd); object->tree_hwnd = NULL;
        DestroyWindow(object->table_hwnd); object->table_hwnd = NULL;
        return SPW_STATUS_WIN32_ERROR;
    }
    spw_tree_column_layout(object);
    return SPW_STATUS_OK;
}

static void spw_list_clear_selection(SpwObject *object) {
    LVITEMW state;
    state.mask = LVIF_STATE_VALUE;
    state.iItem = 0;
    state.iSubItem = 0;
    state.state = 0u;
    state.stateMask = LVIS_SELECTED_VALUE;
    state.pszText = NULL;
    state.cchTextMax = 0;
    state.iImage = 0;
    state.lParam = 0;
    SendMessageW(object->hwnd, LVM_SETITEMSTATE_VALUE, (WPARAM)(intptr_t)-1, (LPARAM)(uintptr_t)&state);
}

static int32_t spw_list_set_selected_indexes(SpwObject *object, const int32_t *indexes, uint32_t count) {
    uint32_t i;
    LVITEMW state;
    if (object == NULL || !spw_control_is_list_view(object->kind)) return SPW_STATUS_NOT_FOUND;
    if (count > 0u && indexes == NULL) return SPW_STATUS_BAD_ARGUMENT;
    object->suppress_notifications += 1u;
    spw_list_clear_selection(object);
    state.mask = LVIF_STATE_VALUE;
    state.iSubItem = 0;
    state.state = LVIS_SELECTED_VALUE;
    state.stateMask = LVIS_SELECTED_VALUE;
    state.pszText = NULL;
    state.cchTextMax = 0;
    state.iImage = 0;
    state.lParam = 0;
    for (i = 0u; i < count; ++i) {
        if (indexes[i] <= 0) continue;
        state.iItem = indexes[i] - 1;
        SendMessageW(object->hwnd, LVM_SETITEMSTATE_VALUE, (WPARAM)state.iItem, (LPARAM)(uintptr_t)&state);
        if ((GetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE) & (LONG_PTR)LVS_SINGLESEL_VALUE) != 0)
            break;
    }
    object->suppress_notifications -= 1u;
    return SPW_STATUS_OK;
}

static uint32_t spw_list_get_selected_indexes(SpwObject *object, int32_t *out_indexes, uint32_t capacity) {
    int32_t current = -1;
    uint32_t count = 0u;
    if (object == NULL || !spw_control_is_list_view(object->kind)) return 0u;
    for (;;) {
        current = (int32_t)SendMessageW(object->hwnd, LVM_GETNEXTITEM_VALUE,
                                       (WPARAM)(intptr_t)current, (LPARAM)LVNI_SELECTED_VALUE);
        if (current < 0) break;
        if (out_indexes != NULL && count < capacity)
            out_indexes[count] = current + 1;
        count += 1u;
    }
    return count;
}

static int32_t spw_list_set_multiple_selection(SpwObject *object, int32_t multiple) {
    LONG_PTR style;
    if (object == NULL || !spw_control_is_list_view(object->kind)) return SPW_STATUS_NOT_FOUND;
    style = GetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE);
    if (multiple)
        style &= ~((LONG_PTR)LVS_SINGLESEL_VALUE);
    else
        style |= (LONG_PTR)LVS_SINGLESEL_VALUE;
    SetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE, style);
    return SPW_STATUS_OK;
}

static int32_t spw_list_set_header(SpwObject *object, const char *text, uint32_t text_length, int32_t visible) {
    LVCOLUMNW column;
    LONG_PTR style;
    int32_t status;
    if (object == NULL || !spw_control_is_list(object->kind)) return SPW_STATUS_NOT_FOUND;
    status = spw_utf8_to_wide(text, text_length, g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
    if (status != SPW_STATUS_OK) return status;
    column.mask = LVCF_TEXT_VALUE;
    column.fmt = LVCFMT_LEFT_VALUE;
    column.cx = 0;
    column.pszText = g_control_text_wide;
    column.cchTextMax = 0;
    column.iSubItem = 0;
    SendMessageW(object->hwnd, LVM_SETCOLUMNW_VALUE, 0u, (LPARAM)(uintptr_t)&column);
    style = GetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE);
    if (visible) style &= ~((LONG_PTR)LVS_NOCOLUMNHEADER_VALUE);
    else style |= (LONG_PTR)LVS_NOCOLUMNHEADER_VALUE;
    SetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE, style);
    InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
    return SPW_STATUS_OK;
}

static void spw_table_apply_column_widths(SpwObject *object, int32_t control_width) {
    uint32_t i;
    int32_t base_total = 0;
    int32_t expandable_count = 0;
    int32_t available;
    int32_t extra;
    if (object == NULL || !spw_control_has_table_surface(object->kind) || object->table_column_count == 0u)
        return;
    available = control_width > 8 ? control_width - 8 : 1;
    for (i = 0u; i < object->table_column_count; ++i) {
        int32_t width = object->table_column_widths[i] > 0 ? object->table_column_widths[i] : 80;
        base_total += width;
        if (object->table_column_expandables[i]) expandable_count += 1;
    }
    extra = available > base_total ? available - base_total : 0;
    for (i = 0u; i < object->table_column_count; ++i) {
        int32_t width = object->table_column_widths[i] > 0 ? object->table_column_widths[i] : 80;
        if (expandable_count > 0 && object->table_column_expandables[i]) {
            int32_t share = extra / expandable_count;
            width += share;
            extra -= share;
            expandable_count -= 1;
        }
        SendMessageW(spw_table_hwnd(object), LVM_SETCOLUMNWIDTH_VALUE, (WPARAM)i, (LPARAM)width);
    }
}

static int32_t spw_table_set_columns(SpwObject *object, const char *blob, uint32_t blob_length,
                                     const int32_t *widths, const int32_t *alignments,
                                     const int32_t *expandables, uint32_t column_count,
                                     int32_t headers_visible, int32_t resizable) {
    const uint8_t *bytes = (const uint8_t *)blob;
    uint32_t offset = 0u;
    uint32_t i;
    RECT rect;
    LONG_PTR style;
    HWND header;
    int32_t status = SPW_STATUS_OK;
    if (object == NULL || !spw_control_has_table_surface(object->kind)) return SPW_STATUS_NOT_FOUND;
    if (column_count == 0u || column_count > SPW_MAX_TABLE_COLUMNS) return SPW_STATUS_BAD_ARGUMENT;
    if (blob == NULL || widths == NULL || alignments == NULL || expandables == NULL) return SPW_STATUS_BAD_ARGUMENT;
    object->suppress_notifications += 1u;
    while (SendMessageW(spw_table_hwnd(object), LVM_DELETECOLUMN_VALUE, 0u, 0) != 0) { }
    for (i = 0u; i < column_count; ++i) {
        uint32_t title_length;
        LVCOLUMNW column;
        int32_t fmt;
        if (blob_length - offset < 4u) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        title_length = spw_read_u32_le(bytes + offset);
        offset += 4u;
        if (title_length > blob_length - offset) { status = SPW_STATUS_BAD_ARGUMENT; break; }
        status = spw_utf8_to_wide((const char *)(bytes + offset), title_length,
                                  g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
        if (status != SPW_STATUS_OK) break;
        fmt = alignments[i] == 1 ? (int32_t)LVCFMT_RIGHT_VALUE
             : alignments[i] == 2 ? (int32_t)LVCFMT_CENTER_VALUE
             : (int32_t)LVCFMT_LEFT_VALUE;
        column.mask = LVCF_FMT_VALUE | LVCF_WIDTH_VALUE | LVCF_TEXT_VALUE | LVCF_SUBITEM_VALUE;
        column.fmt = fmt;
        column.cx = widths[i] > 0 ? widths[i] : 80;
        column.pszText = g_control_text_wide;
        column.cchTextMax = 0;
        column.iSubItem = (int32_t)i;
        if (SendMessageW(spw_table_hwnd(object), LVM_INSERTCOLUMNW_VALUE, (WPARAM)i,
                         (LPARAM)(uintptr_t)&column) < 0) {
            status = SPW_STATUS_WIN32_ERROR;
            break;
        }
        object->table_column_widths[i] = column.cx;
        object->table_column_expandables[i] = expandables[i] ? 1 : 0;
        offset += title_length;
    }
    if (status == SPW_STATUS_OK && offset != blob_length) status = SPW_STATUS_BAD_ARGUMENT;
    if (status == SPW_STATUS_OK) object->table_column_count = column_count;
    if (status == SPW_STATUS_OK) object->table_headers_visible = headers_visible ? 1u : 0u;
    style = GetWindowLongPtrW(spw_table_hwnd(object), GWL_STYLE_VALUE);
    if (headers_visible) style &= ~((LONG_PTR)LVS_NOCOLUMNHEADER_VALUE);
    else style |= (LONG_PTR)LVS_NOCOLUMNHEADER_VALUE;
    SetWindowLongPtrW(spw_table_hwnd(object), GWL_STYLE_VALUE, style);
    header = (HWND)(uintptr_t)SendMessageW(spw_table_hwnd(object), LVM_GETHEADER_VALUE, 0u, 0);
    if (header != NULL) {
        LONG_PTR header_style = GetWindowLongPtrW(header, GWL_STYLE_VALUE);
        if (resizable) header_style &= ~((LONG_PTR)HDS_NOSIZING_VALUE);
        else header_style |= (LONG_PTR)HDS_NOSIZING_VALUE;
        SetWindowLongPtrW(header, GWL_STYLE_VALUE, header_style);
    }
    if (status == SPW_STATUS_OK && GetClientRect(spw_table_hwnd(object), &rect))
        spw_table_apply_column_widths(object, rect.right - rect.left);
    InvalidateRect(spw_table_hwnd(object), NULL, TRUE_VALUE);
    if (object->kind == SPW_CONTROL_TREE_COLUMN) spw_tree_column_layout(object);
    object->suppress_notifications -= 1u;
    return status;
}

static int32_t spw_table_set_cells(SpwObject *object, const char *blob, uint32_t blob_length,
                                   uint32_t row_count, uint32_t column_count) {
    const uint8_t *bytes = (const uint8_t *)blob;
    uint32_t offset = 0u;
    uint32_t row;
    uint32_t column_index;
    int32_t status = SPW_STATUS_OK;
    if (object == NULL || !spw_control_has_table_surface(object->kind)) return SPW_STATUS_NOT_FOUND;
    if (column_count == 0u || column_count != object->table_column_count) return SPW_STATUS_BAD_ARGUMENT;
    if (row_count > 0u && blob == NULL) return SPW_STATUS_BAD_ARGUMENT;
    object->suppress_notifications += 1u;
    spw_table_release_image_list(object);
    SendMessageW(spw_table_hwnd(object), LVM_SETEXTENDEDLISTVIEWSTYLE_VALUE,
                 (WPARAM)LVS_EX_SUBITEMIMAGES_VALUE, 0);
    SendMessageW(spw_table_hwnd(object), LVM_DELETEALLITEMS_VALUE, 0u, 0);
    for (row = 0u; row < row_count && status == SPW_STATUS_OK; ++row) {
        for (column_index = 0u; column_index < column_count; ++column_index) {
            uint32_t cell_length;
            LVITEMW item;
            LRESULT result;
            if (blob_length - offset < 4u) { status = SPW_STATUS_BAD_ARGUMENT; break; }
            cell_length = spw_read_u32_le(bytes + offset);
            offset += 4u;
            if (cell_length > blob_length - offset) { status = SPW_STATUS_BAD_ARGUMENT; break; }
            status = spw_utf8_to_wide((const char *)(bytes + offset), cell_length,
                                      g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
            if (status != SPW_STATUS_OK) break;
            item.mask = LVIF_TEXT_VALUE;
            item.iItem = (int32_t)row;
            item.iSubItem = (int32_t)column_index;
            item.state = 0u;
            item.stateMask = 0u;
            item.pszText = g_control_text_wide;
            item.cchTextMax = 0;
            item.iImage = -1;
            item.lParam = 0;
            if (column_index == 0u)
                result = SendMessageW(spw_table_hwnd(object), LVM_INSERTITEMW_VALUE, 0u, (LPARAM)(uintptr_t)&item);
            else
                result = SendMessageW(spw_table_hwnd(object), LVM_SETITEMW_VALUE, 0u, (LPARAM)(uintptr_t)&item);
            if (result < 0 && column_index == 0u) { status = SPW_STATUS_WIN32_ERROR; break; }
            offset += cell_length;
        }
    }
    if (status == SPW_STATUS_OK && offset != blob_length) status = SPW_STATUS_BAD_ARGUMENT;
    if (object->kind == SPW_CONTROL_TREE_COLUMN) spw_tree_column_layout(object);
    object->suppress_notifications -= 1u;
    return status;
}


/* Packed cell format: u32 UTF-8 length, i32 image width, i32 image height,
 * u32 BGRA byte length, followed by UTF-8 bytes and top-down BGRA bytes.
 * Cells are row-major. One shared small image list serves all subitems. */
static int32_t spw_table_apply_cells_with_images(SpwObject *object, const uint8_t *blob,
                                                uint32_t blob_length, uint32_t row_count,
                                                uint32_t column_count) {
    uint32_t offset = 0u, cell, cell_count, row, column_index;
    int32_t max_width = 0, max_height = 0;
    int32_t status = SPW_STATUS_OK;
    HIMAGELIST new_list = NULL;
    uint64_t count64 = (uint64_t)row_count * (uint64_t)column_count;

    if (object == NULL || !spw_control_has_table_surface(object->kind)) return SPW_STATUS_NOT_FOUND;
    if (column_count == 0u || column_count != object->table_column_count) return SPW_STATUS_BAD_ARGUMENT;
    if (count64 > 0xffffffffu) return SPW_STATUS_BAD_ARGUMENT;
    cell_count = (uint32_t)count64;
    if (cell_count > 0u && blob == NULL) return SPW_STATUS_BAD_ARGUMENT;

    for (cell = 0u; cell < cell_count; ++cell) {
        uint32_t text_length, image_length;
        int32_t width, height;
        uint64_t expected;
        if (blob_length - offset < 16u) return SPW_STATUS_BAD_ARGUMENT;
        text_length = spw_read_u32_le(blob + offset);
        width = (int32_t)spw_read_u32_le(blob + offset + 4u);
        height = (int32_t)spw_read_u32_le(blob + offset + 8u);
        image_length = spw_read_u32_le(blob + offset + 12u);
        offset += 16u;
        if (text_length > blob_length - offset) return SPW_STATUS_BAD_ARGUMENT;
        offset += text_length;
        if (image_length > blob_length - offset) return SPW_STATUS_BAD_ARGUMENT;
        if (image_length == 0u) {
            if (width != 0 || height != 0) return SPW_STATUS_BAD_ARGUMENT;
        } else {
            if (width <= 0 || height <= 0 || width > 256 || height > 256) return SPW_STATUS_BAD_ARGUMENT;
            expected = (uint64_t)(uint32_t)width * (uint64_t)(uint32_t)height * 4u;
            if (expected != (uint64_t)image_length) return SPW_STATUS_BAD_ARGUMENT;
            if (width > max_width) max_width = width;
            if (height > max_height) max_height = height;
        }
        offset += image_length;
    }
    if (offset != blob_length) return SPW_STATUS_BAD_ARGUMENT;

    if (max_width > 0 && max_height > 0) {
        new_list = ImageList_Create(max_width, max_height, ILC_COLOR32_VALUE | ILC_MASK_VALUE,
                                    (int32_t)cell_count, 1);
        if (new_list == NULL) return SPW_STATUS_WIN32_ERROR;
    }

    object->suppress_notifications += 1u;
    spw_table_release_image_list(object);
    SendMessageW(spw_table_hwnd(object), LVM_SETEXTENDEDLISTVIEWSTYLE_VALUE,
                 (WPARAM)LVS_EX_SUBITEMIMAGES_VALUE,
                 new_list != NULL ? (LPARAM)LVS_EX_SUBITEMIMAGES_VALUE : 0);
    if (new_list != NULL) {
        SendMessageW(spw_table_hwnd(object), LVM_SETIMAGELIST_VALUE, (WPARAM)LVSIL_SMALL_VALUE,
                     (LPARAM)(uintptr_t)new_list);
        if (object->kind == SPW_CONTROL_TREE_COLUMN) object->tree_column_table_image_list = new_list;
        else object->list_image_list = new_list;
    }
    SendMessageW(spw_table_hwnd(object), LVM_DELETEALLITEMS_VALUE, 0u, 0);

    offset = 0u;
    for (row = 0u; row < row_count && status == SPW_STATUS_OK; ++row) {
        for (column_index = 0u; column_index < column_count; ++column_index) {
            uint32_t text_length, image_length;
            int32_t width, height, image_index = -1;
            const uint8_t *image_bytes;
            HBITMAP bitmap;
            LVITEMW item;
            LRESULT result;

            text_length = spw_read_u32_le(blob + offset);
            width = (int32_t)spw_read_u32_le(blob + offset + 4u);
            height = (int32_t)spw_read_u32_le(blob + offset + 8u);
            image_length = spw_read_u32_le(blob + offset + 12u);
            offset += 16u;
            status = spw_utf8_to_wide((const char *)(blob + offset), text_length,
                                      g_control_text_wide, SPW_MAX_CONTROL_TEXT_WCHARS);
            if (status != SPW_STATUS_OK) break;
            offset += text_length;
            image_bytes = blob + offset;
            if (image_length > 0u) {
                bitmap = spw_create_padded_bitmap_bgra(image_bytes, image_length, width, height,
                                                        max_width, max_height);
                if (bitmap == NULL) { status = SPW_STATUS_WIN32_ERROR; break; }
                image_index = ImageList_Add(new_list, bitmap, NULL);
                DeleteObject((HGDIOBJ)bitmap);
                if (image_index < 0) { status = SPW_STATUS_WIN32_ERROR; break; }
            }
            item.mask = LVIF_TEXT_VALUE | (image_index >= 0 ? LVIF_IMAGE_VALUE : 0u);
            item.iItem = (int32_t)row;
            item.iSubItem = (int32_t)column_index;
            item.state = 0u;
            item.stateMask = 0u;
            item.pszText = g_control_text_wide;
            item.cchTextMax = 0;
            item.iImage = image_index;
            item.lParam = 0;
            if (column_index == 0u)
                result = SendMessageW(spw_table_hwnd(object), LVM_INSERTITEMW_VALUE, 0u, (LPARAM)(uintptr_t)&item);
            else
                result = SendMessageW(spw_table_hwnd(object), LVM_SETITEMW_VALUE, 0u, (LPARAM)(uintptr_t)&item);
            if (result < 0 && column_index == 0u) { status = SPW_STATUS_WIN32_ERROR; break; }
            offset += image_length;
        }
    }
    object->suppress_notifications -= 1u;
    if (status != SPW_STATUS_OK) {
        SendMessageW(spw_table_hwnd(object), LVM_DELETEALLITEMS_VALUE, 0u, 0);
        spw_table_release_image_list(object);
        SendMessageW(spw_table_hwnd(object), LVM_SETEXTENDEDLISTVIEWSTYLE_VALUE,
                     (WPARAM)LVS_EX_SUBITEMIMAGES_VALUE, 0);
    }
    if (object->kind == SPW_CONTROL_TREE_COLUMN) spw_tree_column_layout(object);
    return status;
}

