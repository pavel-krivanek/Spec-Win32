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
IMP BOOL WINAPI PostMessageW(HWND, UINT, WPARAM, LPARAM);
IMP BOOL WINAPI IsWindowEnabled(HWND);
IMP void WINAPI Sleep(uint32_t);

#define WM_LBUTTONDOWN_VALUE 0x0201u
#define WM_LBUTTONUP_VALUE   0x0202u
#define WM_CLOSE_VALUE       0x0010u
#define BM_CLICK_VALUE       0x00F5u

static LPARAM point_param(int x, int y) {
    return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y << 16) | (uint16_t)x);
}

static HWND wait_for_window(const char *title) {
    uint32_t i;
    for (i = 0; i < 200u; ++i) {
        HWND h = FindWindowA((const char *)0, title);
        if (h) return h;
        Sleep(25u);
    }
    return (HWND)0;
}

static HWND first_button(HWND top) {
    HWND button = FindWindowExA(top, (HWND)0, "Button", (const char *)0);
    if (!button) button = FindWindowExA(top, (HWND)0, "BUTTON", (const char *)0);
    return button;
}

int mainCRTStartup(void) {
    HWND owner = wait_for_window("Spec Win32 v061 owner");
    HWND background = wait_for_window("Spec Win32 v061 background");
    HWND open_button;
    HWND blocked;
    HWND background_button;

    if (!owner) return 2;
    if (!background) return 3;
    open_button = first_button(owner);
    if (!open_button) return 4;

    /* Post, rather than synchronously SendMessage, because the button callback
       itself blocks in BlockedDialogWindow>>open until the dialog resolves. */
    if (!PostMessageW(open_button, WM_LBUTTONDOWN_VALUE, 1u, point_param(18, 14))) return 5;
    if (!PostMessageW(open_button, WM_LBUTTONUP_VALUE, 0u, point_param(18, 14))) return 6;

    blocked = wait_for_window("Spec Win32 v061 blocked");
    if (!blocked) return 7;
    if (!IsWindowEnabled(owner)) return 8;
    if (!IsWindowEnabled(background)) return 9;

    background_button = first_button(background);
    if (!background_button) return 10;
    SendMessageW(background_button, BM_CLICK_VALUE, 0u, 0);
    Sleep(250u);

    if (!PostMessageW(blocked, WM_CLOSE_VALUE, 0u, 0)) return 11;
    return 0;
}
