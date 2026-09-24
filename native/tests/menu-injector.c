#include <stdint.h>
#if defined(_WIN32)
# define IMP __declspec(dllimport)
# define WINAPI __stdcall
#else
# define IMP
# define WINAPI
#endif
typedef void *HWND; typedef int32_t BOOL; typedef int32_t LONG; typedef uint32_t DWORD; typedef uint32_t UINT; typedef uintptr_t WPARAM; typedef intptr_t LPARAM; typedef intptr_t LRESULT;
typedef struct { LONG left, top, right, bottom; } RECT;
IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HWND WINAPI FindWindowExA(HWND, HWND, const char *, const char *);
IMP BOOL WINAPI GetWindowRect(HWND, RECT *);
IMP BOOL WINAPI SetCursorPos(int32_t, int32_t);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI mouse_event(DWORD, DWORD, DWORD, DWORD, uintptr_t);
IMP void WINAPI Sleep(DWORD);
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONUP 0x0202u
#define MOUSEEVENTF_LEFTDOWN 0x0002u
#define MOUSEEVENTF_LEFTUP 0x0004u
static LPARAM point_param(int x,int y){return (LPARAM)(uintptr_t)(((uint32_t)(uint16_t)y<<16)|(uint16_t)x);} 
static HWND find_button(HWND top,const char *caption){HWND h=FindWindowExA(top,0,"Button",caption);if(!h)h=FindWindowExA(top,0,"BUTTON",caption);return h;}
static void click(HWND h){SendMessageW(h,WM_LBUTTONDOWN,1u,point_param(12,12));SendMessageW(h,WM_LBUTTONUP,0u,point_param(12,12));}
static HWND wait_menu(void){int i;HWND h;for(i=0;i<80;i++){h=FindWindowA("#32768",0);if(h)return h;Sleep(50u);}return 0;}
static int select_first(HWND menu,DWORD delay){RECT r;if(!GetWindowRect(menu,&r))return 0;Sleep(delay);SetCursorPos(r.left+40,r.top+12);mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);Sleep(350u);return 1;}
int mainCRTStartup(void){HWND top,button,menu;top=FindWindowA(0,"Spec Win32 v031 Menus");if(!top)return 2;
 button=find_button(top,"Standalone popup");if(!button)return 3;click(button);menu=wait_menu();if(!menu)return 4;if(!select_first(menu,3000u))return 5;
 button=find_button(top,"Actions");if(!button)return 6;click(button);menu=wait_menu();if(!menu)return 7;if(!select_first(menu,300u))return 8;
 Sleep(800u);button=find_button(top,"Actions");if(!button)return 9;click(button);menu=wait_menu();if(!menu)return 10;if(!select_first(menu,300u))return 11;return 0;}
