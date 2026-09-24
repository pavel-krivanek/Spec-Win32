#include <stdint.h>
#if defined(_WIN32)
# define IMP __declspec(dllimport)
# define WINAPI __stdcall
#else
# define IMP
# define WINAPI
#endif

typedef void *HWND;
typedef void *HANDLE;
typedef uint32_t DWORD;
typedef uint32_t UINT;
typedef int32_t BOOL;
typedef uint64_t WPARAM;
typedef int64_t LPARAM;
typedef int64_t LRESULT;
typedef uint16_t WCHAR;

IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowW(const WCHAR *, const WCHAR *);
IMP HWND WINAPI GetWindow(HWND, UINT);
IMP int32_t WINAPI GetClassNameA(HWND, char *, int32_t);
IMP int32_t WINAPI GetWindowTextW(HWND, WCHAR *, int32_t);
IMP BOOL WINAPI SetWindowTextW(HWND, const WCHAR *);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP BOOL WINAPI IsWindowEnabled(HWND);
IMP BOOL WINAPI IsWindowVisible(HWND);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI GetStdHandle(DWORD);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);

#define GW_HWNDNEXT 2u
#define GW_OWNER 4u
#define GW_CHILD 5u
#define WM_COMMAND 0x0111u
#define IDOK 1u
#define IDCANCEL 2u
#define STD_OUTPUT_HANDLE ((DWORD)-11)

static DWORD text_length(const char *s){DWORD n=0;while(s[n])++n;return n;}
static void write_text(const char *s){DWORD w=0;WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,text_length(s),&w,(void*)0);}
static int same_ascii(const char*a,const char*b){while(*a&&*b&&*a==*b){++a;++b;}return *a==*b;}
static int same_wide(const WCHAR*a,const WCHAR*b){while(*a&&*b&&*a==*b){++a;++b;}return *a==*b;}

static HWND find_dialog_title(const WCHAR *title) {
    return FindWindowW((const WCHAR*)0,title);
}
static HWND find_owned_dialog(HWND owner) {
    HWND w=FindWindowA((const char*)0,(const char*)0);
    while(w){
        char cls[64]; cls[0]=0;
        if(GetClassNameA(w,cls,63)>0 && same_ascii(cls,"#32770") && IsWindowVisible(w) && GetWindow(w,GW_OWNER)==owner)
            return w;
        w=GetWindow(w,GW_HWNDNEXT);
    }
    return (HWND)0;
}
static HWND find_edit_recursive(HWND parent) {
    HWND child=GetWindow(parent,GW_CHILD);
    while(child){
        char cls[64]; cls[0]=0;
        if(GetClassNameA(child,cls,63)>0 && same_ascii(cls,"Edit")) return child;
        { HWND nested=find_edit_recursive(child); if(nested) return nested; }
        child=GetWindow(child,GW_HWNDNEXT);
    }
    return (HWND)0;
}
static int contains_message_recursive(HWND parent,const WCHAR *expected){
    HWND child=GetWindow(parent,GW_CHILD);
    while(child){
        WCHAR text[256]; text[0]=0;
        if(GetWindowTextW(child,text,255)>0 && same_wide(text,expected)) return 1;
        if(contains_message_recursive(child,expected)) return 1;
        child=GetWindow(child,GW_HWNDNEXT);
    }
    return 0;
}
static HWND wait_dialog_title(const WCHAR *title,DWORD loops){
    DWORD i; HWND w=(HWND)0;
    for(i=0;i<loops && !w;++i){w=find_dialog_title(title);if(!w)Sleep(25u);} return w;
}
static HWND wait_owned_dialog(HWND owner,DWORD loops){
    DWORD i; HWND w=(HWND)0;
    for(i=0;i<loops && !w;++i){w=find_owned_dialog(owner);if(!w)Sleep(25u);} return w;
}

