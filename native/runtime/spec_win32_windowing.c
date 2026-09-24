static int32_t spw_scroll_max(int32_t content, int32_t page) {
    int32_t value = content - page;
    return value > 0 ? value : 0;
}

static int32_t spw_scroll_clamp(int32_t value, int32_t maximum) {
    if (value < 0) return 0;
    if (value > maximum) return maximum;
    return value;
}

static int32_t spw_scroll_view_apply(SpwObject *viewport) {
    SpwObject *content;
    RECT client;
    SCROLLINFO si;
    int32_t page_width, page_height, max_x, max_y;
    int32_t show_h = 0, show_v = 0;
    int32_t iteration;
    if (viewport == NULL || viewport->kind != SPW_CONTROL_SCROLL_VIEWPORT)
        return SPW_STATUS_NOT_FOUND;
    content = spw_find_object_by_id(viewport->scroll_content_id);
    if (content == NULL || content->kind != SPW_CONTROL_SCROLL_CONTENT)
        return SPW_STATUS_NOT_FOUND;

    /* Scroll policy is independent from scrollability.  AUTOMATIC matches
     * Gtk's automatic policy, HIDDEN keeps the adjustment/programmatic scroll
     * path active without painting a bar (needed by Miller), ALWAYS reserves
     * bar space unconditionally, and DISABLED clamps the axis to zero. */
    show_h = viewport->scroll_h_policy == SPW_SCROLL_POLICY_ALWAYS ? 1 : 0;
    show_v = viewport->scroll_v_policy == SPW_SCROLL_POLICY_ALWAYS ? 1 : 0;
    ShowScrollBar(viewport->hwnd, SB_HORZ_VALUE, show_h ? TRUE_VALUE : FALSE_VALUE);
    ShowScrollBar(viewport->hwnd, SB_VERT_VALUE, show_v ? TRUE_VALUE : FALSE_VALUE);
    for (iteration = 0; iteration < 3; ++iteration) {
        int32_t need_h, need_v;
        if (!GetClientRect(viewport->hwnd, &client))
            return SPW_STATUS_WIN32_ERROR;
        page_width = client.right - client.left;
        page_height = client.bottom - client.top;
        need_h = (viewport->scroll_h_policy == SPW_SCROLL_POLICY_ALWAYS) ||
                 (viewport->scroll_h_policy == SPW_SCROLL_POLICY_AUTOMATIC &&
                  viewport->scroll_content_width > page_width);
        need_v = (viewport->scroll_v_policy == SPW_SCROLL_POLICY_ALWAYS) ||
                 (viewport->scroll_v_policy == SPW_SCROLL_POLICY_AUTOMATIC &&
                  viewport->scroll_content_height > page_height);
        if (need_h == show_h && need_v == show_v) break;
        show_h = need_h;
        show_v = need_v;
        ShowScrollBar(viewport->hwnd, SB_HORZ_VALUE, show_h ? TRUE_VALUE : FALSE_VALUE);
        ShowScrollBar(viewport->hwnd, SB_VERT_VALUE, show_v ? TRUE_VALUE : FALSE_VALUE);
    }
    if (!GetClientRect(viewport->hwnd, &client))
        return SPW_STATUS_WIN32_ERROR;
    page_width = client.right - client.left;
    page_height = client.bottom - client.top;
    max_x = viewport->scroll_h_enabled ? spw_scroll_max(viewport->scroll_content_width, page_width) : 0;
    max_y = viewport->scroll_v_enabled ? spw_scroll_max(viewport->scroll_content_height, page_height) : 0;
    viewport->scroll_x = spw_scroll_clamp(viewport->scroll_x, max_x);
    viewport->scroll_y = spw_scroll_clamp(viewport->scroll_y, max_y);

    si.cbSize = (UINT)sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE_VALUE | SIF_PAGE_VALUE | SIF_POS_VALUE;
    si.nMin = 0;
    si.nMax = viewport->scroll_content_width > 0 ? viewport->scroll_content_width - 1 : 0;
    si.nPage = page_width > 0 ? (UINT)page_width : 1u;
    si.nPos = viewport->scroll_x;
    si.nTrackPos = 0;
    SetScrollInfo(viewport->hwnd, SB_HORZ_VALUE, &si, TRUE_VALUE);
    ShowScrollBar(viewport->hwnd, SB_HORZ_VALUE, show_h ? TRUE_VALUE : FALSE_VALUE);

    si.nMax = viewport->scroll_content_height > 0 ? viewport->scroll_content_height - 1 : 0;
    si.nPage = page_height > 0 ? (UINT)page_height : 1u;
    si.nPos = viewport->scroll_y;
    SetScrollInfo(viewport->hwnd, SB_VERT_VALUE, &si, TRUE_VALUE);
    ShowScrollBar(viewport->hwnd, SB_VERT_VALUE, show_v ? TRUE_VALUE : FALSE_VALUE);

    if (!MoveWindow(content->hwnd,
                    -viewport->scroll_x, -viewport->scroll_y,
                    viewport->scroll_content_width > 0 ? viewport->scroll_content_width : 1,
                    viewport->scroll_content_height > 0 ? viewport->scroll_content_height : 1,
                    TRUE_VALUE))
        return SPW_STATUS_WIN32_ERROR;
    return SPW_STATUS_OK;
}

