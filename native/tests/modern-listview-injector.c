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
typedef struct { int32_t x; int32_t y; } POINT;
typedef struct { HWND hwndFrom; uintptr_t idFrom; UINT code; } NMHDR;
typedef struct {
    UINT mask; int32_t iItem; int32_t iSubItem; UINT state; UINT stateMask;
    uint16_t *pszText; int32_t cchTextMax; int32_t iImage; LPARAM lParam;
} LVITEMW;
typedef struct {
    NMHDR hdr; int32_t iItem; int32_t iSubItem; UINT uNewState; UINT uOldState;
    UINT uChanged; POINT ptAction; LPARAM lParam;
} NMLISTVIEW;
typedef struct {
    NMHDR hdr; int32_t iItem; int32_t iSubItem; UINT uNewState; UINT uOldState;
    UINT uChanged; POINT ptAction; LPARAM lParam; UINT uKeyFlags;
} NMITEMACTIVATE;

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI GetWindow(HWND, UINT);
IMP int32_t WINAPI GetClassNameA(HWND, char *, int32_t);
IMP HWND WINAPI GetParent(HWND);
IMP int32_t WINAPI GetDlgCtrlID(HWND);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define GW_HWNDNEXT 2u
#define GW_CHILD 5u
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define WM_COMMAND_VALUE 0x0111u
#define WM_NOTIFY_VALUE 0x004Eu
#define WM_KEYDOWN_VALUE 0x0100u
#define VK_RETURN_VALUE 0x0Du
#define VK_DOWN_VALUE 0x28u
#define CBN_SELCHANGE_VALUE 1u
#define CB_GETCOUNT_VALUE 0x0146u
#define CB_GETCURSEL_VALUE 0x0147u
#define CB_SETCURSEL_VALUE 0x014Eu
#define LVM_FIRST_VALUE 0x1000u
#define LVM_GETIMAGELIST_VALUE (LVM_FIRST_VALUE + 2u)
#define LVM_GETITEMCOUNT_VALUE (LVM_FIRST_VALUE + 4u)
#define LVM_GETNEXTITEM_VALUE (LVM_FIRST_VALUE + 12u)
#define LVM_SETITEMSTATE_VALUE (LVM_FIRST_VALUE + 43u)
#define LVSIL_SMALL_VALUE 1u
#define LVNI_SELECTED_VALUE 0x0002u
#define LVIF_STATE_VALUE 0x0008u
#define LVIS_SELECTED_VALUE 0x0002u
#define LVN_ITEMCHANGED_VALUE (-101)
#define NM_DBLCLK_VALUE (-3)

