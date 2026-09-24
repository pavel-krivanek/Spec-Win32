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
#define TV_FIRST_VALUE 0x1100u
#define TVM_EXPAND_VALUE (TV_FIRST_VALUE + 2u)
#define TVM_GETCOUNT_VALUE (TV_FIRST_VALUE + 5u)
#define TVM_GETIMAGELIST_VALUE (TV_FIRST_VALUE + 8u)
#define TVM_GETNEXTITEM_VALUE (TV_FIRST_VALUE + 10u)
#define TVM_SELECTITEM_VALUE (TV_FIRST_VALUE + 11u)
#define TVE_EXPAND_VALUE 0x0002u
#define TVGN_ROOT_VALUE 0x0000u
#define TVGN_NEXT_VALUE 0x0001u
#define TVGN_CHILD_VALUE 0x0004u
#define TVGN_CARET_VALUE 0x0009u
#define TVSIL_NORMAL_VALUE 0u
#define LVM_FIRST_VALUE 0x1000u
#define LVM_GETITEMCOUNT_VALUE (LVM_FIRST_VALUE + 4u)
#define LVM_GETNEXTITEM_VALUE (LVM_FIRST_VALUE + 12u)
#define LVM_GETIMAGELIST_VALUE (LVM_FIRST_VALUE + 2u)
#define LVM_GETHEADER_VALUE (LVM_FIRST_VALUE + 31u)
#define LVNI_SELECTED_VALUE 0x0002u
#define LVSIL_SMALL_VALUE 1u
#define HDM_FIRST_VALUE 0x1200u
#define HDM_GETITEMCOUNT_VALUE (HDM_FIRST_VALUE + 0u)

static DWORD text_length(const char *s) { DWORD n=0; while(s[n]) ++n; return n; }
static void write_text(const char *s) {
    DWORD written=0; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,text_length(s),&written,(void*)0);
}
static int same_string(const char *a,const char *b) {
    while(*a && *b && *a==*b){++a;++b;} return *a==*b;
}
static HWND find_class(HWND parent, const char *wanted) {
    HWND child=GetWindow(parent,GW_CHILD);
    while(child){
        char name[96]; name[0]=0;
        if(GetClassNameA(child,name,95)>0 && same_string(name,wanted)) return child;
        {
            HWND nested=find_class(child,wanted);
            if(nested) return nested;
        }
        child=GetWindow(child,GW_HWNDNEXT);
    }
    return (HWND)0;
}
static int wait_initial(HWND top, HWND *tree, HWND *list) {
    DWORD i;
    for(i=0;i<400u;++i){
        HWND header;
        *tree=find_class(top,"SysTreeView32");
        *list=find_class(top,"SysListView32");
        header=*list ? (HWND)(uintptr_t)SendMessageW(*list,LVM_GETHEADER_VALUE,0,0) : (HWND)0;
        if(*tree && *list &&
           SendMessageW(*tree,TVM_GETCOUNT_VALUE,0,0)==5 &&
           SendMessageW(*tree,TVM_GETIMAGELIST_VALUE,TVSIL_NORMAL_VALUE,0)!=0 &&
           SendMessageW(*list,LVM_GETITEMCOUNT_VALUE,0,0)==2 &&
           SendMessageW(*list,LVM_GETIMAGELIST_VALUE,LVSIL_SMALL_VALUE,0)!=0 &&
           header && SendMessageW(header,HDM_GETITEMCOUNT_VALUE,0,0)==2) return 1;
        Sleep(25u);
    }
    return 0;
}
static int drive_expand_and_select(HWND tree) {
    HWND root=(HWND)(uintptr_t)SendMessageW(tree,TVM_GETNEXTITEM_VALUE,TVGN_ROOT_VALUE,0);
    HWND first_child;
    HWND second_child;
    if(!root) return 0;
    SendMessageW(tree,TVM_EXPAND_VALUE,TVE_EXPAND_VALUE,(LPARAM)(uintptr_t)root);
    first_child=(HWND)(uintptr_t)SendMessageW(tree,TVM_GETNEXTITEM_VALUE,TVGN_CHILD_VALUE,(LPARAM)(uintptr_t)root);
    if(!first_child) return 0;
    second_child=(HWND)(uintptr_t)SendMessageW(tree,TVM_GETNEXTITEM_VALUE,TVGN_NEXT_VALUE,(LPARAM)(uintptr_t)first_child);
    if(!second_child) return 0;
    SendMessageW(tree,TVM_SELECTITEM_VALUE,TVGN_CARET_VALUE,(LPARAM)(uintptr_t)second_child);
    return 1;
}
static int wait_synchronized(HWND list) {
    DWORD i;
    for(i=0;i<400u;++i){
        LRESULT selected=SendMessageW(list,LVM_GETNEXTITEM_VALUE,(WPARAM)(uint64_t)-1,LVNI_SELECTED_VALUE);
        if(SendMessageW(list,LVM_GETITEMCOUNT_VALUE,0,0)==4 && selected==2) return 1;
        Sleep(25u);
    }
    return 0;
}
static int wait_refreshed_with_icons(HWND tree, HWND list) {
    DWORD i;
    for(i=0;i<400u;++i){
        if(SendMessageW(tree,TVM_GETCOUNT_VALUE,0,0)==3 &&
           SendMessageW(tree,TVM_GETIMAGELIST_VALUE,TVSIL_NORMAL_VALUE,0)!=0 &&
           SendMessageW(list,LVM_GETITEMCOUNT_VALUE,0,0)==3 &&
           SendMessageW(list,LVM_GETIMAGELIST_VALUE,LVSIL_SMALL_VALUE,0)!=0) return 1;
        Sleep(25u);
    }
    return 0;
}
static int wait_text_only(HWND tree, HWND list) {
    DWORD i;
    for(i=0;i<400u;++i){
        if(SendMessageW(tree,TVM_GETCOUNT_VALUE,0,0)==3 &&
           SendMessageW(tree,TVM_GETIMAGELIST_VALUE,TVSIL_NORMAL_VALUE,0)==0 &&
           SendMessageW(list,LVM_GETITEMCOUNT_VALUE,0,0)==3 &&
           SendMessageW(list,LVM_GETIMAGELIST_VALUE,LVSIL_SMALL_VALUE,0)==0) return 1;
        Sleep(25u);
    }
    return 0;
}

int mainCRTStartup(void) {
    HWND top=(HWND)0, tree=(HWND)0, list=(HWND)0;
    DWORD i;
    for(i=0;i<480u && !top;++i){
        top=FindWindowA((const char*)0,"Spec Win32 v049 Modern TreeColumnView");
        if(!top) Sleep(25u);
    }
    if(!top) return 2;
    if(!wait_initial(top,&tree,&list)) return 3;
    write_text("TREECOLUMN-INJECTOR-INITIAL-COMPOSITE-PASS\r\n");
    if(!drive_expand_and_select(tree)) return 4;
    if(!wait_synchronized(list)) return 5;
    write_text("TREECOLUMN-INJECTOR-NATIVE-SYNC-PASS\r\n");
    if(!wait_refreshed_with_icons(tree,list)) return 6;
    write_text("TREECOLUMN-INJECTOR-REFRESH-IMAGE-LISTS-PASS\r\n");
    if(!wait_text_only(tree,list)) return 7;
    write_text("TREECOLUMN-INJECTOR-IMAGE-LISTS-CLEAR-PASS\r\n");
    Sleep(500u);
    return 0;
}
