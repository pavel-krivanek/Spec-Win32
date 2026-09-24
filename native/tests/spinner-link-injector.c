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
int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v026 Spinner Link");
    HWND link;
    if (!top) return 2;
    link = FindWindowExA(top, (HWND)0, "SysLink", (const char *)0);
    if (!link) return 3;
    if (!PostMessageW(link, WM_LBUTTONDOWN, 1u, point_param(20, 10))) return 4;
    if (!PostMessageW(link, WM_LBUTTONUP, 0u, point_param(20, 10))) return 5;
    return 0;
}
