/* RichEdit-backed code surface ----------------------------------------- */
static COLORREF spw_code_colorref(uint32_t rgb) {
    return (COLORREF)(((rgb & 0x0000ffu) << 16) |
                      (rgb & 0x00ff00u) |
                      ((rgb & 0xff0000u) >> 16));
}

static int32_t spw_code_gutter_width(const SpwObject *object) {
    int32_t width = 4;
    if (object == NULL) return width;
    if (object->code_line_numbers) width = SPW_CODE_GUTTER_WIDTH;
    if (object->code_mark_count > 0u)
        width += SPW_CODE_MARK_GUTTER_WIDTH;
    return width;
}

static int32_t spw_code_ensure_loaded(void) {
    if (g_msftedit_module != NULL)
        return SPW_STATUS_OK;
    g_msftedit_module = LoadLibraryW(g_msftedit_library_name);
    return g_msftedit_module != NULL ? SPW_STATUS_OK : SPW_STATUS_WIN32_ERROR;
}

static void spw_code_copy_face_name(WCHAR *target) {
    static const WCHAR face[] = { 'C','o','n','s','o','l','a','s',0 };
    uint32_t i = 0u;
    while (i < 31u && face[i] != 0) {
        target[i] = face[i];
        i += 1u;
    }
    target[i] = 0;
}

static void spw_code_apply_base_format(SpwObject *object) {
    CHARFORMATW format = {0};
    if (object == NULL || object->hwnd == NULL || object->kind != SPW_CONTROL_CODE)
        return;
    format.cbSize = (UINT)sizeof(CHARFORMATW);
    format.dwMask = CFM_FACE_VALUE | CFM_SIZE_VALUE | CFM_COLOR_VALUE |
                    CFM_BOLD_VALUE | CFM_ITALIC_VALUE | CFM_UNDERLINE_VALUE;
    format.dwEffects = 0u;
    format.yHeight = 200; /* 10 pt, in twentieths of a point. */
    format.crTextColor = spw_code_colorref(object->code_foreground_rgb);
    spw_code_copy_face_name(format.szFaceName);
    SendMessageW(object->hwnd, EM_SETCHARFORMAT_VALUE,
        (WPARAM)SCF_ALL_VALUE, (LPARAM)(uintptr_t)&format);
    SendMessageW(object->hwnd, EM_SETBKGNDCOLOR_VALUE, 0u,
        (LPARAM)(uintptr_t)spw_code_colorref(object->code_background_rgb));
}

static void spw_code_apply_margin(SpwObject *object) {
    RECT rect;
    if (object == NULL || object->hwnd == NULL || object->kind != SPW_CONTROL_CODE)
        return;
    if (!GetClientRect(object->hwnd, &rect))
        return;
    /* RichEdit does not reliably honor EM_SETMARGINS for the text origin
       under Wine.  Reserve the gutter in the control's formatting rectangle
       instead.  Use EM_SETRECT rather than EM_SETRECTNP because RichEdit
       explicitly does not support the latter.  This also gives Windows and Wine identical clipping/wrapping
       semantics and can be asserted through EM_GETRECT in the native injector. */
    rect.left = spw_code_gutter_width(object);
    rect.top += 1;
    if (rect.right > rect.left + 2) rect.right -= 2;
    if (rect.bottom > rect.top + 1) rect.bottom -= 1;
    SendMessageW(object->hwnd, EM_SETRECT_VALUE, 0, (LPARAM)(uintptr_t)&rect);
}

static int32_t spw_code_initialize_control(SpwObject *object) {
    if (object == NULL || object->kind != SPW_CONTROL_CODE || object->hwnd == NULL)
        return SPW_STATUS_NOT_FOUND;
    object->code_line_numbers = 0u;
    object->code_foreground_rgb = 0x202020u;
    object->code_background_rgb = 0xffffffu;
    object->code_mark_count = 0u;
    spw_code_apply_base_format(object);
    spw_code_apply_margin(object);
    SendMessageW(object->hwnd, EM_SETEVENTMASK_VALUE, 0u,
        SendMessageW(object->hwnd, EM_GETEVENTMASK_VALUE, 0u, 0) |
            ENM_CHANGE_VALUE | ENM_SCROLL_VALUE);
    SendMessageW(object->hwnd, EM_SETTARGETDEVICE_VALUE, 0u, 1); /* no wrap */
    return SPW_STATUS_OK;
}

