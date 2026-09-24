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
typedef uint32_t DWORD;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
typedef int32_t BOOL;
typedef void *HANDLE;
typedef struct { int32_t x; int32_t y; } POINT;
typedef struct { int32_t left; int32_t top; int32_t right; int32_t bottom; } RECT;

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP BOOL WINAPI GetClientRect(HWND, RECT *);
IMP BOOL WINAPI ClientToScreen(HWND, POINT *);
IMP HWND WINAPI WindowFromPoint(POINT);
IMP BOOL WINAPI GetWindowRect(HWND, RECT *);
IMP int WINAPI GetWindowTextA(HWND, char *, int);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define BM_CLICK 0x00f5u
#define STD_OUTPUT_HANDLE ((DWORD)-11)

static DWORD text_length(const char *s) { DWORD n = 0; while (s[n]) ++n; return n; }
static void write_text(const char *s) { DWORD w = 0; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), s, text_length(s), &w, (void *)0); }
static int same_text(HWND hwnd, const char *expected) {
    char text[128]; int n = GetWindowTextA(hwnd, text, 127); DWORD i;
    if (n < 0) return 0; text[n] = 0;
    for (i = 0; expected[i] || text[i]; ++i) if (expected[i] != text[i]) return 0;
    return 1;
}
static HWND wait_child(HWND parent, const char *title, DWORD attempts) {
    DWORD i; HWND h = (HWND)0;
    for (i = 0; i < attempts && !h; ++i) { h = FindWindowExA(parent, (HWND)0, "Button", title); if (!h) Sleep(25u); }
    return h;
}
static int wait_child_gone(HWND parent, const char *title, DWORD attempts) {
    DWORD i; for (i = 0; i < attempts; ++i) { if (!FindWindowExA(parent, (HWND)0, "Button", title)) return 1; Sleep(25u); } return 0;
}
static HWND wait_top_at(POINT point, const char *title, DWORD attempts) {
    DWORD i; HWND h;
    for (i = 0; i < attempts; ++i) { h = WindowFromPoint(point); if (h && same_text(h, title)) return h; Sleep(25u); }
    return (HWND)0;
}

int mainCRTStartup(void) {
    HWND top = (HWND)0, main, overlay2, overlay3, replacement, hit;
    RECT client, mainRect, overlayRect; POINT origin, center; DWORD i;
    for (i = 0; i < 400u && !top; ++i) { top = FindWindowA((const char *)0, "Spec Win32 v040 Overlay"); if (!top) Sleep(25u); }
    if (!top) return 2;
    main = wait_child(top, "MAIN BACKGROUND", 200u);
    overlay2 = wait_child(top, "OVERLAY TWO", 200u);
    if (!main || !overlay2) return 3;
    if (!GetClientRect(top, &client)) return 4;
    origin.x = 0; origin.y = 0; if (!ClientToScreen(top, &origin)) return 5;
    center.x = origin.x + (client.right - client.left) / 2;
    center.y = origin.y + (client.bottom - client.top) / 2;
    if (!GetWindowRect(main, &mainRect) || !GetWindowRect(overlay2, &overlayRect)) return 6;
    if (mainRect.left != origin.x || mainRect.top != origin.y) return 7;
    if ((mainRect.right - mainRect.left) != (client.right - client.left) || (mainRect.bottom - mainRect.top) != (client.bottom - client.top)) return 8;
    if ((overlayRect.right - overlayRect.left) >= (mainRect.right - mainRect.left) || (overlayRect.bottom - overlayRect.top) >= (mainRect.bottom - mainRect.top)) return 9;
    hit = wait_top_at(center, "OVERLAY TWO", 200u); if (!hit) return 10;
    write_text("OVERLAY-INJECTOR-INITIAL-ZPASS\r\n");
    SendMessageW(hit, BM_CLICK, 0, 0);
    replacement = wait_child(top, "REPLACED MAIN", 320u); if (!replacement) return 11;
    if (!wait_child_gone(top, "MAIN BACKGROUND", 100u)) return 12;
    overlay3 = wait_child(top, "OVERLAY THREE", 320u); if (!overlay3) return 13;
    hit = wait_top_at(center, "OVERLAY THREE", 320u); if (!hit) return 14;
    write_text("OVERLAY-INJECTOR-DYNAMIC-ZPASS\r\n");
    SendMessageW(hit, BM_CLICK, 0, 0);
    if (!wait_child_gone(top, "OVERLAY THREE", 320u)) return 15;
    if (!wait_top_at(center, "OVERLAY TWO", 320u)) return 16;
    if (!wait_child(top, "REPLACED MAIN", 20u)) return 17;
    write_text("OVERLAY-INJECTOR-REMOVE-ZPASS\r\n");
    return 0;
}
