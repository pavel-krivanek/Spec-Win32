#include <stdint.h>
#if defined(_WIN32)
# define IMP __declspec(dllimport)
# define WINAPI __stdcall
#else
# define IMP
# define WINAPI
#endif

typedef void *HWND;
typedef uint32_t DWORD;
typedef int32_t BOOL;
typedef void *HANDLE;
typedef struct { int32_t left; int32_t top; int32_t right; int32_t bottom; } RECT;

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI GetWindow(HWND, uint32_t);
IMP int32_t WINAPI GetClassNameA(HWND, char *, int32_t);
IMP BOOL WINAPI GetWindowRect(HWND, RECT *);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

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

static int same_string(const char *a, const char *b) {
    while (*a && *b && *a == *b) { ++a; ++b; }
    return *a == *b;
}

static void collect_lists(HWND parent, HWND *items, int32_t *count, int32_t capacity) {
    HWND child = GetWindow(parent, GW_CHILD);
    while (child) {
        char class_name[96];
        class_name[0] = 0;
        if (GetClassNameA(child, class_name, 95) > 0 && same_string(class_name, "SysListView32")) {
            if (*count < capacity) items[(*count)++] = child;
        }
        collect_lists(child, items, count, capacity);
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

static int32_t list_count(HWND top, HWND *items, int32_t capacity) {
    int32_t count = 0;
    collect_lists(top, items, &count, capacity);
    return count;
}

static int wait_count(HWND top, int32_t expected, DWORD attempts) {
    DWORD i;
    HWND items[16];
    for (i = 0; i < attempts; ++i) {
        if (list_count(top, items, 16) == expected) return 1;
        Sleep(25u);
    }
    return 0;
}

static void sort_by_left(HWND *items, int32_t count) {
    int32_t i, j;
    for (i = 0; i < count; ++i) {
        for (j = i + 1; j < count; ++j) {
            RECT a, b;
            if (!GetWindowRect(items[i], &a) || !GetWindowRect(items[j], &b)) continue;
            if (b.left < a.left) {
                HWND tmp = items[i];
                items[i] = items[j];
                items[j] = tmp;
            }
        }
    }
}

int mainCRTStartup(void) {
    HWND top = (HWND)0;
    HWND items[16];
    DWORD i;
    int32_t count;
    RECT first, last;

    for (i = 0; i < 400u && !top; ++i) {
        top = FindWindowA((const char *)0, "Spec Win32 v042 Miller");
        if (!top) Sleep(25u);
    }
    if (!top) return 2;
    if (!wait_count(top, 1, 200u)) return 3;
    write_text("MILLER-INJECTOR-ROOT-PASS\r\n");

    if (!wait_count(top, 4, 320u)) return 4;
    write_text("MILLER-INJECTOR-DYNAMIC-PAGES-PASS\r\n");

    count = list_count(top, items, 16);
    if (count != 4) return 20;
    sort_by_left(items, count);
    if (!GetWindowRect(items[0], &first) || !GetWindowRect(items[3], &last)) return 21;
    if ((first.right - first.left) < 100 || (last.right - last.left) < 100) return 22;
    write_text("MILLER-INJECTOR-NATIVE-PAGES-PASS\r\n");

    Sleep(3000u);
    return 0;
}
