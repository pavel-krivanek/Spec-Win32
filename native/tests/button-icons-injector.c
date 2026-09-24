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
static void click(HWND hwnd) {
    SendMessageW(hwnd, WM_LBUTTONDOWN, 1u, point_param(18, 14));
    SendMessageW(hwnd, WM_LBUTTONUP, 0u, point_param(18, 14));
}
int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v029 Button Icons");
    HWND button;
    HWND toggle;
    HWND disabled;
    if (!top) return 2;
    button = FindWindowExA(top, (HWND)0, "Button", "Run with icon");
    if (!button) button = FindWindowExA(top, (HWND)0, "BUTTON", "Run with icon");
    if (!button) return 3;
    disabled = FindWindowExA(top, (HWND)0, "Button", "Disabled icon");
    if (!disabled) disabled = FindWindowExA(top, (HWND)0, "BUTTON", "Disabled icon");
    if (!disabled) return 4;
    if (IsWindowEnabled(disabled)) return 5;
    toggle = FindWindowExA(top, (HWND)0, "Button", "Pinned");
    if (!toggle) toggle = FindWindowExA(top, (HWND)0, "BUTTON", "Pinned");
    if (!toggle) return 6;
    click(button);
    click(toggle);
    return 0;
}
