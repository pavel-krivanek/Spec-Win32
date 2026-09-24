#ifndef SPEC_WIN32_RUNTIME_H
#define SPEC_WIN32_RUNTIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPW_ABI_VERSION 0x00290000u

#define SPW_STATUS_OK 0
#define SPW_STATUS_NOT_RUNNING (-1)
#define SPW_STATUS_BAD_ARGUMENT (-2)
#define SPW_STATUS_NOT_FOUND (-3)
#define SPW_STATUS_WIN32_ERROR (-4)
#define SPW_STATUS_CAPACITY (-5)
#define SPW_STATUS_START_FAILED (-6)

#define SPW_EVENT_CLOSE_REQUESTED 1u
#define SPW_EVENT_RESIZED 2u
#define SPW_EVENT_DESTROYED 3u
#define SPW_EVENT_CLICKED 4u
#define SPW_EVENT_BARRIER 5u
#define SPW_EVENT_TOGGLED 6u
#define SPW_EVENT_TEXT_CHANGED 7u
#define SPW_EVENT_SELECTION_CHANGED 8u
#define SPW_EVENT_VALUE_CHANGED 9u
#define SPW_EVENT_FOCUS_RECEIVED 10u
#define SPW_EVENT_FOCUS_LOST 11u
#define SPW_EVENT_LIST_SELECTION_CHANGED 12u
#define SPW_EVENT_ACTIVATED 13u
#define SPW_EVENT_COLUMN_CLICKED 14u
#define SPW_EVENT_TREE_SELECTION_CHANGED 15u
#define SPW_EVENT_TREE_EXPANSION_CHANGED 16u
#define SPW_EVENT_KEY_DOWN 17u
#define SPW_EVENT_KEY_UP 18u
#define SPW_EVENT_MOUSE_DOWN 19u
#define SPW_EVENT_MOUSE_UP 20u
#define SPW_EVENT_MOUSE_MOVE 21u
#define SPW_EVENT_MOUSE_ENTER 22u
#define SPW_EVENT_MOUSE_LEAVE 23u
#define SPW_EVENT_MOUSE_DOUBLE_CLICK 24u
#define SPW_EVENT_CONTEXT_MENU_REQUESTED 25u
#define SPW_EVENT_NUMBER_STEP 26u
#define SPW_EVENT_POPUP_DISMISS_REQUESTED 27u
#define SPW_EVENT_MENU_COMMAND 28u
#define SPW_EVENT_PANED_POSITION_CHANGED 29u
#define SPW_EVENT_SCROLL_POSITION_CHANGED 30u

#define SPW_CONTROL_LABEL 1u
#define SPW_CONTROL_BUTTON 2u
#define SPW_CONTROL_CHECKBOX 3u
#define SPW_CONTROL_RADIOBUTTON 4u
#define SPW_CONTROL_TEXT_INPUT 5u
#define SPW_CONTROL_DROP_LIST 6u
#define SPW_CONTROL_SLIDER_HORIZONTAL 7u
#define SPW_CONTROL_SLIDER_VERTICAL 8u
#define SPW_CONTROL_PROGRESS 9u
#define SPW_CONTROL_NOTEBOOK 10u
#define SPW_CONTROL_LIST 11u
#define SPW_CONTROL_TABLE 12u
#define SPW_CONTROL_TREE 13u
#define SPW_CONTROL_TOGGLE_BUTTON 14u
#define SPW_CONTROL_SWITCH 15u
#define SPW_CONTROL_SPINNER 16u
#define SPW_CONTROL_LINK 17u
#define SPW_CONTROL_NUMBER_INPUT 18u
#define SPW_CONTROL_SEARCH_INPUT 19u
#define SPW_CONTROL_IMAGE 20u
#define SPW_CONTROL_SCROLL_VIEWPORT 21u
#define SPW_CONTROL_SCROLL_CONTENT 22u
#define SPW_CONTROL_PANED 23u
#define SPW_CONTROL_PANED_CONTENT 24u
#define SPW_CONTROL_TAB_CONTENT 25u
#define SPW_CONTROL_FRAME 26u
#define SPW_CONTROL_TEXT_AREA 27u
#define SPW_CONTROL_TREE_COLUMN 28u
#define SPW_CONTROL_COMPONENT_ROW 29u
#define SPW_CONTROL_CODE 30u

