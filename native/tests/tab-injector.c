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

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP HWND WINAPI GetWindow(HWND, UINT);
IMP int32_t WINAPI GetClassNameA(HWND, char *, int32_t);
IMP BOOL WINAPI IsWindowVisible(HWND);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define TCM_FIRST 0x1300u
#define TCM_GETITEMCOUNT (TCM_FIRST + 4u)
#define TCM_GETCURSEL (TCM_FIRST + 11u)
#define GW_HWNDNEXT 2u
#define GW_CHILD 5u
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

static int string_equal(const char *a, const char *b) {
    while (*a && *b && *a == *b) { ++a; ++b; }
    return *a == *b;
}

static HWND find_descendant_class(HWND parent, const char *wanted) {
    HWND child = GetWindow(parent, GW_CHILD);
    while (child) {
        char class_name[96];
        class_name[0] = 0;
        if (GetClassNameA(child, class_name, 95) > 0 && string_equal(class_name, wanted)) return child;
        {
            HWND nested = find_descendant_class(child, wanted);
            if (nested) return nested;
        }
        child = GetWindow(child, GW_HWNDNEXT);
    }
    return (HWND)0;
}


int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v038 Tab");
    HWND tab;
    HWND first_page;
    HWND second_page;
    HWND first_label;
    HWND second_label;
    LRESULT count;

    if (!top) return 2;
    tab = find_descendant_class(top, "SysTabControl32");
    if (!tab) return 3;
    count = SendMessageW(tab, TCM_GETITEMCOUNT, 0, 0);
    if (count != 2) return 4;

    first_page = FindWindowExA(tab, (HWND)0, "PharoSpecWin32RuntimeWindow", (const char *)0);
    if (!first_page) return 5;
    second_page = FindWindowExA(tab, first_page, "PharoSpecWin32RuntimeWindow", (const char *)0);
    if (!second_page) return 6;
    if (!IsWindowVisible(first_page)) return 7;
    if (IsWindowVisible(second_page)) return 8;
    first_label = FindWindowExA(first_page, (HWND)0, "Static", "TAB PAGE ONE");
    if (!first_label) return 9;

    SendMessageW(tab, 0x0100u, 0x27u, 0); /* WM_KEYDOWN / VK_RIGHT */
    SendMessageW(tab, 0x0101u, 0x27u, 0); /* WM_KEYUP / VK_RIGHT */
    Sleep(700);

    if (SendMessageW(tab, TCM_GETCURSEL, 0, 0) != 1) return 16;
    if (IsWindowVisible(first_page)) return 11;
    if (!IsWindowVisible(second_page)) return 12;
    second_label = FindWindowExA(second_page, (HWND)0, "Static", "TAB PAGE TWO");
    if (!second_label || !IsWindowVisible(second_label)) return 13;
    write_text("TAB-INJECTOR-SELECTION-PASS\r\n");

    Sleep(1300);
    count = SendMessageW(tab, TCM_GETITEMCOUNT, 0, 0);
    if (count != 1) return 14;
    if (!IsWindowVisible(second_page)) return 15;
    write_text("TAB-INJECTOR-DYNAMIC-PASS\r\n");
    return 0;
}