static int32_t spw_scroll_view_scroll_message(SpwObject *viewport, UINT message, WPARAM wParam) {
    RECT client;
    SCROLLINFO si;
    int32_t *position;
    int32_t content_extent, page, maximum, value;
    uint32_t code;
    if (viewport == NULL || viewport->kind != SPW_CONTROL_SCROLL_VIEWPORT)
        return 0;
    if (!GetClientRect(viewport->hwnd, &client))
        return 0;
    if (message == WM_HSCROLL_VALUE) {
        if (!viewport->scroll_h_enabled) return 1;
        position = &viewport->scroll_x;
        content_extent = viewport->scroll_content_width;
        page = client.right - client.left;
    } else {
        if (!viewport->scroll_v_enabled) return 1;
        position = &viewport->scroll_y;
        content_extent = viewport->scroll_content_height;
        page = client.bottom - client.top;
    }
    maximum = spw_scroll_max(content_extent, page);
    value = *position;
    code = ((uint32_t)(uintptr_t)wParam) & 0xffffu;
    if (code == SB_LINEUP_VALUE) value -= 24;
    else if (code == SB_LINEDOWN_VALUE) value += 24;
    else if (code == SB_PAGEUP_VALUE) value -= page;
    else if (code == SB_PAGEDOWN_VALUE) value += page;
    else if (code == SB_TOP_VALUE) value = 0;
    else if (code == SB_BOTTOM_VALUE) value = maximum;
    else if (code == SB_THUMBPOSITION_VALUE || code == SB_THUMBTRACK_VALUE) {
        si.cbSize = (UINT)sizeof(SCROLLINFO);
        si.fMask = SIF_ALL_VALUE;
        if (GetScrollInfo(viewport->hwnd,
                          message == WM_HSCROLL_VALUE ? SB_HORZ_VALUE : SB_VERT_VALUE,
                          &si))
            value = si.nTrackPos;
    }
    value = spw_scroll_clamp(value, maximum);
    if (*position != value) {
        *position = value;
        spw_scroll_view_apply(viewport);
        spw_push_event(SPW_EVENT_SCROLL_POSITION_CHANGED, viewport->id,
            (int64_t)viewport->scroll_x, (int64_t)viewport->scroll_y);
    } else {
        spw_scroll_view_apply(viewport);
    }
    return 1;
}


