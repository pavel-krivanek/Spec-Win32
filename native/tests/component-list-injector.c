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
typedef uint64_t WPARAM;
typedef int64_t LPARAM;
typedef int64_t LRESULT;
typedef int32_t BOOL;
typedef void *HANDLE;
typedef struct { int32_t left, top, right, bottom; } RECT;

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI GetWindow(HWND, UINT);
IMP HWND WINAPI GetParent(HWND);
IMP int32_t WINAPI GetClassNameA(HWND, char *, int32_t);
IMP BOOL WINAPI GetWindowRect(HWND, RECT *);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define GW_HWNDNEXT 2u
#define GW_CHILD 5u
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define WM_LBUTTONDOWN_VALUE 0x0201u
#define WM_LBUTTONUP_VALUE 0x0202u
#define WM_LBUTTONDBLCLK_VALUE 0x0203u
#define WM_VSCROLL_VALUE 0x0115u
#define BM_CLICK_VALUE 0x00F5u
#define SB_PAGEDOWN_VALUE 3u
#define MAX_ROWS 32u

static DWORD text_length(const char *s) { DWORD n=0; while(s[n]) ++n; return n; }
static void write_text(const char *s) {
    DWORD written=0; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,text_length(s),&written,(void*)0);
}
static int same_ascii(const char *a,const char *b) {
    while(*a && *b) {
        char ca=*a, cb=*b;
        if(ca>='A' && ca<='Z') ca=(char)(ca-'A'+'a');
        if(cb>='A' && cb<='Z') cb=(char)(cb-'A'+'a');
        if(ca!=cb) return 0;
        ++a; ++b;
    }
    return *a==*b;
}
static int class_is(HWND hwnd,const char *wanted) {
    char name[96]; name[0]=0;
    return GetClassNameA(hwnd,name,95)>0 && same_ascii(name,wanted);
}
static HWND direct_child_class(HWND parent,const char *wanted) {
    HWND child=GetWindow(parent,GW_CHILD);
    while(child) {
        if(class_is(child,wanted)) return child;
        child=GetWindow(child,GW_HWNDNEXT);
    }
    return (HWND)0;
}
static void collect_row_hosts(HWND parent, HWND *rows, uint32_t *count) {
    HWND child=GetWindow(parent,GW_CHILD);
    while(child && *count<MAX_ROWS) {
        if(class_is(child,"PharoSpecWin32RuntimeWindow") &&
           direct_child_class(child,"Edit") && direct_child_class(child,"Button")) {
            uint32_t i; int duplicate=0;
            for(i=0;i<*count;++i) if(rows[i]==child) duplicate=1;
            if(!duplicate) rows[(*count)++]=child;
        }
        collect_row_hosts(child,rows,count);
        child=GetWindow(child,GW_HWNDNEXT);
    }
}
static void sort_rows_by_top(HWND *rows,uint32_t count) {
    uint32_t i,j;
    for(i=0;i<count;++i) for(j=i+1;j<count;++j) {
        RECT a,b;
        if(GetWindowRect(rows[i],&a) && GetWindowRect(rows[j],&b) && b.top<a.top) {
            HWND t=rows[i]; rows[i]=rows[j]; rows[j]=t;
        }
    }
}
static uint32_t wait_rows(HWND top,uint32_t wanted,HWND *rows) {
    DWORD attempt;
    for(attempt=0;attempt<480u;++attempt) {
        uint32_t count=0;
        collect_row_hosts(top,rows,&count);
        if(count>=wanted) { sort_rows_by_top(rows,count); return count; }
        Sleep(25u);
    }
    return 0;
}
static int drive_row_three(HWND row) {
    HWND edit=direct_child_class(row,"Edit");
    HWND button=direct_child_class(row,"Button");
    HWND content;
    HWND viewport;
    if(!edit || !button) return 0;
    SendMessageW(edit,WM_LBUTTONDOWN_VALUE,0,0);
    SendMessageW(edit,WM_LBUTTONUP_VALUE,0,0);
    SendMessageW(button,BM_CLICK_VALUE,0,0);
    SendMessageW(edit,WM_LBUTTONDBLCLK_VALUE,0,0);
    content=GetParent(row);
    viewport=content ? GetParent(content) : (HWND)0;
    if(!viewport) return 0;
    SendMessageW(viewport,WM_VSCROLL_VALUE,SB_PAGEDOWN_VALUE,0);
    return 1;
}

int mainCRTStartup(void) {
    HWND top=(HWND)0;
    HWND rows[MAX_ROWS];
    DWORD i;
    uint32_t count;
    for(i=0;i<480u && !top;++i) {
        top=FindWindowA((const char*)0,"Spec Win32 v050 ComponentList Smoke");
        if(!top) Sleep(25u);
    }
    if(!top) return 2;
    count=wait_rows(top,12u,rows);
    if(count<12u) return 3;
    write_text("COMPONENTLIST-INJECTOR-ROWS-PASS\r\n");
    if(!drive_row_three(rows[2])) return 4;
    write_text("COMPONENTLIST-INJECTOR-INTERACTION-SENT\r\n");
    count=wait_rows(top,13u,rows);
    if(count<13u) return 5;
    if(!direct_child_class(rows[12],"Edit") || !direct_child_class(rows[12],"Button")) return 6;
    write_text("COMPONENTLIST-INJECTOR-DYNAMIC-PASS\r\n");
    Sleep(400u);
    return 0;
}
