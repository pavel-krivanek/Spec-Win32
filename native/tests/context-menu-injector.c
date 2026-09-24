#include <stdint.h>
#if defined(_WIN32)
# define IMP __declspec(dllimport)
# define WINAPI __stdcall
#else
# define IMP
# define WINAPI
#endif

typedef void *HWND;
typedef int32_t BOOL;
typedef int32_t LONG;
typedef uint32_t DWORD;
typedef uint32_t UINT;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
typedef struct { LONG left, top, right, bottom; } RECT;

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP BOOL WINAPI GetWindowRect(HWND, RECT *);
IMP BOOL WINAPI SetForegroundWindow(HWND);
IMP BOOL WINAPI SetCursorPos(int32_t, int32_t);
IMP LRESULT WINAPI SendMessageA(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI mouse_event(DWORD, DWORD, DWORD, DWORD, uintptr_t);
IMP void WINAPI Sleep(DWORD);

#define WM_CONTEXTMENU 0x007Bu
#define MOUSEEVENTF_LEFTDOWN 0x0002u
#define MOUSEEVENTF_LEFTUP 0x0004u

static LPARAM pack_point(int32_t x, int32_t y) {
    return (LPARAM)(((uint32_t)(uint16_t)x) | ((uint32_t)(uint16_t)y << 16));
}

int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v021 context menu smoke");
    HWND child;
    HWND menu;
    RECT rect;
    int32_t x, y;
    if (!top) return 2;
    child = FindWindowExA(top, (HWND)0, "Button", (const char *)0);
    if (!child) child = FindWindowExA(top, (HWND)0, "BUTTON", (const char *)0);
    if (!child) child = FindWindowExA(top, (HWND)0, (const char *)0, (const char *)0);
    if (!child) return 3;
    if (!GetWindowRect(child, &rect)) return 4;
    x = (rect.left + rect.right) / 2;
    y = (rect.top + rect.bottom) / 2;
    SetForegroundWindow(top);
    SendMessageA(child, WM_CONTEXTMENU, (WPARAM)(uintptr_t)child, pack_point(x, y));
    Sleep(800u);
    menu = FindWindowA("#32768", (const char *)0);
    if (!menu) return 5;
    if (!GetWindowRect(menu, &rect)) return 6;
    SetCursorPos(rect.left + 36, rect.top + 12);
    Sleep(200u);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0u, 0u, 0u, 0u);
    mouse_event(MOUSEEVENTF_LEFTUP, 0u, 0u, 0u, 0u);
    Sleep(300u);
    return 0;
}
