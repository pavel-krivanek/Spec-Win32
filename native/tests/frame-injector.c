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
typedef intptr_t LONG_PTR;
typedef int32_t BOOL;
typedef void *HANDLE;

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP LONG_PTR WINAPI GetWindowLongPtrA(HWND, int32_t);
IMP BOOL WINAPI IsWindowVisible(HWND);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define GWL_STYLE (-16)
#define BS_TYPEMASK 0x0000000fu
#define BS_GROUPBOX 0x00000007u
#define BM_GETCHECK 0x00f0u
#define BM_CLICK 0x00f5u
#define BST_CHECKED 1
#define STD_OUTPUT_HANDLE ((DWORD)-11)

static DWORD text_length(const char *s) {
    DWORD n = 0;
    while (s[n]) ++n;
    return n;
}

static void write_text(const char *s) {
    DWORD written = 0;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), s, text_length(s), &written, (void *)0);
}

static HWND wait_child(HWND parent, const char *class_name, const char *title, DWORD attempts) {
    DWORD i;
    HWND result = (HWND)0;
    for (i = 0; i < attempts && !result; ++i) {
        result = FindWindowExA(parent, (HWND)0, class_name, title);
        if (!result) Sleep(25u);
    }
    return result;
}

static int wait_child_gone(HWND parent, const char *class_name, const char *title, DWORD attempts) {
    DWORD i;
    for (i = 0; i < attempts; ++i) {
        if (!FindWindowExA(parent, (HWND)0, class_name, title)) return 1;
        Sleep(25u);
    }
    return 0;
}

static int wait_visible(HWND hwnd, DWORD attempts) {
    DWORD i;
    for (i = 0; i < attempts; ++i) {
        if (IsWindowVisible(hwnd)) return 1;
        Sleep(25u);
    }
    return 0;
}

int mainCRTStartup(void) {
    HWND top = (HWND)0;
    HWND frame;
    HWND label;
    HWND edit;
    HWND check;
    HWND replace_button;
    HWND replacement;
    HWND remove_button;
    LONG_PTR style;
    DWORD i;

    for (i = 0; i < 400u && !top; ++i) {
        top = FindWindowA((const char *)0, "Spec Win32 v039 Frame");
        if (!top) Sleep(25u);
    }
    if (!top) return 2;

    frame = wait_child(top, "Button", "FRAME GROUP", 200u);
    if (!frame) return 3;
    style = GetWindowLongPtrA(frame, GWL_STYLE);
    if (((uint32_t)style & BS_TYPEMASK) != BS_GROUPBOX) return 4;
    if (!wait_visible(frame, 200u)) return 5;

    label = wait_child(frame, "Static", "FRAME CHILD LABEL", 100u);
    edit = wait_child(frame, "Edit", (const char *)0, 100u);
    check = wait_child(frame, "Button", "FRAME CHILD CHECK", 100u);
    replace_button = wait_child(frame, "Button", "REPLACE FRAME CHILD", 100u);
    if (!label) return 6;
    if (!edit) return 7;
    if (!check) return 8;
    if (!replace_button) return 9;

    SendMessageW(check, BM_CLICK, 0, 0);
    if (SendMessageW(check, BM_GETCHECK, 0, 0) != BST_CHECKED) return 15;
    write_text("FRAME-INJECTOR-NATIVE-PASS\r\n");

    SendMessageW(replace_button, BM_CLICK, 0, 0);
    replacement = wait_child(frame, "Static", "FRAME REPLACEMENT", 320u);
    if (!replacement) return 10;
    if (FindWindowExA(frame, (HWND)0, "Static", "FRAME CHILD LABEL")) return 11;
    remove_button = wait_child(frame, "Button", "REMOVE FRAME CHILD", 100u);
    if (!remove_button) return 12;
    write_text("FRAME-INJECTOR-REPLACE-PASS\r\n");
    /* Give the Smalltalk-side smoke an observation window before removing
     * the just-created replacement subtree. Without this pause the injector
     * can complete both native transitions before the asynchronous event pump
     * observes the intermediate state. */
    Sleep(500u);

    SendMessageW(remove_button, BM_CLICK, 0, 0);
    if (!wait_child_gone(frame, "Static", "FRAME REPLACEMENT", 320u)) return 13;
    if (!IsWindowVisible(frame)) return 14;
    write_text("FRAME-INJECTOR-DYNAMIC-PASS\r\n");
    return 0;
}
