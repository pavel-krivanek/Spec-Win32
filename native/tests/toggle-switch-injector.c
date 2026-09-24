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
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONUP 0x0202u
static LPARAM point_param(int x, int y) {
    return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y << 16) | (uint16_t)x);
}
static int click(HWND hwnd) {
    if (!PostMessageW(hwnd, WM_LBUTTONDOWN, 1, point_param(15, 10))) return 0;
    if (!PostMessageW(hwnd, WM_LBUTTONUP, 0, point_param(15, 10))) return 0;
    return 1;
}
int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v025 Toggle Switch");
    HWND mode_b;
    HWND sw;
    if (!top) return 2;
    mode_b = FindWindowExA(top, (HWND)0, "Button", "Mode B");
    if (!mode_b) mode_b = FindWindowExA(top, (HWND)0, "BUTTON", "Mode B");
    if (!mode_b) return 3;
    sw = FindWindowExA(top, mode_b, "Button", (const char *)0);
    if (!sw) sw = FindWindowExA(top, mode_b, "BUTTON", (const char *)0);
    if (!sw) return 4;
    if (!click(mode_b)) return 5;
    if (!click(sw)) return 6;
    return 0;
}
