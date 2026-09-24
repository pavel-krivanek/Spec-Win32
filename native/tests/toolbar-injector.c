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
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP BOOL WINAPI IsWindowEnabled(HWND);
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONUP 0x0202u
static LPARAM point_param(int x, int y) {
    return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y << 16) | (uint16_t)x);
}
static HWND find_button(HWND top, const char *caption) {
    HWND button = FindWindowExA(top, (HWND)0, "Button", caption);
    if (!button) button = FindWindowExA(top, (HWND)0, "BUTTON", caption);
    return button;
}
static void click(HWND hwnd) {
    SendMessageW(hwnd, WM_LBUTTONDOWN, 1u, point_param(16, 14));
    SendMessageW(hwnd, WM_LBUTTONUP, 0u, point_param(16, 14));
}
int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v030 Toolbar");
    HWND run;
    HWND pin;
    HWND disabled;
    HWND help;
    if (!top) return 2;
    run = find_button(top, "Run");
    if (!run) return 3;
    pin = find_button(top, "Pin");
    if (!pin) return 4;
    disabled = find_button(top, "Disabled");
    if (!disabled) return 5;
    if (IsWindowEnabled(disabled)) return 6;
    help = find_button(top, "Help");
    if (!help) return 7;
    click(run);
    click(pin);
    return 0;
}
