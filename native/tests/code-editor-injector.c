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
typedef uint16_t WCHAR;
typedef struct RECT { int32_t left; int32_t top; int32_t right; int32_t bottom; } RECT;
typedef struct POINT { int32_t x; int32_t y; } POINT;

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI GetWindow(HWND, UINT);
IMP int32_t WINAPI GetClassNameA(HWND, char *, int32_t);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define GW_HWNDNEXT 2u
#define GW_CHILD 5u
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define WM_GETTEXTLENGTH_VALUE 0x000Eu
#define EM_SETSEL_VALUE 0x00B1u
#define EM_GETRECT_VALUE 0x00B2u
#define EM_REPLACESEL_VALUE 0x00C2u
#define EM_GETLINECOUNT_VALUE 0x00BAu
#define EM_GETFIRSTVISIBLELINE_VALUE 0x00CEu
#define EM_LINESCROLL_VALUE 0x00B6u

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
static HWND find_descendant_class(HWND parent,const char *wanted) {
    HWND child=GetWindow(parent,GW_CHILD);
    while(child) {
        HWND nested;
        if(class_is(child,wanted)) return child;
        nested=find_descendant_class(child,wanted);
        if(nested) return nested;
        child=GetWindow(child,GW_HWNDNEXT);
    }
    return (HWND)0;
}


int mainCRTStartup(void) {
    HWND top=(HWND)0;
    HWND rich=(HWND)0;
    DWORD attempt;
    LRESULT line_count;
    RECT format_rect = {0,0,0,0};
    static const WCHAR injected[] = {
        '\r','\n','"','i','n','j','e','c','t','e','d',' ',':','=',' ', '9','9','.','"','\r','\n',0
    };
    for(attempt=0;attempt<480u && !top;++attempt) {
        top=FindWindowA((const char*)0,"Spec Win32 v052 Code Smoke");
        if(!top) Sleep(25u);
    }
    if(!top) return 2;
    for(attempt=0;attempt<240u && !rich;++attempt) {
        rich=find_descendant_class(top,"RICHEDIT50W");
        if(!rich) Sleep(25u);
    }
    if(!rich) return 3;
    write_text("CODE-INJECTOR-RICHEDIT-PASS\r\n");
    for(attempt=0;attempt<240u;++attempt) {
        format_rect.left=format_rect.top=format_rect.right=format_rect.bottom=0;
        SendMessageW(rich,EM_GETRECT_VALUE,0,(LPARAM)(uintptr_t)&format_rect);
        if(format_rect.left >= 48) break;
        Sleep(25u);
    }
    if(format_rect.left < 48) return 7;
    write_text("CODE-INJECTOR-GUTTER-PASS\r\n");
    line_count=SendMessageW(rich,EM_GETLINECOUNT_VALUE,0,0);
    if(line_count<20) return 4;
    /* Do not send the RichEdit 3+ pointer form of EM_POSFROMCHAR from this
       external process: WM_USER messages do not marshal caller pointers.
       Line-number placement is exercised in-process by the runtime and is
       covered by the visual gutter audit. */
    write_text("CODE-INJECTOR-LINECOUNT-PASS\r\n");
    SendMessageW(rich,EM_SETSEL_VALUE,(WPARAM)(uint32_t)-1,(LPARAM)-1);
    SendMessageW(rich,EM_REPLACESEL_VALUE,1u,(LPARAM)(uintptr_t)injected);
    if(SendMessageW(rich,WM_GETTEXTLENGTH_VALUE,0,0)<=0) return 5;
    SendMessageW(rich,EM_LINESCROLL_VALUE,0,18);
    if(SendMessageW(rich,EM_GETFIRSTVISIBLELINE_VALUE,0,0)<=0) return 6;
    write_text("CODE-INJECTOR-EDIT-SCROLL-PASS\r\n");
    Sleep(500u);
    return 0;
}
