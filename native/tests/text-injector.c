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
typedef int64_t LONG_PTR;
typedef uint64_t WPARAM;
typedef int64_t LPARAM;
typedef int64_t LRESULT;
typedef uint16_t WCHAR;
typedef void *HANDLE;

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI GetWindow(HWND, uint32_t);
IMP int32_t WINAPI GetClassNameA(HWND, char *, int32_t);
IMP LONG_PTR WINAPI GetWindowLongPtrW(HWND, int32_t);
IMP LRESULT WINAPI SendMessageW(HWND, uint32_t, WPARAM, LPARAM);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define GW_HWNDNEXT 2u
#define GW_CHILD 5u
#define GWL_STYLE (-16)
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define WS_HSCROLL 0x00100000u
#define WS_VSCROLL 0x00200000u
#define ES_MULTILINE 0x00000004u
#define ES_AUTOHSCROLL 0x00000080u
#define ES_READONLY 0x00000800u
#define EM_SETSEL 0x00b1u
#define EM_REPLACESEL 0x00c2u

static DWORD text_length(const char *s) { DWORD n=0; while(s[n]) ++n; return n; }
static void write_text(const char *s) { DWORD w=0; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,text_length(s),&w,(void*)0); }
static int same_string(const char *a,const char *b){while(*a&&*b&&*a==*b){++a;++b;}return *a==*b;}

static HWND find_multiline_edit(HWND parent) {
    HWND child = GetWindow(parent, GW_CHILD);
    while (child) {
        char class_name[64];
        LONG_PTR style;
        class_name[0]=0;
        if (GetClassNameA(child,class_name,63)>0 && same_string(class_name,"Edit")) {
            style=GetWindowLongPtrW(child,GWL_STYLE);
            if ((((uint64_t)style)&ES_MULTILINE)!=0) return child;
        }
        {
            HWND nested=find_multiline_edit(child);
            if(nested) return nested;
        }
        child=GetWindow(child,GW_HWNDNEXT);
    }
    return (HWND)0;
}

int mainCRTStartup(void) {
    HWND top=(HWND)0, edit=(HWND)0, wrapped_edit=(HWND)0, nowrap_edit=(HWND)0;
    DWORD i;
    LONG_PTR style;
    static const WCHAR insertion[] = { '-', 'N','A','T','I','V','E','-', 0 };
    for(i=0;i<400u && !top;++i){top=FindWindowA((const char*)0,"Spec Win32 v043 Text"); if(!top) Sleep(25u);}
    if(!top) return 2;
    for(i=0;i<240u && !edit;++i){edit=find_multiline_edit(top); if(!edit) Sleep(25u);}
    if(!edit) return 3;
    style=GetWindowLongPtrW(edit,GWL_STYLE);
    if((((uint64_t)style)&ES_MULTILINE)==0) return 4;
    if((((uint64_t)style)&WS_VSCROLL)==0) return 5;
    if((((uint64_t)style)&WS_HSCROLL)!=0) return 6;
    wrapped_edit=edit;
    write_text("TEXT-INJECTOR-WRAPPED-STYLE-PASS\r\n");

    /* Native UTF-16 offset 11 is after Alpha\\r\\nBeta. Spec logical offset is 10. */
    SendMessageW(edit,EM_SETSEL,(WPARAM)11,(LPARAM)11);
    SendMessageW(edit,EM_REPLACESEL,(WPARAM)1,(LPARAM)(uintptr_t)insertion);
    write_text("TEXT-INJECTOR-EDIT-PASS\r\n");

    for(i=0;i<320u;++i){
        edit=find_multiline_edit(top);
        if(edit){
            style=GetWindowLongPtrW(edit,GWL_STYLE);
            if(((((uint64_t)style)&WS_HSCROLL)!=0) && ((((uint64_t)style)&ES_AUTOHSCROLL)!=0)) break;
        }
        Sleep(25u);
    }
    if(i==320u) return 7;
    nowrap_edit=edit;
    if(nowrap_edit==wrapped_edit) return 10;
    write_text("TEXT-INJECTOR-WRAP-HWND-REPLACED-PASS\r\n");
    write_text("TEXT-INJECTOR-NOWRAP-STYLE-PASS\r\n");

    for(i=0;i<320u;++i){
        edit=find_multiline_edit(top);
        if(edit){
            style=GetWindowLongPtrW(edit,GWL_STYLE);
            if((((uint64_t)style)&ES_READONLY)!=0) break;
        }
        Sleep(25u);
    }
    if(i==320u) return 8;
    write_text("TEXT-INJECTOR-READONLY-PASS\r\n");

    for(i=0;i<320u;++i){
        edit=find_multiline_edit(top);
        if(edit){
            style=GetWindowLongPtrW(edit,GWL_STYLE);
            if(((((uint64_t)style)&ES_READONLY)==0) && ((((uint64_t)style)&WS_HSCROLL)==0)) break;
        }
        Sleep(25u);
    }
    if(i==320u) return 9;
    if(edit==nowrap_edit) return 11;
    write_text("TEXT-INJECTOR-WRAP-RESTORE-HWND-REPLACED-PASS\r\n");
    write_text("TEXT-INJECTOR-WRAP-RESTORED-PASS\r\n");
    Sleep(500u);
    return 0;
}