static int32_t spw_code_configure(SpwObject *object, int wrap_word,
                                  int scroll_bars, int undo_enabled) {
    LONG_PTR style;
    if (object == NULL || object->kind != SPW_CONTROL_CODE || object->hwnd == NULL)
        return SPW_STATUS_NOT_FOUND;
    style = GetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE);
    if (scroll_bars) style |= (LONG_PTR)WS_VSCROLL_VALUE;
    else style &= ~((LONG_PTR)WS_VSCROLL_VALUE);
    if (scroll_bars && !wrap_word) style |= (LONG_PTR)WS_HSCROLL_VALUE;
    else style &= ~((LONG_PTR)WS_HSCROLL_VALUE);
    SetWindowLongPtrW(object->hwnd, GWL_STYLE_VALUE, style);
    SetWindowPos(object->hwnd, NULL, 0, 0, 0, 0,
        SWP_NOMOVE_VALUE | SWP_NOSIZE_VALUE | SWP_NOZORDER_VALUE |
        SWP_NOACTIVATE_VALUE | SWP_FRAMECHANGED_VALUE);
    SendMessageW(object->hwnd, EM_SETTARGETDEVICE_VALUE, 0u,
        (LPARAM)(wrap_word ? 0 : 1));
    ShowScrollBar(object->hwnd, SB_VERT_VALUE, scroll_bars ? TRUE_VALUE : FALSE_VALUE);
    ShowScrollBar(object->hwnd, SB_HORZ_VALUE,
        (scroll_bars && !wrap_word) ? TRUE_VALUE : FALSE_VALUE);
    object->text_undo_enabled = undo_enabled ? 1u : 0u;
    if (!object->text_undo_enabled)
        SendMessageW(object->hwnd, EM_EMPTYUNDOBUFFER_VALUE, 0, 0);
    spw_code_apply_margin(object);
    InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
    return SPW_STATUS_OK;
}

static int32_t spw_code_set_line_numbers_impl(SpwObject *object, int enabled) {
    if (object == NULL || object->kind != SPW_CONTROL_CODE || object->hwnd == NULL)
        return SPW_STATUS_NOT_FOUND;
    object->code_line_numbers = enabled ? 1u : 0u;
    spw_code_apply_margin(object);
    InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
    return SPW_STATUS_OK;
}

static void spw_code_set_range_format(SpwObject *object, uint32_t start_units,
                                      uint32_t end_units, uint32_t rgb,
                                      uint32_t flags) {
    CHARFORMATW format = {0};
    format.cbSize = (UINT)sizeof(CHARFORMATW);
    format.dwMask = CFM_COLOR_VALUE | CFM_BOLD_VALUE |
                    CFM_ITALIC_VALUE | CFM_UNDERLINE_VALUE;
    format.crTextColor = spw_code_colorref(rgb);
    if ((flags & SPW_CODE_STYLE_BOLD) != 0u) format.dwEffects |= CFE_BOLD_VALUE;
    if ((flags & SPW_CODE_STYLE_ITALIC) != 0u) format.dwEffects |= CFE_ITALIC_VALUE;
    if ((flags & SPW_CODE_STYLE_UNDERLINE) != 0u) format.dwEffects |= CFE_UNDERLINE_VALUE;
    SendMessageW(object->hwnd, EM_SETSEL_VALUE, (WPARAM)start_units, (LPARAM)end_units);
    SendMessageW(object->hwnd, EM_SETCHARFORMAT_VALUE,
        (WPARAM)SCF_SELECTION_VALUE, (LPARAM)(uintptr_t)&format);
}

