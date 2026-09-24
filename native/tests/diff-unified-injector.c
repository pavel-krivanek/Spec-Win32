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

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI GetWindow(HWND, UINT);
IMP int32_t WINAPI GetClassNameA(HWND, char *, int32_t);
IMP int32_t WINAPI GetWindowLongW(HWND, int32_t);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define GW_HWNDNEXT 2u
#define GW_CHILD 5u
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define GWL_STYLE_VALUE (-16)
#define ES_READONLY_VALUE 0x0800u
#define EM_GETLINECOUNT_VALUE 0x00BAu
#define EM_GETFIRSTVISIBLELINE_VALUE 0x00CEu
#define WM_VSCROLL_VALUE 0x0115u
#define SB_PAGEDOWN_VALUE 3u
#define SB_TOP_VALUE 6u

static DWORD text_length(const char *s) { DWORD n=0; while(s[n]) ++n; return n; }
static void write_text(const char *s) {
    DWORD written=0; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,text_length(s),&written,(void*)0);
}
static void write_uint(uint32_t value) {
    char buffer[16]; uint32_t i=0,j;
    if(value==0u) { write_text("0"); return; }
    while(value>0u && i<15u) { buffer[i++]=(char)('0'+(value%10u)); value/=10u; }
    for(j=0;j<i/2u;++j) { char t=buffer[j]; buffer[j]=buffer[i-1u-j]; buffer[i-1u-j]=t; }
    buffer[i]=0; write_text(buffer);
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
static void collect_descendants(HWND parent,const char *wanted,HWND *out,uint32_t *count,uint32_t cap) {
    HWND child=GetWindow(parent,GW_CHILD);
    while(child) {
        if(class_is(child,wanted) && *count<cap) out[(*count)++]=child;
        collect_descendants(child,wanted,out,count,cap);
        child=GetWindow(child,GW_HWNDNEXT);
    }
}
static int wait_synced(HWND a,HWND b,int require_positive) {
    DWORD attempt;
    for(attempt=0;attempt<200u;++attempt) {
        LRESULT la=SendMessageW(a,EM_GETFIRSTVISIBLELINE_VALUE,0,0);
        LRESULT lb=SendMessageW(b,EM_GETFIRSTVISIBLELINE_VALUE,0,0);
        if(la==lb && (!require_positive || la>0)) return 1;
        Sleep(25u);
    }
    return 0;
}

int mainCRTStartup(void) {
    HWND top=(HWND)0;
    HWND rich[4]={(HWND)0,(HWND)0,(HWND)0,(HWND)0};
    uint32_t count=0;
    DWORD attempt;
    for(attempt=0;attempt<480u && !top;++attempt) {
        top=FindWindowA((const char*)0,"Spec Win32 v060 DiffUnified Smoke");
        if(!top) Sleep(25u);
    }
    if(!top) return 2;
    for(attempt=0;attempt<240u;++attempt) {
        count=0; collect_descendants(top,"RICHEDIT50W",rich,&count,4u);
        if(count>=2u) break;
        Sleep(25u);
    }
    if(count!=2u) return 3;
    write_text("DIFF-UNIFIED-INJECTOR-TWO-RICHEDIT-PASS\r\n");
    if((((uint32_t)GetWindowLongW(rich[0],GWL_STYLE_VALUE)) & ES_READONLY_VALUE)==0u ||
       (((uint32_t)GetWindowLongW(rich[1],GWL_STYLE_VALUE)) & ES_READONLY_VALUE)==0u) return 4;
    write_text("DIFF-UNIFIED-INJECTOR-READONLY-PASS\r\n");
    if(SendMessageW(rich[0],EM_GETLINECOUNT_VALUE,0,0)<40 || SendMessageW(rich[1],EM_GETLINECOUNT_VALUE,0,0)<40) return 5;
    SendMessageW(rich[0],WM_VSCROLL_VALUE,SB_PAGEDOWN_VALUE,0);
    SendMessageW(rich[0],WM_VSCROLL_VALUE,SB_PAGEDOWN_VALUE,0);
    if(!wait_synced(rich[0],rich[1],1)) { write_text("SYNC-FAIL-LR a="); write_uint((uint32_t)SendMessageW(rich[0],EM_GETFIRSTVISIBLELINE_VALUE,0,0)); write_text(" b="); write_uint((uint32_t)SendMessageW(rich[1],EM_GETFIRSTVISIBLELINE_VALUE,0,0)); write_text("\r\n"); return 6; }
    write_text("DIFF-UNIFIED-INJECTOR-LEFT-TO-RIGHT-SYNC-PASS\r\n");
    SendMessageW(rich[1],WM_VSCROLL_VALUE,SB_TOP_VALUE,0);
    if(!wait_synced(rich[0],rich[1],0)) { write_text("SYNC-FAIL-RL a="); write_uint((uint32_t)SendMessageW(rich[0],EM_GETFIRSTVISIBLELINE_VALUE,0,0)); write_text(" b="); write_uint((uint32_t)SendMessageW(rich[1],EM_GETFIRSTVISIBLELINE_VALUE,0,0)); write_text("\r\n"); return 7; }
    if(SendMessageW(rich[0],EM_GETFIRSTVISIBLELINE_VALUE,0,0)!=0) return 7;
    write_text("DIFF-UNIFIED-INJECTOR-RIGHT-TO-LEFT-SYNC-PASS\r\n");
    Sleep(300u);
    return 0;
}
