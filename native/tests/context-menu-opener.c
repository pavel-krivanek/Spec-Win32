#include <stdint.h>
#if defined(_WIN32)
# define IMP __declspec(dllimport)
# define WINAPI __stdcall
#else
# define IMP
# define WINAPI
#endif
typedef void *HWND; typedef int32_t BOOL; typedef int32_t LONG; typedef uint32_t UINT; typedef uintptr_t WPARAM; typedef intptr_t LPARAM; typedef intptr_t LRESULT; typedef struct { LONG left, top, right, bottom; } RECT;
IMP HWND WINAPI FindWindowA(const char *, const char *); IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *); IMP BOOL WINAPI GetWindowRect(HWND, RECT *); IMP LRESULT WINAPI SendMessageA(HWND, UINT, WPARAM, LPARAM);
#define WM_CONTEXTMENU 0x007Bu
static LPARAM pack_point(int32_t x, int32_t y) { return (LPARAM)(((uint32_t)(uint16_t)x) | ((uint32_t)(uint16_t)y << 16)); }
int mainCRTStartup(void) { HWND top=FindWindowA((const char*)0,"Spec Win32 v021 context menu smoke"), child; RECT r; int32_t x,y; if(!top) return 2; child=FindWindowExA(top,(HWND)0,"Button",(const char*)0); if(!child) child=FindWindowExA(top,(HWND)0,(const char*)0,(const char*)0); if(!child) return 3; if(!GetWindowRect(child,&r)) return 4; x=(r.left+r.right)/2; y=(r.top+r.bottom)/2; SendMessageA(child,WM_CONTEXTMENU,(WPARAM)(uintptr_t)child,pack_point(x,y)); return 0; }
