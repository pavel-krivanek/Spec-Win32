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

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);

#define WM_KEYDOWN 0x0100u
#define WM_KEYUP 0x0101u
#define VK_RETURN 0x0Du

int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v023 NativeKey Dialog");
    HWND button;
    if (!top) return 2;
    button = FindWindowExA(top, (HWND)0, "Button", (const char *)0);
    if (!button) button = FindWindowExA(top, (HWND)0, "BUTTON", (const char *)0);
    if (!button) return 3;
    SendMessageW(button, WM_KEYDOWN, (WPARAM)VK_RETURN, (LPARAM)0);
    SendMessageW(button, WM_KEYUP, (WPARAM)VK_RETURN, (LPARAM)0);
    return 0;
}
