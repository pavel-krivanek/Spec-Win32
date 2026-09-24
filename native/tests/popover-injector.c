#include <stdint.h>
#if defined(_WIN32)
# define IMP __declspec(dllimport)
# define WINAPI __stdcall
#else
# define IMP
# define WINAPI
#endif
typedef void *HWND;
typedef uint32_t UINT;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
typedef int32_t BOOL;
IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP HWND WINAPI GetWindow(HWND, UINT);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP BOOL WINAPI IsWindowVisible(HWND);
IMP void WINAPI Sleep(uint32_t);
#define GW_OWNER 4u
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONUP 0x0202u
#define WM_KEYDOWN 0x0100u
#define WM_ACTIVATE 0x0006u
#define WA_INACTIVE 0u
#define VK_ESCAPE 0x1bu
static LPARAM point_param(int x, int y) {
    return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y << 16) | (uint16_t)x);
}
static void click(HWND hwnd) {
    SendMessageW(hwnd, WM_LBUTTONDOWN, 1u, point_param(16, 12));
    SendMessageW(hwnd, WM_LBUTTONUP, 0u, point_param(16, 12));
}
static HWND find_owned_visible_popup(HWND owner) {
    HWND after = (HWND)0;
    HWND w;
    for (;;) {
        w = FindWindowExA((HWND)0, after, "PharoSpecWin32RuntimeWindow", (const char *)0);
        if (!w) return (HWND)0;
        if (w != owner && GetWindow(w, GW_OWNER) == owner && IsWindowVisible(w)) return w;
        after = w;
    }
}
static HWND find_child(HWND parent, const char *klass, const char *caption) {
    HWND h = FindWindowExA(parent, (HWND)0, klass, caption);
    if (!h && klass[0] >= 'A' && klass[0] <= 'Z') {
        /* Wine class matching is normally insensitive; explicit aliases are handled at call sites. */
    }
    return h;
}
int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v032 Popover");
    HWND details;
    HWND popup;
    HWND inside;
    HWND edit;
    int i;
    if (!top) return 2;
    details = find_child(top, "Button", "Details");
    if (!details) details = find_child(top, "BUTTON", "Details");
    if (!details) return 3;
    click(details);
    popup = (HWND)0;
    for (i = 0; i < 100 && !popup; ++i) { Sleep(25); popup = find_owned_visible_popup(top); }
    if (!popup) return 4;
    /* Hold the first live popup long enough for a screenshot after Smalltalk replaces its content. */
    Sleep(2200);
    inside = find_child(popup, "Button", "Inside action");
    if (!inside) inside = find_child(popup, "BUTTON", "Inside action");
    if (!inside) return 5;
    click(inside);
    Sleep(500);
    if (IsWindowVisible(popup)) return 6;
    click(details);
    popup = (HWND)0;
    for (i = 0; i < 100 && !popup; ++i) { Sleep(25); popup = find_owned_visible_popup(top); }
    if (!popup) return 7;
    edit = find_child(popup, "Edit", (const char *)0);
    if (!edit) edit = find_child(popup, "EDIT", (const char *)0);
    if (!edit) return 8;
    Sleep(500);
    SendMessageW(edit, WM_KEYDOWN, VK_ESCAPE, 0);
    Sleep(600);
    if (IsWindowVisible(popup)) return 9;
    click(details);
    popup = (HWND)0;
    for (i = 0; i < 100 && !popup; ++i) { Sleep(25); popup = find_owned_visible_popup(top); }
    if (!popup) return 10;
    Sleep(300);
    SendMessageW(popup, WM_ACTIVATE, WA_INACTIVE, 0);
    Sleep(700);
    if (IsWindowVisible(popup)) return 11;
    return 0;
}