#define SPW_SCROLL_POLICY_DISABLED 0u
#define SPW_SCROLL_POLICY_AUTOMATIC 1u
#define SPW_SCROLL_POLICY_HIDDEN 2u
#define SPW_SCROLL_POLICY_ALWAYS 3u

typedef struct SpwEvent {
    uint32_t type;
    uint32_t flags;
    uint64_t object_id;
    int64_t argument1;
    int64_t argument2;
} SpwEvent;

typedef struct SpwBounds {
    uint64_t object_id;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} SpwBounds;

uint32_t spw_abi_version(void);
uint32_t spw_pointer_bits(void);
int32_t spw_runtime_start(void);
int32_t spw_runtime_stop(void);
int32_t spw_runtime_is_running(void);
uint32_t spw_runtime_start_error(void);
uint32_t spw_runtime_object_count(void);
uint32_t spw_runtime_dropped_event_count(void);
int32_t spw_runtime_wake(void);
int32_t spw_wait_for_activity(uint32_t timeout_ms);
int32_t spw_event_pop(SpwEvent *out_event);

int32_t spw_window_create(const char *utf8_title, uint32_t title_length,
                          int32_t x, int32_t y, int32_t width, int32_t height,
                          uint64_t *out_object_id);
int32_t spw_popup_window_create(uint64_t owner_object_id,
                                int32_t x, int32_t y,
                                int32_t width, int32_t height,
                                int32_t autohide,
                                uint64_t *out_object_id);
int32_t spw_window_show(uint64_t object_id, int32_t command_value);
int32_t spw_window_set_title(uint64_t object_id,
                             const char *utf8_title, uint32_t title_length);
int32_t spw_window_set_menu(uint64_t object_id, const void *menu_blob, uint32_t blob_length, uint32_t item_count);
int32_t spw_window_set_bounds(uint64_t object_id,
                              int32_t x, int32_t y, int32_t width, int32_t height);
int32_t spw_window_get_client_size(uint64_t object_id,
                                   int32_t *out_width, int32_t *out_height);
int32_t spw_window_destroy(uint64_t object_id);
int32_t spw_window_is_visible(uint64_t object_id);
int32_t spw_window_activate(uint64_t object_id);

int32_t spw_window_get_bounds(uint64_t object_id,
                              int32_t *out_x, int32_t *out_y,
                              int32_t *out_width, int32_t *out_height);
int32_t spw_window_is_minimized(uint64_t object_id);
int32_t spw_window_is_maximized(uint64_t object_id);
int32_t spw_window_is_foreground(uint64_t object_id);
int32_t spw_window_center(uint64_t object_id);
int32_t spw_window_center_relative(uint64_t object_id, uint64_t relative_object_id);
int32_t spw_window_set_owner(uint64_t object_id, uint64_t owner_object_id);
int32_t spw_window_set_enabled(uint64_t object_id, int32_t enabled);
int32_t spw_window_is_enabled(uint64_t object_id);
int32_t spw_window_set_wait_cursor(uint64_t object_id, int32_t enabled);
int32_t spw_window_wait_cursor_active(uint64_t object_id);
int32_t spw_show_message(uint64_t owner_object_id, int32_t message_kind,
                         const char *utf8_message, uint32_t message_length,
                         const char *utf8_title, uint32_t title_length);
int32_t spw_file_dialog(uint64_t owner_object_id, int32_t mode,
                        const char *utf8_title, uint32_t title_length,
                        const char *utf8_initial_dir, uint32_t initial_dir_length,
                        const char *utf8_initial_name, uint32_t initial_name_length,
                        const void *utf8_filter, uint32_t filter_length,
                        char *out_utf8, uint32_t output_capacity, uint32_t *out_length);

int32_t spw_control_create(uint32_t kind, uint64_t parent_object_id,
                           const char *utf8_text, uint32_t text_length,
                           int32_t x, int32_t y, int32_t width, int32_t height,
                           uint64_t *out_object_id);