static int32_t spw_paned_main_extent(SpwObject *paned) {
    RECT client;
    if (paned == NULL || paned->kind != SPW_CONTROL_PANED || paned->hwnd == NULL)
        return 0;
    if (!GetClientRect(paned->hwnd, &client))
        return 0;
    return paned->paned_vertical ? (client.bottom - client.top) : (client.right - client.left);
}

static int32_t spw_paned_clamp_position(SpwObject *paned, int32_t position, int32_t main_extent) {
    int32_t available, first_min, second_min, lower, upper;
    if (main_extent <= 0) return 0;
    if (!(paned->paned_first_present && paned->paned_second_present))
        return 0;
    available = main_extent - SPW_PANED_SPLITTER_WIDTH;
    if (available < 0) available = 0;
    first_min = paned->paned_first_min;
    second_min = paned->paned_second_min;
    if (first_min < 0) first_min = 0;
    if (second_min < 0) second_min = 0;
    if (first_min > available) first_min = available;
    if (second_min > available) second_min = available;
    lower = first_min;
    upper = available - second_min;
    if (upper < lower) {
        lower = available / 2;
        upper = lower;
    }
    if (position < lower) position = lower;
    if (position > upper) position = upper;
    return position;
}

static int32_t spw_paned_apply(SpwObject *paned, int push_event) {
    RECT client;
    SpwObject *first, *second;
    int32_t width, height, main_extent, available, old_available, position;
    if (paned == NULL || paned->kind != SPW_CONTROL_PANED)
        return SPW_STATUS_BAD_ARGUMENT;
    first = spw_find_object_by_id(paned->paned_first_content_id);
    second = spw_find_object_by_id(paned->paned_second_content_id);
    if (first == NULL || second == NULL)
        return SPW_STATUS_NOT_FOUND;
    if (!GetClientRect(paned->hwnd, &client))
        return SPW_STATUS_WIN32_ERROR;
    width = client.right - client.left;
    height = client.bottom - client.top;
    main_extent = paned->paned_vertical ? height : width;

    if (paned->paned_last_main_extent > 0 && main_extent != paned->paned_last_main_extent &&
        paned->paned_first_present && paned->paned_second_present) {
        available = main_extent - SPW_PANED_SPLITTER_WIDTH;
        old_available = paned->paned_last_main_extent - SPW_PANED_SPLITTER_WIDTH;
        if (available < 0) available = 0;
        if (old_available < 1) old_available = 1;
        if (paned->paned_first_resize && !paned->paned_second_resize)
            paned->paned_position += available - old_available;
        else if (paned->paned_first_resize && paned->paned_second_resize)
            paned->paned_position = (int32_t)(((int64_t)paned->paned_position * (int64_t)available) / (int64_t)old_available);
    }
    paned->paned_last_main_extent = main_extent;

    if (paned->paned_first_present && !paned->paned_second_present) {
        ShowWindow(first->hwnd, SW_SHOW_VALUE);
        ShowWindow(second->hwnd, SW_HIDE_VALUE);
        MoveWindow(first->hwnd, 0, 0, width > 0 ? width : 1, height > 0 ? height : 1, TRUE_VALUE);
        paned->paned_position = 0;
        return SPW_STATUS_OK;
    }
    if (!paned->paned_first_present && paned->paned_second_present) {
        ShowWindow(first->hwnd, SW_HIDE_VALUE);
        ShowWindow(second->hwnd, SW_SHOW_VALUE);
        MoveWindow(second->hwnd, 0, 0, width > 0 ? width : 1, height > 0 ? height : 1, TRUE_VALUE);
        paned->paned_position = 0;
        return SPW_STATUS_OK;
    }
    if (!paned->paned_first_present && !paned->paned_second_present) {
        ShowWindow(first->hwnd, SW_HIDE_VALUE);
        ShowWindow(second->hwnd, SW_HIDE_VALUE);
        paned->paned_position = 0;
        return SPW_STATUS_OK;
    }

    position = spw_paned_clamp_position(paned, paned->paned_position, main_extent);
    paned->paned_position = position;
    ShowWindow(first->hwnd, SW_SHOW_VALUE);
    ShowWindow(second->hwnd, SW_SHOW_VALUE);
    if (paned->paned_vertical) {
        int32_t second_y = position + SPW_PANED_SPLITTER_WIDTH;
        int32_t second_height = height - second_y;
        if (second_height < 1) second_height = 1;
        MoveWindow(first->hwnd, 0, 0, width > 0 ? width : 1, position > 0 ? position : 1, TRUE_VALUE);
        MoveWindow(second->hwnd, 0, second_y, width > 0 ? width : 1, second_height, TRUE_VALUE);
    } else {
        int32_t second_x = position + SPW_PANED_SPLITTER_WIDTH;
        int32_t second_width = width - second_x;
        if (second_width < 1) second_width = 1;
        MoveWindow(first->hwnd, 0, 0, position > 0 ? position : 1, height > 0 ? height : 1, TRUE_VALUE);
        MoveWindow(second->hwnd, second_x, 0, second_width, height > 0 ? height : 1, TRUE_VALUE);
    }
    if (push_event)
        spw_push_event(SPW_EVENT_PANED_POSITION_CHANGED, paned->id, paned->paned_position, main_extent);
    return SPW_STATUS_OK;
}