static DWORD text_length(const char *s) { DWORD n=0; while(s[n]) ++n; return n; }
static void write_text(const char *s) {
    DWORD written=0; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,text_length(s),&written,(void*)0);
}
static int same_string(const char *a,const char *b) {
    while(*a && *b && *a==*b){++a;++b;} return *a==*b;
}
static void find_controls(HWND parent, HWND *combo, HWND *list) {
    HWND child=GetWindow(parent,GW_CHILD);
    while(child){
        char name[96]; name[0]=0;
        if(GetClassNameA(child,name,95)>0){
            if(!*combo && same_string(name,"ComboBox")) *combo=child;
            if(!*list && same_string(name,"SysListView32")) *list=child;
        }
        if(!*combo || !*list) find_controls(child,combo,list);
        child=GetWindow(child,GW_HWNDNEXT);
    }
}
static int wait_initial(HWND top, HWND *combo, HWND *list) {
    DWORD i;
    for(i=0;i<320u;++i){
        *combo=(HWND)0; *list=(HWND)0; find_controls(top,combo,list);
        if(*combo && *list && SendMessageW(*combo,CB_GETCOUNT_VALUE,0,0)==4 &&
           SendMessageW(*list,LVM_GETITEMCOUNT_VALUE,0,0)==5 &&
           SendMessageW(*list,LVM_GETIMAGELIST_VALUE,LVSIL_SMALL_VALUE,0)!=0) return 1;
        Sleep(25u);
    }
    return 0;
}
static void notify_combo_selection(HWND combo,int32_t zero_index) {
    HWND parent=GetParent(combo);
    int32_t id=GetDlgCtrlID(combo);
    WPARAM command=(WPARAM)((uint32_t)id & 0xFFFFu) | ((WPARAM)CBN_SELCHANGE_VALUE<<16);
    SendMessageW(combo,CB_SETCURSEL_VALUE,(WPARAM)zero_index,0);
    SendMessageW(parent,WM_COMMAND_VALUE,command,(LPARAM)(uintptr_t)combo);
}
static int notify_list_second_row(HWND list) {
    LRESULT selected;
    /* Drive the actual common control with pointer-free keyboard messages.
       First Down selects row 1, second Down advances to row 2. */
    SendMessageW(list,WM_KEYDOWN_VALUE,(WPARAM)VK_DOWN_VALUE,0);
    SendMessageW(list,WM_KEYDOWN_VALUE,(WPARAM)VK_DOWN_VALUE,0);
    selected=SendMessageW(list,LVM_GETNEXTITEM_VALUE,(WPARAM)(intptr_t)-1,(LPARAM)LVNI_SELECTED_VALUE);
    if(selected!=1) return 0;
    SendMessageW(list,WM_KEYDOWN_VALUE,(WPARAM)VK_RETURN_VALUE,0);
    return 1;
}
static int wait_refreshed(HWND combo, HWND list) {
    DWORD i;
    for(i=0;i<320u;++i){
        LRESULT combo_count=SendMessageW(combo,CB_GETCOUNT_VALUE,0,0);
        LRESULT list_count=SendMessageW(list,LVM_GETITEMCOUNT_VALUE,0,0);
        LRESULT combo_selected=SendMessageW(combo,CB_GETCURSEL_VALUE,0,0);
        LRESULT list_selected=SendMessageW(list,LVM_GETNEXTITEM_VALUE,(WPARAM)(intptr_t)-1,(LPARAM)LVNI_SELECTED_VALUE);
        if(combo_count==3 && list_count==4 && combo_selected==0 && list_selected==2 &&
           SendMessageW(list,LVM_GETIMAGELIST_VALUE,LVSIL_SMALL_VALUE,0)!=0) return 1;
        Sleep(25u);
    }
    return 0;
}


static int wait_icon_list_cleared(HWND list) {
    DWORD i;
    for(i=0;i<320u;++i){
        if(SendMessageW(list,LVM_GETITEMCOUNT_VALUE,0,0)==2 &&
           SendMessageW(list,LVM_GETIMAGELIST_VALUE,LVSIL_SMALL_VALUE,0)==0) return 1;
        Sleep(25u);
    }
    return 0;
}

int mainCRTStartup(void) {
    HWND top=(HWND)0, combo=(HWND)0, list=(HWND)0;
    DWORD i;
    for(i=0;i<400u && !top;++i){
        top=FindWindowA((const char*)0,"Spec Win32 v046 Modern ListView");
        if(!top) Sleep(25u);
    }
    if(!top) return 2;
    if(!wait_initial(top,&combo,&list)) return 3;
    if(SendMessageW(combo,CB_GETCURSEL_VALUE,0,0)!=1) return 4;
    write_text("MODERN-INJECTOR-INITIAL-ICONLIST-PASS\r\n");
    notify_combo_selection(combo,2);
    if(!notify_list_second_row(list)) return 8;
    write_text("MODERN-INJECTOR-LIST-SELECTION-PASS\r\n");
    if(!wait_refreshed(combo,list)) return 5;
    write_text("MODERN-INJECTOR-NATIVE-REFRESH-ICONLIST-PASS\r\n");
    if(!wait_icon_list_cleared(list)) return 9;
    write_text("MODERN-INJECTOR-ICONLIST-CLEAR-PASS\r\n");
    Sleep(900u);
    return 0;
}
