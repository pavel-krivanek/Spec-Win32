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
typedef struct { int32_t left, top, right, bottom; } RECT;
IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP BOOL WINAPI GetClientRect(HWND, RECT *);
IMP BOOL WINAPI PostMessageW(HWND, UINT, WPARAM, LPARAM);
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONUP 0x0202u
static LPARAM point_param(int x, int y) {
    return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y << 16) | (uint16_t)x);
}
static int click_affordance(HWND hwnd, int upper_half) {
    RECT rect;
    int x, y;
    if (!GetClientRect(hwnd, &rect)) return 0;
    x = rect.right - 5;
    y = upper_half ? 4 : (rect.bottom - rect.top) / 2;
    if (!PostMessageW(hwnd, WM_LBUTTONDOWN, 1u, point_param(x, y))) return 0;
    if (!PostMessageW(hwnd, WM_LBUTTONUP, 0u, point_param(x, y))) return 0;
    return 1;
}
int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v027 Number Search");
    HWND number;
    HWND search;
    if (!top) return 2;
    number = FindWindowExA(top, (HWND)0, "Edit", (const char *)0);
    if (!number) return 3;
    search = FindWindowExA(top, number, "Edit", (const char *)0);
    if (!search) return 4;
    if (!click_affordance(number, 1)) return 5;
    if (!click_affordance(search, 0)) return 6;
    return 0;
}
