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
#define BM_GETCHECK_VALUE 0x00f0u
static HWND find_button(HWND top, const char *caption) {
    HWND button = FindWindowExA(top, (HWND)0, "Button", caption);
    if (!button) button = FindWindowExA(top, (HWND)0, "BUTTON", caption);
    return button;
}
int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v030 Toolbar");
    HWND execute;
    HWND pin;
    HWND disabled;
    HWND help;
    HWND late;
    if (!top) return 2;
    execute = find_button(top, "Execute");
    if (!execute) return 3;
    if (IsWindowEnabled(execute)) return 4;
    pin = find_button(top, "Pin");
    if (!pin) return 5;
    if (SendMessageW(pin, BM_GETCHECK_VALUE, 0u, 0) != 0) return 6;
    disabled = find_button(top, "Disabled");
    if (!disabled || IsWindowEnabled(disabled)) return 7;
    help = find_button(top, "Help");
    if (!help) return 8;
    late = find_button(top, "Late");
    if (!late || late == help) return 9;
    if (find_button(top, "Run")) return 10;
    return 0;
}
