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
int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v056 Athens");
    HWND surface;
    RECT rect;
    const int x = 37;
    const int y = 29;
    if (!top) return 2;
    surface = FindWindowExA(top, (HWND)0, "Static", "");
    if (!surface) return 3;
    if (!GetClientRect(surface, &rect)) return 4;
    if (rect.right - rect.left <= x || rect.bottom - rect.top <= y) return 5;
    if (!PostMessageW(surface, WM_LBUTTONDOWN, 1u, point_param(x, y))) return 6;
    if (!PostMessageW(surface, WM_LBUTTONUP, 0u, point_param(x, y))) return 7;
    return 0;
}