static int spw_paned_hit_splitter(SpwObject *paned, LPARAM lParam) {
    int32_t coordinate;
    if (paned == NULL || !(paned->paned_first_present && paned->paned_second_present))
        return 0;
    coordinate = paned->paned_vertical ? spw_signed_high_word(lParam) : spw_signed_low_word(lParam);
    return coordinate >= paned->paned_position &&
           coordinate < paned->paned_position + SPW_PANED_SPLITTER_WIDTH;
}

static LRESULT WINAPI spw_window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    SpwObject *object;

    if (hwnd == g_dispatcher && message == SPW_WM_COMMAND) {
        spw_process_command((SpwCommand *)(uintptr_t)lParam);
        return 0;
    }

    object = spw_find_object_by_hwnd(hwnd);
    if (object != NULL) {
        if (object->kind == SPW_CONTROL_PANED) {
            if (message == WM_LBUTTONDOWN_VALUE && spw_paned_hit_splitter(object, lParam)) {
                int32_t coordinate = object->paned_vertical ? spw_signed_high_word(lParam) : spw_signed_low_word(lParam);
                object->paned_dragging = 1u;
                object->paned_drag_offset = coordinate - object->paned_position;
                SetCapture(hwnd);
                return 0;
            }
            if (message == WM_MOUSEMOVE_VALUE && object->paned_dragging) {
                int32_t coordinate = object->paned_vertical ? spw_signed_high_word(lParam) : spw_signed_low_word(lParam);
                object->paned_position = coordinate - object->paned_drag_offset;
                spw_paned_apply(object, 1);
                return 0;
            }
            if (message == WM_LBUTTONUP_VALUE && object->paned_dragging) {
                int32_t coordinate = object->paned_vertical ? spw_signed_high_word(lParam) : spw_signed_low_word(lParam);
                object->paned_position = coordinate - object->paned_drag_offset;
                object->paned_dragging = 0u;
                ReleaseCapture();
                spw_paned_apply(object, 1);
                return 0;
            }
        }
        switch (message) {
            case WM_NOTIFY_VALUE: {
                NMHDR *header = (NMHDR *)(uintptr_t)lParam;
                SpwObject *child;
                if (header == NULL)
                    break;
                child = spw_find_object_by_hwnd(header->hwndFrom);
                if (child != NULL && spw_control_is_list_view(child->kind) && child->suppress_notifications == 0u) {
                    if ((int32_t)header->code == LVN_ITEMCHANGED_VALUE) {
                        NMLISTVIEW *change = (NMLISTVIEW *)(uintptr_t)lParam;
                        if ((change->uChanged & LVIF_STATE_VALUE) != 0u &&
                            ((change->uOldState ^ change->uNewState) & LVIS_SELECTED_VALUE) != 0u) {
                            int64_t selected = (change->uNewState & LVIS_SELECTED_VALUE) != 0u ? 1 : 0;
                            spw_push_event(SPW_EVENT_LIST_SELECTION_CHANGED, child->id, (int64_t)change->iItem + 1, selected);
                        }
                        return 0;
                    }
                    if ((int32_t)header->code == NM_DBLCLK_VALUE) {
                        NMITEMACTIVATE *activation = (NMITEMACTIVATE *)(uintptr_t)lParam;
                        if (activation->iItem >= 0)
                            spw_push_event(SPW_EVENT_ACTIVATED, child->id, (int64_t)activation->iItem + 1, 1);
                        return 0;
                    }
                    if (child->kind == SPW_CONTROL_TABLE && (int32_t)header->code == LVN_COLUMNCLICK_VALUE) {
                        NMLISTVIEW *click = (NMLISTVIEW *)(uintptr_t)lParam;
                        spw_push_event(SPW_EVENT_COLUMN_CLICKED, child->id, (int64_t)click->iSubItem + 1, 0);
                        return 0;
                    }
                }
                if (child != NULL && child->kind == SPW_CONTROL_TREE && child->suppress_notifications == 0u) {
                    if ((int32_t)header->code == TVN_SELCHANGEDW_VALUE) {
                        int32_t token = spw_tree_selected_token(child);
                        if (token > 0)
                            spw_push_event(SPW_EVENT_TREE_SELECTION_CHANGED, child->id, token, 0);
                        return 0;
                    }
                    if ((int32_t)header->code == TVN_ITEMEXPANDEDW_VALUE) {
                        NMTREEVIEWW *tv = (NMTREEVIEWW *)(uintptr_t)lParam;
                        int32_t token = spw_tree_token_for_item(child, tv->itemNew.hItem);
                        int64_t expanded = (tv->itemNew.state & TVIS_EXPANDED_VALUE) ? 1 : 0;
                        if (token > 0)
                            spw_push_event(SPW_EVENT_TREE_EXPANSION_CHANGED, child->id, token, expanded);
                        return 0;
                    }
                    if ((int32_t)header->code == NM_DBLCLK_VALUE) { int32_t token=spw_tree_selected_token(child); if(token>0) spw_push_event(SPW_EVENT_ACTIVATED,child->id,token,1); return 0; }
                }
                if (child != NULL && child->kind == SPW_CONTROL_LINK &&
                    ((int32_t)header->code == NM_CLICK_VALUE ||
                     (int32_t)header->code == NM_RETURN_VALUE)) {
                    spw_push_event(SPW_EVENT_CLICKED, child->id, 0, 0);
                    return 0;
                }
                if (child != NULL && child->kind == SPW_CONTROL_NOTEBOOK &&
                    (int32_t)header->code == TCN_SELCHANGE_VALUE &&
                    child->suppress_notifications == 0u) {
                    LRESULT selected = SendMessageW(child->hwnd, TCM_GETCURSEL_VALUE, 0u, 0);
                    int64_t normalized_index = selected < 0 ? 0 : (int64_t)selected + 1;
                    spw_push_event(SPW_EVENT_SELECTION_CHANGED, child->id, normalized_index, 0);
                    return 0;
                }
                break;
            }

            case WM_COMMAND_VALUE: {
                HWND child_hwnd = (HWND)(uintptr_t)lParam;
                uint32_t notification = ((uint32_t)(uintptr_t)wParam >> 16) & 0xffffu;
                SpwObject *child = spw_find_object_by_hwnd(child_hwnd);
                if (child == NULL) {
                    uint32_t command_id = ((uint32_t)(uintptr_t)wParam) & 0xffffu;
                    uint32_t slot = spw_object_slot(object);
                    if (object->type == SPW_OBJECT_WINDOW && child_hwnd == NULL &&
                        command_id > 0u && command_id <= g_window_menu_item_counts[slot]) {
                        spw_push_event(SPW_EVENT_MENU_COMMAND, object->id, (int64_t)command_id, 0);
                        return 0;
                    }
                    break;
                }
                if (child->kind == SPW_CONTROL_BUTTON && notification == BN_CLICKED_VALUE) {
                    spw_push_event(SPW_EVENT_CLICKED, child->id, 0, 0);
                    return 0;
                }
                if (child->kind == SPW_CONTROL_CHECKBOX && notification == BN_CLICKED_VALUE) {
                    int32_t checked =
                        SendMessageW(child->hwnd, BM_GETCHECK_VALUE, 0, 0) == BST_CHECKED_VALUE ? 1 : 0;
                    spw_push_event(SPW_EVENT_TOGGLED, child->id, checked, 0);
                    return 0;
                }
                if ((child->kind == SPW_CONTROL_TOGGLE_BUTTON || child->kind == SPW_CONTROL_SWITCH) && notification == BN_CLICKED_VALUE) {
                    int32_t checked =
                        SendMessageW(child->hwnd, BM_GETCHECK_VALUE, 0, 0) == BST_CHECKED_VALUE ? 1 : 0;
                    if (child->kind == SPW_CONTROL_SWITCH)
                        InvalidateRect(child->hwnd, NULL, TRUE_VALUE);
                    spw_push_event(SPW_EVENT_TOGGLED, child->id, checked, 0);
                    return 0;
                }
                if (child->kind == SPW_CONTROL_RADIOBUTTON && notification == BN_CLICKED_VALUE) {
                    SendMessageW(child->hwnd, BM_SETCHECK_VALUE, BST_CHECKED_VALUE, 0);
                    spw_push_event(SPW_EVENT_TOGGLED, child->id, 1, 0);
                    return 0;
                }
                if (spw_control_is_text_input(child->kind) &&
                    notification == EN_CHANGE_VALUE &&
                    child->suppress_notifications == 0u) {
                    if (child->kind == SPW_CONTROL_SEARCH_INPUT)
                        InvalidateRect(child->hwnd, NULL, TRUE_VALUE);
                    if (child->kind == SPW_CONTROL_TEXT_AREA && !child->text_undo_enabled)
                        SendMessageW(child->hwnd, EM_EMPTYUNDOBUFFER_VALUE, 0, 0);
                    spw_push_event(SPW_EVENT_TEXT_CHANGED, child->id, 0, 0);
                    return 0;
                }
                if (child->kind == SPW_CONTROL_CODE &&
                    notification == EN_VSCROLL_VALUE &&
                    child->suppress_notifications == 0u) {
                    spw_push_event(SPW_EVENT_SCROLL_POSITION_CHANGED, child->id, 0, 0);
                    return 0;
                }
                if (child->kind == SPW_CONTROL_DROP_LIST &&
                    notification == CBN_SELCHANGE_VALUE &&
                    child->suppress_notifications == 0u) {
                    LRESULT selected = SendMessageW(child->hwnd, CB_GETCURSEL_VALUE, 0u, 0);
                    int64_t normalized_index = selected == CB_ERR_VALUE ? 0 : (int64_t)selected + 1;
                    spw_push_event(SPW_EVENT_SELECTION_CHANGED, child->id, normalized_index, 0);
                    return 0;
                }
                break;
            }

            case WM_HSCROLL_VALUE:
            case WM_VSCROLL_VALUE: {
                HWND child_hwnd;
                SpwObject *child;
                if ((HWND)(uintptr_t)lParam == NULL && object->kind == SPW_CONTROL_SCROLL_VIEWPORT) {
                    if (spw_scroll_view_scroll_message(object, message, wParam)) return 0;
                }
                child_hwnd = (HWND)(uintptr_t)lParam;
                child = spw_find_object_by_hwnd(child_hwnd);
                if (child != NULL && spw_control_is_slider(child->kind) && child->suppress_notifications == 0u) {
                    int32_t value = (int32_t)SendMessageW(child->hwnd, TBM_GETPOS_VALUE, 0, 0);
                    child->normalized_value = spw_clamp_normalized_value(value);
                    spw_push_event(SPW_EVENT_VALUE_CHANGED, child->id, child->normalized_value, 0);
                    return 0;
                }
                break;
            }

            case WM_ACTIVATE_VALUE:
                if (object->popup_window && object->popup_autohide &&
                    (((uint32_t)(uintptr_t)wParam) & 0xffffu) == WA_INACTIVE_VALUE) {
                    spw_push_event(SPW_EVENT_POPUP_DISMISS_REQUESTED, object->id, 2, 0);
                    return 0;
                }
                break;

            case WM_SETCURSOR_VALUE:
                if (object->type == SPW_OBJECT_WINDOW && object->wait_cursor) {
                    SetCursor(LoadCursorW(NULL, IDC_WAIT_VALUE));
                    return 1;
                }
                break;

            case WM_CLOSE_VALUE:
                /* Spec owns close policy. Do not call DestroyWindow here. */
                spw_push_event(SPW_EVENT_CLOSE_REQUESTED, object->id, 0, 0);
                return 0;

            case WM_SIZE_VALUE: {
                uint32_t packed = (uint32_t)(uintptr_t)lParam;
                int64_t width = (int64_t)(packed & 0xffffu);
                int64_t height = (int64_t)((packed >> 16) & 0xffffu);
                if (object->kind == SPW_CONTROL_SCROLL_VIEWPORT && object->scroll_content_id != 0u)
                    spw_scroll_view_apply(object);
                if (object->kind == SPW_CONTROL_PANED && object->paned_initialized)
                    spw_paned_apply(object, 0);
                if (object->type == SPW_OBJECT_WINDOW)
                    spw_push_event(SPW_EVENT_RESIZED, object->id, width, height);
                break;
            }

            case WM_NCDESTROY_VALUE: {
                uint64_t dead_id = object->id;
                if (object->kind == SPW_CONTROL_PANED && object->paned_dragging) {
                    object->paned_dragging = 0u;
                    ReleaseCapture();
                }
                spw_unregister_children_of(dead_id);
                spw_unregister_object(object);
                spw_push_event(SPW_EVENT_DESTROYED, dead_id, 0, 0);
                break;
            }

            default:
                break;
        }
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

static int32_t spw_register_runtime_class(const WCHAR *class_name, HBRUSH background) {
    WNDCLASSEXW wc;
    ATOM atom;
    DWORD error;

    wc.cbSize = (UINT)sizeof(WNDCLASSEXW);
    wc.style = 0u;
    wc.lpfnWndProc = spw_window_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_instance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW_VALUE);
    wc.hbrBackground = background;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = class_name;
    wc.hIconSm = NULL;

    atom = RegisterClassExW(&wc);
    if (atom != 0u)
        return SPW_STATUS_OK;

    error = GetLastError();
    if (error == ERROR_CLASS_ALREADY_EXISTS)
        return SPW_STATUS_OK;

    g_start_error = error;
    return SPW_STATUS_WIN32_ERROR;
}