int32_t spw_control_set_text(uint64_t object_id,
                             const char *utf8_text, uint32_t text_length);
int32_t spw_control_set_tooltip(uint64_t object_id,
                                const char *utf8_text, uint32_t text_length);
int32_t spw_control_set_context_menu_enabled(uint64_t object_id, int32_t enabled);
int32_t spw_control_show_context_menu(uint64_t object_id,
                                      const void *items_blob, uint32_t blob_length,
                                      const int32_t *flags, uint32_t item_count,
                                      int32_t screen_x, int32_t screen_y);
int32_t spw_control_show_popup_menu(uint64_t object_id, const void *menu_blob, uint32_t blob_length,
                                    uint32_t item_count, int32_t screen_x, int32_t screen_y);
int32_t spw_control_get_screen_bounds(uint64_t object_id, int32_t *out_x, int32_t *out_y,
                                      int32_t *out_width, int32_t *out_height);
int32_t spw_object_get_monitor_work_area(uint64_t object_id, int32_t *out_x, int32_t *out_y,
                                          int32_t *out_width, int32_t *out_height);
int32_t spw_get_cursor_position(int32_t *out_x, int32_t *out_y);
int32_t spw_control_get_text(uint64_t object_id,
                             char *out_utf8, uint32_t capacity, uint32_t *out_length);
int32_t spw_control_set_checked(uint64_t object_id, int32_t checked);
int32_t spw_control_get_checked(uint64_t object_id);
int32_t spw_control_set_editable(uint64_t object_id, int32_t editable);
int32_t spw_control_set_max_length(uint64_t object_id, uint32_t max_length);
int32_t spw_control_set_password(uint64_t object_id, int32_t password);
int32_t spw_control_set_placeholder(uint64_t object_id,
                                    const char *utf8_text, uint32_t text_length);
int32_t spw_control_set_image_bgra(uint64_t object_id,
                                   const void *pixels, uint32_t byte_length,
                                   int32_t width, int32_t height);
int32_t spw_control_set_image_auto_scale(uint64_t object_id, int32_t auto_scale);
int32_t spw_control_set_items(uint64_t object_id,
                              const void *items_blob, uint32_t blob_length, uint32_t item_count);
int32_t spw_list_set_rows(uint64_t object_id,
                          const void *rows_blob, uint32_t blob_length, uint32_t row_count);
int32_t spw_control_set_selected_index(uint64_t object_id, int32_t selected_index);
int32_t spw_control_get_selected_index(uint64_t object_id);
int32_t spw_control_set_value(uint64_t object_id, int32_t normalized_value);
int32_t spw_control_get_value(uint64_t object_id);
int32_t spw_control_set_indeterminate(uint64_t object_id, int32_t indeterminate);
int32_t spw_control_set_enabled(uint64_t object_id, int32_t enabled);
int32_t spw_control_show(uint64_t object_id, int32_t visible);
int32_t spw_control_set_focus(uint64_t object_id);
int32_t spw_control_has_focus(uint64_t object_id);
int32_t spw_control_set_selection(uint64_t object_id, int32_t start_offset, int32_t end_offset);
int32_t spw_control_get_selection(uint64_t object_id, int32_t *out_start_offset, int32_t *out_end_offset);
int32_t spw_text_configure(uint64_t object_id, int32_t wrap_word, int32_t scroll_bars, int32_t undo_enabled);
int32_t spw_text_replace_selection(uint64_t object_id, const char *utf8_text, uint32_t text_length);
int32_t spw_text_command(uint64_t object_id, int32_t command_code);
int32_t spw_text_scroll_to_line(uint64_t object_id, int32_t zero_based_line);
int32_t spw_text_first_visible_line(uint64_t object_id);
int32_t spw_code_set_line_numbers(uint64_t object_id, int32_t enabled);
int32_t spw_code_set_styles(uint64_t object_id, const int32_t *spans, uint32_t span_count,
                            uint32_t foreground_rgb, uint32_t background_rgb);
