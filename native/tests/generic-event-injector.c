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
typedef int32_t BOOL;
IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP BOOL WINAPI PostMessageW(HWND, UINT, WPARAM, LPARAM);
#define WM_KEYDOWN 0x0100u
#define WM_KEYUP 0x0101u
#define WM_MOUSEMOVE 0x0200u
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONUP 0x0202u
#define WM_LBUTTONDBLCLK 0x0203u
#define WM_MOUSELEAVE 0x02a3u
static LPARAM point_param(int x, int y) {
    return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y << 16) | (uint16_t)x);
}
int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v018 generic events");
    HWND button;
    if (!top) return 2;
    button = FindWindowExA(top, (HWND)0, "Button", (const char *)0);
    if (!button) button = FindWindowExA(top, (HWND)0, "BUTTON", (const char *)0);
    if (!button) button = FindWindowExA(top, (HWND)0, (const char *)0, (const char *)0);
    if (!button) return 3;
    PostMessageW(button, WM_MOUSEMOVE, 0, point_param(20, 18));
    PostMessageW(button, WM_LBUTTONDOWN, 1, point_param(20, 18));
    PostMessageW(button, WM_LBUTTONUP, 0, point_param(20, 18));
    PostMessageW(button, WM_LBUTTONDBLCLK, 1, point_param(21, 19));
    PostMessageW(button, WM_KEYDOWN, (WPARAM)'A', 0);
    PostMessageW(button, WM_KEYUP, (WPARAM)'A', 0);
    PostMessageW(button, WM_MOUSELEAVE, 0, 0);
    return 0;
}
