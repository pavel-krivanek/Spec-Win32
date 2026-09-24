#include <stdint.h>
#if defined(_WIN32)
# define IMP __declspec(dllimport)
# define WINAPI __stdcall
#else
# define IMP
# define WINAPI
#endif
typedef void *HWND; typedef int32_t BOOL; typedef uint32_t DWORD; typedef uint32_t UINT; typedef uintptr_t WPARAM; typedef intptr_t LPARAM; typedef intptr_t LRESULT;
IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI Sleep(DWORD);
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONUP 0x0202u
static LPARAM point_param(int x,int y){return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y<<16)|(uint16_t)x);}
static HWND find_child(HWND top,const char *klass,const char *caption){return FindWindowExA(top,0,klass,caption);}
static HWND find_button(HWND top,const char *caption){HWND h=find_child(top,"Button",caption);if(!h)h=find_child(top,"BUTTON",caption);return h;}
static HWND find_static(HWND top,const char *caption){HWND h=find_child(top,"Static",caption);if(!h)h=find_child(top,"STATIC",caption);return h;}
static HWND wait_button(HWND top,const char *caption){int i;HWND h;for(i=0;i<100;i++){h=find_button(top,caption);if(h)return h;Sleep(50u);}return 0;}
static HWND wait_static(HWND top,const char *caption){int i;HWND h;for(i=0;i<100;i++){h=find_static(top,caption);if(h)return h;Sleep(50u);}return 0;}
static void click(HWND h){SendMessageW(h,WM_LBUTTONDOWN,1u,point_param(12,12));SendMessageW(h,WM_LBUTTONUP,0u,point_param(12,12));Sleep(200u);}
int mainCRTStartup(void){HWND top,h;
 top=FindWindowA(0,"Spec Win32 v035 Button Action Bars"); if(!top)return 2;
 if(!wait_static(top,"Ready"))return 3;
 h=wait_button(top,"Left action"); if(!h)return 4; click(h);
 h=wait_button(top,"Right action"); if(!h)return 5; click(h);
 h=wait_button(top,"OK"); if(!h)return 6; click(h);
 h=wait_button(top,"Cancel"); if(!h)return 7; click(h);
 Sleep(800u);
 return 0;
}
