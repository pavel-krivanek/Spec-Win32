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
typedef uint16_t WORD;
typedef uintptr_t ULONG_PTR;
typedef struct { LONG left, top, right, bottom; } RECT;
typedef struct { LONG dx, dy; DWORD mouseData, dwFlags, time; ULONG_PTR dwExtraInfo; } MOUSEINPUT;
typedef struct { WORD wVk, wScan; DWORD dwFlags, time; ULONG_PTR dwExtraInfo; } KEYBDINPUT;
typedef struct { DWORD uMsg; WORD wParamL, wParamH; } HARDWAREINPUT;
typedef struct { DWORD type; union { MOUSEINPUT mi; KEYBDINPUT ki; HARDWAREINPUT hi; }; } INPUT;
IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP BOOL WINAPI GetWindowRect(HWND, RECT *);
IMP BOOL WINAPI SetForegroundWindow(HWND);
IMP BOOL WINAPI SetCursorPos(int32_t, int32_t);
IMP UINT WINAPI SendInput(UINT, INPUT *, int32_t);
IMP void WINAPI Sleep(DWORD);
#define INPUT_MOUSE 0u
#define INPUT_KEYBOARD 1u
#define MOUSEEVENTF_LEFTDOWN 0x0002u
#define MOUSEEVENTF_LEFTUP 0x0004u
#define KEYEVENTF_KEYUP 0x0002u
#define VK_CONTROL 0x11u
static void send_mouse(DWORD flags) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = flags;
    SendInput(1u, &input, (int32_t)sizeof(INPUT));
}
static void send_key(WORD vk, DWORD flags) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = flags;
    SendInput(1u, &input, (int32_t)sizeof(INPUT));
}
int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v020 shortcut smoke");
    HWND edit;
    RECT rect;
    if (!top) return 2;
    edit = FindWindowExA(top, (HWND)0, "Edit", (const char *)0);
    if (!edit) edit = FindWindowExA(top, (HWND)0, "EDIT", (const char *)0);
    if (!edit) edit = FindWindowExA(top, (HWND)0, (const char *)0, (const char *)0);
    if (!edit) return 3;
    if (!GetWindowRect(edit, &rect)) return 4;
    SetForegroundWindow(top);
    SetCursorPos((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
    send_mouse(MOUSEEVENTF_LEFTDOWN);
    send_mouse(MOUSEEVENTF_LEFTUP);
    Sleep(100u);
    send_key((WORD)VK_CONTROL, 0u);
    Sleep(100u);
    send_key((WORD)'K', 0u);
    send_key((WORD)'K', KEYEVENTF_KEYUP);
    Sleep(100u);
    send_key((WORD)VK_CONTROL, KEYEVENTF_KEYUP);
    return 0;
}
