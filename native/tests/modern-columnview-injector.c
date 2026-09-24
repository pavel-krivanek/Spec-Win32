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
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define GW_HWNDNEXT 2u
#define GW_CHILD 5u
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define WM_KEYDOWN_VALUE 0x0100u
#define WM_LBUTTONDOWN_VALUE 0x0201u
#define WM_LBUTTONUP_VALUE 0x0202u
#define MK_LBUTTON_VALUE 0x0001u
#define VK_RETURN_VALUE 0x0Du
#define VK_DOWN_VALUE 0x28u
#define LVM_FIRST_VALUE 0x1000u
#define LVM_GETIMAGELIST_VALUE (LVM_FIRST_VALUE + 2u)
#define LVM_GETITEMCOUNT_VALUE (LVM_FIRST_VALUE + 4u)
#define LVM_GETNEXTITEM_VALUE (LVM_FIRST_VALUE + 12u)
#define LVM_GETHEADER_VALUE (LVM_FIRST_VALUE + 31u)
#define LVM_GETEXTENDEDLISTVIEWSTYLE_VALUE (LVM_FIRST_VALUE + 55u)
#define HDM_FIRST_VALUE 0x1200u
#define HDM_GETITEMCOUNT_VALUE (HDM_FIRST_VALUE + 0u)
#define LVSIL_SMALL_VALUE 1u
#define LVNI_SELECTED_VALUE 0x0002u
#define LVS_EX_SUBITEMIMAGES_VALUE 0x00000002u

static DWORD text_length(const char *s) { DWORD n=0; while(s[n]) ++n; return n; }
static void write_text(const char *s) {
    DWORD written=0; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,text_length(s),&written,(void*)0);
}
static void write_i64(int64_t value) {
    char b[32]; int n=0, i; uint64_t u;
    if(value<0){ b[n++]='-'; u=(uint64_t)(-value); } else u=(uint64_t)value;
    { char r[24]; int rn=0; do { r[rn++]=(char)('0'+(u%10u)); u/=10u; } while(u); for(i=rn-1;i>=0;--i)b[n++]=r[i]; }
    b[n]=0; write_text(b);
}
static int same_string(const char *a,const char *b) {
    while(*a && *b && *a==*b){++a;++b;} return *a==*b;
}
static void find_list(HWND parent, HWND *list) {
    HWND child=GetWindow(parent,GW_CHILD);
    while(child){
        char name[96]; name[0]=0;
        if(GetClassNameA(child,name,95)>0 && same_string(name,"SysListView32")) {
            *list=child; return;
        }
        find_list(child,list);
        if(*list) return;
        child=GetWindow(child,GW_HWNDNEXT);
    }
}
static int initial_ready(HWND list) {
    HWND header=(HWND)(uintptr_t)SendMessageW(list,LVM_GETHEADER_VALUE,0,0);
    LRESULT exstyle=SendMessageW(list,LVM_GETEXTENDEDLISTVIEWSTYLE_VALUE,0,0);
    return SendMessageW(list,LVM_GETITEMCOUNT_VALUE,0,0)==4 &&
           header && SendMessageW(header,HDM_GETITEMCOUNT_VALUE,0,0)==3 &&
           SendMessageW(list,LVM_GETIMAGELIST_VALUE,LVSIL_SMALL_VALUE,0)!=0 &&
           (((uint32_t)exstyle & LVS_EX_SUBITEMIMAGES_VALUE)!=0u);
}
static int wait_initial(HWND top, HWND *list) {
    DWORD i;
    for(i=0;i<400u;++i){
        *list=(HWND)0; find_list(top,list);
        if(*list && initial_ready(*list)) return 1;
        Sleep(25u);
    }
    return 0;
}
static int activate_second_row(HWND list) {
    LRESULT selected;
    /* Click the second visible row in client coordinates. The native small-image
       list gives report rows a stable ~20px height; y=49 is within row 2 after
       the standard header under both Windows and the bundled Wine common control. */
    LPARAM point=(LPARAM)(((uint32_t)49u << 16) | 50u);
    SendMessageW(list,WM_LBUTTONDOWN_VALUE,(WPARAM)MK_LBUTTON_VALUE,point);
    SendMessageW(list,WM_LBUTTONUP_VALUE,0,point);
    selected=SendMessageW(list,LVM_GETNEXTITEM_VALUE,(WPARAM)(intptr_t)-1,(LPARAM)LVNI_SELECTED_VALUE);
    if(selected!=1) return 0;
    SendMessageW(list,WM_KEYDOWN_VALUE,(WPARAM)VK_RETURN_VALUE,0);
    return 1;
}
static int click_score_header(HWND list) {
    HWND header=(HWND)(uintptr_t)SendMessageW(list,LVM_GETHEADER_VALUE,0,0);
    LPARAM point=(LPARAM)(((uint32_t)10u << 16) | 630u);
    DWORD i;
    if(!header) return 0;
    SendMessageW(header,WM_LBUTTONDOWN_VALUE,(WPARAM)MK_LBUTTON_VALUE,point);
    SendMessageW(header,WM_LBUTTONUP_VALUE,0,point);
    for(i=0;i<240u;++i){
        LRESULT selected=SendMessageW(list,LVM_GETNEXTITEM_VALUE,(WPARAM)(intptr_t)-1,(LPARAM)LVNI_SELECTED_VALUE);
        if(selected==0) return 1;
        Sleep(25u);
    }
    return 0;
}