static int32_t spw_register_window_class(void) {
    int32_t status;
    status = spw_register_runtime_class(g_window_class_name, COLOR_WINDOW_BRUSH);
    if (status != SPW_STATUS_OK) return status;
    return spw_register_runtime_class(g_paned_class_name, COLOR_BTNFACE_BRUSH);
}

static DWORD WINAPI spw_ui_thread_main(void *ignored) {
    MSG message;
    int32_t class_status;
    (void)ignored;

    g_instance = (HINSTANCE)GetModuleHandleW(NULL);
    if (g_instance == NULL) {
        g_start_error = GetLastError();
        g_start_status = SPW_STATUS_WIN32_ERROR;
        SetEvent(g_ready_event);
        return 1u;
    }

    {
        INITCOMMONCONTROLSEX controls;
        controls.dwSize = (DWORD)sizeof(INITCOMMONCONTROLSEX);
        controls.dwICC = ICC_LISTVIEW_CLASSES_VALUE | ICC_TREEVIEW_CLASSES_VALUE | ICC_BAR_CLASSES_VALUE | ICC_PROGRESS_CLASS_VALUE | ICC_TAB_CLASSES_VALUE | ICC_LINK_CLASS_VALUE;
        if (!InitCommonControlsEx(&controls)) {
            g_start_error = GetLastError();
            g_start_status = SPW_STATUS_WIN32_ERROR;
            SetEvent(g_ready_event);
            return 2u;
        }
    }

    class_status = spw_register_window_class();
    if (class_status != SPW_STATUS_OK) {
        g_start_status = class_status;
        SetEvent(g_ready_event);
        return 2u;
    }

    g_dispatcher = CreateWindowExW(
        0u,
        g_window_class_name,
        g_window_class_name,
        0u,
        0, 0, 0, 0,
        HWND_MESSAGE_VALUE,
        NULL,
        g_instance,
        NULL);

    if (g_dispatcher == NULL) {
        g_start_error = GetLastError();
        g_start_status = SPW_STATUS_WIN32_ERROR;
        SetEvent(g_ready_event);
        return 3u;
    }

    g_start_status = SPW_STATUS_OK;
    atomic_store_explicit(&g_state, 2u, memory_order_release);
    SetEvent(g_ready_event);

    while (GetMessageW(&message, NULL, 0u, 0u) > 0) {
        HWND dialog_root = NULL;
        SpwObject *message_object = spw_find_object_by_hwnd(message.hwnd);
        if (message_object != NULL) {
            if (message_object->type == SPW_OBJECT_WINDOW) {
                dialog_root = message_object->hwnd;
            } else if (message_object->type == SPW_OBJECT_CONTROL) {
                SpwObject *parent = spw_find_object_by_id(message_object->parent_id);
                if (parent != NULL && parent->type == SPW_OBJECT_WINDOW)
                    dialog_root = parent->hwnd;
            }
        }
        if (dialog_root != NULL && IsDialogMessageW(dialog_root, &message))
            continue;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    if (g_dispatcher != NULL) {
        HWND dispatcher = g_dispatcher;
        g_dispatcher = NULL;
        DestroyWindow(dispatcher);
    }

    atomic_store_explicit(&g_state, 0u, memory_order_release);
    return 0u;
}

static int32_t spw_dispatch_command(SpwCommand *command) {
    DWORD wait_result;

    if (atomic_load_explicit(&g_state, memory_order_acquire) != 2u || g_dispatcher == NULL)
        return SPW_STATUS_NOT_RUNNING;
    if (command == NULL)
        return SPW_STATUS_BAD_ARGUMENT;

    command->done = CreateEventW(NULL, FALSE_VALUE, FALSE_VALUE, NULL);
    if (command->done == NULL)
        return SPW_STATUS_WIN32_ERROR;

    command->status = SPW_STATUS_START_FAILED;
    command->win32_error = 0u;
    command->result32 = 0;

    if (!PostMessageW(g_dispatcher, SPW_WM_COMMAND, 0u, (LPARAM)(uintptr_t)command)) {
        command->win32_error = GetLastError();
        CloseHandle(command->done);
        command->done = NULL;
        return SPW_STATUS_WIN32_ERROR;
    }

    wait_result = WaitForSingleObject(command->done, INFINITE_VALUE);
    CloseHandle(command->done);
    command->done = NULL;

    if (wait_result != WAIT_OBJECT_0_VALUE)
        return SPW_STATUS_WIN32_ERROR;
    return command->status;
}

