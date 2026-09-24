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
typedef struct RECT { int32_t left, top, right, bottom; } RECT;
typedef void *HANDLE;
typedef uint32_t DWORD;

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP HWND WINAPI GetWindow(HWND, UINT);
IMP int32_t WINAPI GetClassNameA(HWND, char *, int32_t);
IMP BOOL WINAPI GetWindowRect(HWND, RECT *);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define WM_MOUSEMOVE 0x0200u
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONUP 0x0202u
#define MK_LBUTTON 0x0001u
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

static void dump_tree(HWND parent, int depth) {
    HWND child = GetWindow(parent, GW_CHILD);
    while (child) {
        char class_name[96];
        int i;
        class_name[0] = 0;
        GetClassNameA(child, class_name, 95);
        for (i = 0; i < depth; ++i) write_text("  ");
        write_text(class_name);
        write_text("\r\n");
        dump_tree(child, depth + 1);
        child = GetWindow(child, GW_HWNDNEXT);
    }
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

static LPARAM point_param(int x, int y) {
    return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y << 16) | (uint16_t)x);
}

int mainCRTStartup(void) {
    HWND top = FindWindowA((const char *)0, "Spec Win32 v037 Paned");
    HWND paned;
    HWND first_content;
    HWND second_content;
    HWND inner_paned;
    HWND inner_first_content;
    RECT before;
    RECT after;
    int before_width;
    int after_width;
    int before_height;
    int after_height;

    if (!top) return 2;
    paned = find_descendant_class(top, "PharoSpecWin32Paned");
    if (!paned) { dump_tree(top, 0); return 3; }
    first_content = FindWindowExA(paned, (HWND)0, "PharoSpecWin32RuntimeWindow", (const char *)0);
    if (!first_content) return 4;
    second_content = FindWindowExA(paned, first_content, "PharoSpecWin32RuntimeWindow", (const char *)0);
    if (!second_content) return 5;
    if (!GetWindowRect(first_content, &before)) return 6;
    before_width = before.right - before.left;
    if (before_width < 240 || before_width > 280) return 7;

    SendMessageW(paned, WM_LBUTTONDOWN, MK_LBUTTON, point_param(263, 100));
    SendMessageW(paned, WM_MOUSEMOVE, MK_LBUTTON, point_param(363, 100));
    SendMessageW(paned, WM_LBUTTONUP, 0, point_param(363, 100));

    if (!GetWindowRect(first_content, &after)) return 8;
    after_width = after.right - after.left;
    if (after_width < before_width + 80) return 9;

    inner_paned = find_descendant_class(second_content, "PharoSpecWin32Paned");
    if (!inner_paned) return 10;
    inner_first_content = FindWindowExA(inner_paned, (HWND)0, "PharoSpecWin32RuntimeWindow", (const char *)0);
    if (!inner_first_content) return 11;
    if (!GetWindowRect(inner_first_content, &before)) return 12;
    before_height = before.bottom - before.top;
    if (before_height < 120 || before_height > 160) return 13;

    SendMessageW(inner_paned, WM_LBUTTONDOWN, MK_LBUTTON, point_param(100, 143));
    SendMessageW(inner_paned, WM_MOUSEMOVE, MK_LBUTTON, point_param(100, 223));
    SendMessageW(inner_paned, WM_LBUTTONUP, 0, point_param(100, 223));

    if (!GetWindowRect(inner_first_content, &after)) return 14;
    after_height = after.bottom - after.top;
    if (after_height < before_height + 60) return 15;
    return 0;
}