static int32_t spw_code_set_styles_impl(SpwObject *object, const int32_t *spans,
                                   uint32_t span_count, uint32_t foreground_rgb,
                                   uint32_t background_rgb) {
    uint32_t text_units;
    uint32_t selection_start = 0u;
    uint32_t selection_end = 0u;
    uint32_t i;
    int32_t status;
    if (object == NULL || object->kind != SPW_CONTROL_CODE || object->hwnd == NULL)
        return SPW_STATUS_NOT_FOUND;
    if (span_count > 0u && spans == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    if (span_count > 16384u)
        return SPW_STATUS_CAPACITY;
    status = spw_control_text_wide(object->hwnd, &text_units);
    if (status != SPW_STATUS_OK)
        return status;
    SendMessageW(object->hwnd, EM_GETSEL_VALUE,
        (WPARAM)(uintptr_t)&selection_start, (LPARAM)(uintptr_t)&selection_end);
    object->code_foreground_rgb = foreground_rgb & 0xffffffu;
    object->code_background_rgb = background_rgb & 0xffffffu;
    spw_code_apply_base_format(object);
    for (i = 0u; i < span_count; ++i) {
        const int32_t *entry = spans + (i * 4u);
        uint32_t start_offset;
        uint32_t end_offset;
        uint32_t start_units;
        uint32_t end_units;
        if (entry[0] < 0 || entry[1] < entry[0])
            continue;
        start_offset = (uint32_t)entry[0];
        end_offset = (uint32_t)entry[1];
        start_units = spw_text_area_utf16_units_for_offset(
            g_control_text_wide, text_units, start_offset);
        end_units = spw_text_area_utf16_units_for_offset(
            g_control_text_wide, text_units, end_offset);
        spw_code_set_range_format(object, start_units, end_units,
            (uint32_t)entry[2] & 0xffffffu, (uint32_t)entry[3]);
    }
    SendMessageW(object->hwnd, EM_SETSEL_VALUE,
        (WPARAM)selection_start, (LPARAM)selection_end);
    InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
    return SPW_STATUS_OK;
}

static void spw_code_set_range_background(SpwObject *object,
                                          uint32_t start_units,
                                          uint32_t end_units,
                                          uint32_t rgb) {
    CHARFORMAT2W format = {0};
    format.cbSize = (UINT)sizeof(CHARFORMAT2W);
    format.dwMask = CFM_BACKCOLOR_VALUE;
    format.crBackColor = spw_code_colorref(rgb);
    SendMessageW(object->hwnd, EM_SETSEL_VALUE, (WPARAM)start_units, (LPARAM)end_units);
    SendMessageW(object->hwnd, EM_SETCHARFORMAT_VALUE,
        (WPARAM)SCF_SELECTION_VALUE, (LPARAM)(uintptr_t)&format);
}

static int32_t spw_code_set_line_decorations_impl(SpwObject *object,
                                                   const int32_t *entries,
                                                   uint32_t entry_count) {
    uint32_t selection_start = 0u;
    uint32_t selection_end = 0u;
    uint32_t text_units;
    uint32_t i;
    if (object == NULL || object->kind != SPW_CONTROL_CODE || object->hwnd == NULL)
        return SPW_STATUS_NOT_FOUND;
    if (entry_count > 0u && entries == NULL)
        return SPW_STATUS_BAD_ARGUMENT;
    if (entry_count > SPW_MAX_CODE_MARKS)
        return SPW_STATUS_CAPACITY;

    text_units = (uint32_t)GetWindowTextLengthW(object->hwnd);
    SendMessageW(object->hwnd, EM_GETSEL_VALUE,
        (WPARAM)(uintptr_t)&selection_start, (LPARAM)(uintptr_t)&selection_end);

    /* Clear background formatting left by a previous patch before applying
       the new per-line decoration set. */
    spw_code_set_range_background(object, 0u, text_units,
        object->code_background_rgb);
    object->code_mark_count = 0u;

    for (i = 0u; i < entry_count; ++i) {
        const int32_t *entry = entries + (i * 4u);
        int32_t line = entry[0];
        LRESULT start_result;
        LRESULT end_result;
        uint32_t start_units;
        uint32_t end_units;
        uint32_t kind;
        if (line < 0) continue;
        start_result = SendMessageW(object->hwnd, EM_LINEINDEX_VALUE,
            (WPARAM)line, 0);
        if (start_result < 0) continue;
        end_result = SendMessageW(object->hwnd, EM_LINEINDEX_VALUE,
            (WPARAM)(line + 1), 0);
        start_units = (uint32_t)start_result;
        end_units = end_result >= 0 ? (uint32_t)end_result : text_units;
        if (end_units < start_units) end_units = start_units;
        if (end_units == start_units && start_units < text_units)
            end_units = start_units + 1u;
        spw_code_set_range_background(object, start_units, end_units,
            (uint32_t)entry[1] & 0xffffffu);

        kind = (uint32_t)entry[2];
        if (kind != 0u && object->code_mark_count < SPW_MAX_CODE_MARKS) {
            uint32_t mark_index = object->code_mark_count++;
            object->code_mark_lines[mark_index] = line;
            object->code_mark_kinds[mark_index] = kind;
            object->code_mark_rgbs[mark_index] = (uint32_t)entry[3] & 0xffffffu;
        }
    }

    SendMessageW(object->hwnd, EM_SETSEL_VALUE,
        (WPARAM)selection_start, (LPARAM)selection_end);
    spw_code_apply_margin(object);
    InvalidateRect(object->hwnd, NULL, TRUE_VALUE);
    return SPW_STATUS_OK;
}

static uint32_t spw_code_uint_to_wide(uint32_t value, WCHAR *buffer, uint32_t capacity) {
    WCHAR reversed[16];
    uint32_t count = 0u;
    uint32_t i;
    if (capacity < 2u) return 0u;
    do {
        reversed[count++] = (WCHAR)('0' + (value % 10u));
        value /= 10u;
    } while (value != 0u && count < 15u);
    if (count + 1u > capacity) return 0u;
    for (i = 0u; i < count; ++i)
        buffer[i] = reversed[count - i - 1u];
    buffer[count] = 0;
    return count;
}

static uint32_t spw_code_mark_for_line(const SpwObject *object, int32_t line,
                                       uint32_t *out_rgb) {
    uint32_t i;
    if (out_rgb != NULL) *out_rgb = 0x606060u;
    if (object == NULL) return 0u;
    for (i = 0u; i < object->code_mark_count; ++i) {
        if (object->code_mark_lines[i] == line) {
            if (out_rgb != NULL) *out_rgb = object->code_mark_rgbs[i];
            return object->code_mark_kinds[i];
        }
    }
    return 0u;
}

static WCHAR spw_code_mark_glyph(uint32_t kind) {
    if (kind == SPW_CODE_MARK_INSERT) return (WCHAR)'+';
    if (kind == SPW_CODE_MARK_DELETE) return (WCHAR)'-';
    if (kind == SPW_CODE_MARK_CHANGE) return (WCHAR)'~';
    return (WCHAR)'*';
}

static void spw_code_draw_gutter(SpwObject *object) {
    RECT client;
    RECT format_rect;
    RECT gutter;
    HDC dc;
    int32_t first_display_line;
    int32_t display_line_count;
    int32_t display_line;
    int32_t line_height = 16;
    int32_t mark_width;
    int32_t number_left;
    COLORREF old_color;
    int old_mode;
    HGDIOBJ old_font = NULL;
    HGDIOBJ rich_font;
    TEXTMETRICW metrics = {0};
    if (object == NULL || object->kind != SPW_CONTROL_CODE ||
        (!object->code_line_numbers && object->code_mark_count == 0u) ||
        object->hwnd == NULL)
        return;
    if (!GetClientRect(object->hwnd, &client))
        return;
    format_rect = client;
    SendMessageW(object->hwnd, EM_GETRECT_VALUE, 0,
        (LPARAM)(uintptr_t)&format_rect);
    gutter = client;
    gutter.right = spw_code_gutter_width(object) - 2;
    dc = GetDC(object->hwnd);
    if (dc == NULL)
        return;
    rich_font = (HGDIOBJ)(uintptr_t)SendMessageW(object->hwnd,
        WM_GETFONT_VALUE, 0, 0);
    if (rich_font != NULL)
        old_font = SelectObject(dc, rich_font);
    if (GetTextMetricsW(dc, &metrics) && metrics.tmHeight > 0)
        line_height = metrics.tmHeight + metrics.tmExternalLeading;
    if (line_height < 8) line_height = 8;
    FillRect(dc, &gutter, GetSysColorBrush(COLOR_BTNFACE_VALUE));
    old_mode = SetBkMode(dc, TRANSPARENT_VALUE);
    old_color = SetTextColor(dc, spw_code_colorref(0x606060u));
    first_display_line = (int32_t)SendMessageW(object->hwnd,
        EM_GETFIRSTVISIBLELINE_VALUE, 0, 0);
    display_line_count = (int32_t)SendMessageW(object->hwnd,
        EM_GETLINECOUNT_VALUE, 0, 0);
    mark_width = object->code_mark_count > 0u ? SPW_CODE_MARK_GUTTER_WIDTH : 0;
    number_left = mark_width;
    for (display_line = first_display_line;
         display_line < display_line_count;
         ++display_line) {
        int32_t position_y;
        RECT line_rect;
        position_y = format_rect.top +
            ((display_line - first_display_line) * line_height);
        if (position_y > client.bottom) break;
        line_rect.top = position_y;
        line_rect.bottom = position_y + line_height;
        if (object->code_mark_count > 0u) {
            uint32_t mark_rgb;
            uint32_t mark_kind = spw_code_mark_for_line(object, display_line, &mark_rgb);
            if (mark_kind != 0u) {
                WCHAR glyph = spw_code_mark_glyph(mark_kind);
                line_rect.left = 1;
                line_rect.right = SPW_CODE_MARK_GUTTER_WIDTH - 3;
                SetTextColor(dc, spw_code_colorref(mark_rgb));
                DrawTextW(dc, &glyph, 1, &line_rect,
                    DT_RIGHT_VALUE | DT_VCENTER_VALUE | DT_SINGLELINE_VALUE |
                    DT_NOPREFIX_VALUE);
            }
        }
        if (object->code_line_numbers) {
            WCHAR number[16];
            uint32_t length = spw_code_uint_to_wide((uint32_t)display_line + 1u,
                number, 16u);
            if (length != 0u) {
                line_rect.left = number_left + 2;
                line_rect.right = number_left + SPW_CODE_GUTTER_WIDTH - 7;
                SetTextColor(dc, spw_code_colorref(0x606060u));
                DrawTextW(dc, number, (int32_t)length, &line_rect,
                    DT_RIGHT_VALUE | DT_VCENTER_VALUE | DT_SINGLELINE_VALUE |
                    DT_NOPREFIX_VALUE);
            }
        }
    }
    SetTextColor(dc, old_color);
    SetBkMode(dc, old_mode);
    if (old_font != NULL) SelectObject(dc, old_font);
    ReleaseDC(object->hwnd, dc);
}

