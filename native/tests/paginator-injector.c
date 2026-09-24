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

#define WM_MOUSEMOVE   0x0200u
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONUP   0x0202u
#define MK_LBUTTON     0x0001u

static LPARAM point_param(int x, int y) {
    return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y << 16) | (uint16_t)x);
}

static BOOL click(HWND hwnd, int x, int y) {
    return PostMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, point_param(x, y)) &&
           PostMessageW(hwnd, WM_LBUTTONUP, 0u, point_param(x, y));
}

static BOOL drag(HWND hwnd, int x1, int y1, int x2, int y2) {
    return PostMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, point_param(x1, y1)) &&
           PostMessageW(hwnd, WM_MOUSEMOVE, MK_LBUTTON, point_param(x2, y2)) &&
           PostMessageW(hwnd, WM_LBUTTONUP, 0u, point_param(x2, y2));
}

int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v058 Paginator");
    HWND surface;
    RECT rect;
    if (!top) return 2;
    surface = FindWindowExA(top, (HWND)0, "Static", "");
    if (!surface) return 3;
    if (!GetClientRect(surface, &rect)) return 4;
    if ((rect.right - rect.left) < 140 || (rect.bottom - rect.top) < 18) return 5;

    /* Page 5 centre: selection becomes 5-6. */
    if (!click(surface, 63, 10)) return 6;
    /* Drag the 5-6 selection window right by exactly one 14px page. */
    if (!drag(surface, 70, 10, 84, 10)) return 7;
    /* Resize the right edge from two pages to three pages. */
    if (!drag(surface, 98, 10, 112, 10)) return 8;
    return 0;
}