int mainCRTStartup(void){
    HWND owner=(HWND)0, dialog, edit;
    DWORD i;
    static const WCHAR info_title[]={ 'I','n','f','o','r','m','a','t','i','o','n',0 };
    static const WCHAR error_title[]={ 'E','r','r','o','r',0 };
    static const WCHAR info_message[]={ 'I','n','f','o','r','m','a','t','i','o','n',' ','U','T','F','-','8',':',' ',0x0050,0x0159,0x00ed,0x006c,0x0069,0x0161,' ',0x017e,0x006c,0x0075,0x0165,0x006f,0x0075,0x010d,0x006b,0x00fd,' ',0x006b,0x016f,0x0148,' ',0xd83d,0xde42,0 };
    static const WCHAR error_message[]={ 'E','r','r','o','r',' ','U','T','F','-','8',':',' ',0x010d,0x0065,0x0072,0x0076,0x0065,0x006e,0x00e1,' ',0x0063,0x0068,0x0079,0x0062,0x0061,' ',0xd83d,0xde42,0 };
    static const WCHAR open_title[]={ 'S','p','e','c',' ','W','i','n','3','2',' ','v','0','4','4',' ','O','p','e','n',' ','C','a','n','c','e','l',0 };
    static const WCHAR save_title[]={ 'S','p','e','c',' ','W','i','n','3','2',' ','v','0','4','4',' ','S','a','v','e',' ','U','T','F','8',0 };
    static const WCHAR utf8_name[]={ 0x017e,0x006c,0x0075,0x0165,0x006f,0x0075,0x010d,0x006b,0x00fd,'-',0xd83d,0xde42,'.','t','x','t',0 };

    for(i=0;i<400u && !owner;++i){owner=FindWindowA((const char*)0,"Spec Win32 v044 Services Owner");if(!owner)Sleep(25u);}
    if(!owner)return 2;

    dialog=wait_dialog_title(info_title,400u); if(!dialog)return 3;
    if(IsWindowEnabled(owner))return 4;
    if(!contains_message_recursive(dialog,info_message))return 5;
    write_text("SERVICES-INJECTOR-INFORM-UTF8-OWNER-PASS\r\n");
    SendMessageW(dialog,WM_COMMAND,IDOK,0);

    dialog=wait_dialog_title(error_title,400u); if(!dialog)return 6;
    if(IsWindowEnabled(owner))return 7;
    if(!contains_message_recursive(dialog,error_message))return 8;
    write_text("SERVICES-INJECTOR-ERROR-UTF8-OWNER-PASS\r\n");
    SendMessageW(dialog,WM_COMMAND,IDOK,0);

    dialog=wait_dialog_title(open_title,400u); if(!dialog)return 9;
    if(IsWindowEnabled(owner))return 10;
    write_text("SERVICES-INJECTOR-OPEN-OWNER-PASS\r\n");
    SendMessageW(dialog,WM_COMMAND,IDCANCEL,0);

    dialog=wait_dialog_title(save_title,400u); if(!dialog)return 11;
    if(IsWindowEnabled(owner))return 12;
    edit=find_edit_recursive(dialog); if(!edit)return 13;
    if(!SetWindowTextW(edit,utf8_name))return 14;
    write_text("SERVICES-INJECTOR-SAVE-DIALOG-READY\r\n");
    Sleep(800u);
    SendMessageW(dialog,WM_COMMAND,IDOK,0);

    /* The folder browser uses a generic #32770 title under Wine; it is the next owner-modal dialog. */
    dialog=wait_owned_dialog(owner,500u); if(!dialog)return 15;
    if(IsWindowEnabled(owner))return 16;
    write_text("SERVICES-INJECTOR-FOLDER-OWNER-PASS\r\n");
    SendMessageW(dialog,WM_COMMAND,IDCANCEL,0);

    for(i=0;i<400u;++i){owner=FindWindowA((const char*)0,"Spec Win32 v044 Wait Active");if(owner)break;Sleep(25u);} if(!owner)return 17;
    write_text("SERVICES-INJECTOR-WAIT-WINDOW-PASS\r\n");

    owner=(HWND)0;
    for(i=0;i<400u && !owner;++i){owner=FindWindowA((const char*)0,"Spec Win32 v044 Wait Restored");if(!owner)Sleep(25u);} if(!owner)return 18;
    write_text("SERVICES-INJECTOR-CURSOR-RESTORE-WINDOW-PASS\r\n");
    return 0;
}
