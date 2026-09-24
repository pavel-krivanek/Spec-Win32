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
IMP BOOL WINAPI PostMessageW(HWND, UINT, WPARAM, LPARAM);
#define WM_CLOSE 0x0010u
int mainCRTStartup(void) {
    HWND modal = FindWindowA((const char *)0, "Spec Win32 v024 close modal");
    if (!modal) return 2;
    return PostMessageW(modal, WM_CLOSE, 0, 0) ? 0 : 3;
}
