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

#define WM_KEYDOWN     0x0100u
#define WM_KEYUP       0x0101u
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONUP   0x0202u
#define MK_LBUTTON     0x0001u

static LPARAM point_param(int x, int y) {
    return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y << 16) | (uint16_t)x);
}

int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v059 Morph");
    HWND after = (HWND)0;
    HWND surface = (HWND)0;
    RECT rect;
    if (!top) return 2;
    for (;;) {
        HWND candidate = FindWindowExA(top, after, "Static", (const char *)0);
        int width, height;
        if (!candidate) break;
        after = candidate;
        if (!GetClientRect(candidate, &rect)) continue;
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
        if (width >= 300 && height >= 180) { surface = candidate; break; }
    }
    if (!surface) return 3;
    if (!PostMessageW(surface, WM_LBUTTONDOWN, MK_LBUTTON, point_param(70, 80))) return 4;
    if (!PostMessageW(surface, WM_LBUTTONUP, 0u, point_param(70, 80))) return 5;
    if (!PostMessageW(surface, WM_KEYDOWN, 0x41u, 0)) return 6;
    if (!PostMessageW(surface, WM_KEYUP, 0x41u, 0)) return 7;
    return 0;
}