static int wait_refreshed(HWND list) {
    DWORD i;
    for(i=0;i<400u;++i){
        LRESULT count=SendMessageW(list,LVM_GETITEMCOUNT_VALUE,0,0);
        LRESULT selected=SendMessageW(list,LVM_GETNEXTITEM_VALUE,(WPARAM)(intptr_t)-1,(LPARAM)LVNI_SELECTED_VALUE);
        LRESULT exstyle=SendMessageW(list,LVM_GETEXTENDEDLISTVIEWSTYLE_VALUE,0,0);
        if(count==3 && selected==2 &&
           SendMessageW(list,LVM_GETIMAGELIST_VALUE,LVSIL_SMALL_VALUE,0)!=0 &&
           (((uint32_t)exstyle & LVS_EX_SUBITEMIMAGES_VALUE)!=0u)) return 1;
        Sleep(25u);
    }
    write_text("COLUMNVIEW-INJECTOR-REFRESH-DIAG count=");
    write_i64((int64_t)SendMessageW(list,LVM_GETITEMCOUNT_VALUE,0,0));
    write_text(" selected=");
    write_i64((int64_t)SendMessageW(list,LVM_GETNEXTITEM_VALUE,(WPARAM)(intptr_t)-1,(LPARAM)LVNI_SELECTED_VALUE));
    write_text(" imageList=");
    write_i64((int64_t)(uintptr_t)SendMessageW(list,LVM_GETIMAGELIST_VALUE,LVSIL_SMALL_VALUE,0));
    write_text(" exstyle=");
    write_i64((int64_t)SendMessageW(list,LVM_GETEXTENDEDLISTVIEWSTYLE_VALUE,0,0));
    write_text("\r\n");
    return 0;
}
static int wait_cleared(HWND list) {
    DWORD i;
    for(i=0;i<400u;++i){
        LRESULT exstyle=SendMessageW(list,LVM_GETEXTENDEDLISTVIEWSTYLE_VALUE,0,0);
        if(SendMessageW(list,LVM_GETITEMCOUNT_VALUE,0,0)==2 &&
           SendMessageW(list,LVM_GETIMAGELIST_VALUE,LVSIL_SMALL_VALUE,0)==0 &&
           (((uint32_t)exstyle & LVS_EX_SUBITEMIMAGES_VALUE)==0u)) return 1;
        Sleep(25u);
    }
    return 0;
}

int mainCRTStartup(void) {
    HWND top=(HWND)0, list=(HWND)0;
    DWORD i;
    for(i=0;i<400u && !top;++i){
        top=FindWindowA((const char*)0,"Spec Win32 v047 Modern ColumnView");
        if(!top) Sleep(25u);
    }
    if(!top) return 2;
    if(!wait_initial(top,&list)) return 3;
    write_text("COLUMNVIEW-INJECTOR-INITIAL-SUBITEM-IMAGES-PASS\r\n");
    if(!activate_second_row(list)) return 4;
    write_text("COLUMNVIEW-INJECTOR-NATIVE-SELECTION-PASS\r\n");
    if(!click_score_header(list)) return 7;
    write_text("COLUMNVIEW-INJECTOR-NATIVE-SORT-PASS\r\n");
    if(!wait_refreshed(list)) return 5;
    write_text("COLUMNVIEW-INJECTOR-REFRESH-SUBITEM-IMAGES-PASS\r\n");
    if(!wait_cleared(list)) return 6;
    write_text("COLUMNVIEW-INJECTOR-IMAGE-LIST-CLEAR-PASS\r\n");
    Sleep(800u);
    return 0;
}