int32_t spw_code_set_line_decorations(uint64_t object_id, const int32_t *entries,
                                      uint32_t entry_count);
int32_t spw_control_set_selected_indexes(uint64_t object_id, const int32_t *indexes, uint32_t count);
int32_t spw_control_get_selected_indexes(uint64_t object_id, int32_t *out_indexes, uint32_t capacity, uint32_t *out_count);
int32_t spw_control_set_multiple_selection(uint64_t object_id, int32_t multiple);
int32_t spw_control_set_header(uint64_t object_id, const char *utf8_text, uint32_t text_length, int32_t visible);
int32_t spw_control_set_table_columns(uint64_t object_id,
                                      const void *titles_blob, uint32_t blob_length,
                                      const int32_t *widths, const int32_t *alignments,
                                      const int32_t *expandables, uint32_t column_count,
                                      int32_t headers_visible, int32_t resizable);
int32_t spw_control_set_table_cells(uint64_t object_id,
                                    const void *cells_blob, uint32_t blob_length,
                                    uint32_t row_count, uint32_t column_count);
int32_t spw_table_set_cells_with_images(uint64_t object_id,
                                        const void *cells_blob, uint32_t blob_length,
                                        uint32_t row_count, uint32_t column_count);
int32_t spw_control_ensure_visible(uint64_t object_id, int32_t index);
int32_t spw_control_set_tree_nodes(uint64_t object_id,const void *labels_blob,uint32_t blob_length,const int32_t *parent_tokens,uint32_t count);
int32_t spw_tree_set_nodes_with_images(uint64_t object_id,const void *rows_blob,uint32_t blob_length,const int32_t *parent_tokens,uint32_t count);
int32_t spw_control_set_tree_selected_token(uint64_t object_id,int32_t token);
int32_t spw_control_get_tree_selected_token(uint64_t object_id);
int32_t spw_control_tree_set_expanded(uint64_t object_id,int32_t token,int32_t expanded);
int32_t spw_control_tree_is_expanded(uint64_t object_id,int32_t token);
int32_t spw_control_tree_ensure_visible(uint64_t object_id,int32_t token);
int32_t spw_control_set_bounds_batch(const SpwBounds *bounds, uint32_t count);
int32_t spw_control_set_z_order(const uint64_t *object_ids, uint32_t count);
int32_t spw_scroll_view_configure(uint64_t viewport_object_id, uint64_t content_object_id,
                                   int32_t content_width, int32_t content_height,
                                   int32_t horizontal_enabled, int32_t vertical_enabled);
int32_t spw_scroll_view_configure_policies(uint64_t viewport_object_id,
                                            uint64_t content_object_id,
                                            int32_t content_width,
                                            int32_t content_height,
                                            int32_t horizontal_policy,
                                            int32_t vertical_policy);
int32_t spw_scroll_view_get_page_extent(uint64_t viewport_object_id,
                                        int32_t *out_width, int32_t *out_height);
int32_t spw_scroll_view_set_position(uint64_t viewport_object_id, int32_t x, int32_t y);
int32_t spw_paned_configure(uint64_t paned_object_id,
                            uint64_t first_content_object_id,
                            uint64_t second_content_object_id,
                            int32_t vertical,
                            int32_t initial_position,
                            int32_t first_present,
                            int32_t second_present,
                            int32_t first_resize,
                            int32_t second_resize,
                            int32_t first_min,
                            int32_t second_min);
int32_t spw_paned_get_position(uint64_t paned_object_id);
int32_t spw_notebook_get_content_rect(uint64_t notebook_object_id,
                                      int32_t *out_x, int32_t *out_y,
                                      int32_t *out_width, int32_t *out_height);
int32_t spw_control_destroy(uint64_t object_id);
int32_t spw_control_measure(uint32_t kind, uint64_t parent_object_id,
                            const char *utf8_text, uint32_t text_length,
                            int32_t *out_width, int32_t *out_height);
int32_t spw_runtime_barrier(uint64_t token);

int32_t spw_beep(uint32_t kind);

#ifdef __cplusplus
}
#endif

#endif
