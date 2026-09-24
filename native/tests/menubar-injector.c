#include <stdint.h>
#if defined(_WIN32)
# define IMP __declspec(dllimport)
# define WINAPI __stdcall
#else
# define IMP
# define WINAPI
#endif
typedef void *HWND; typedef void *HMENU; typedef int32_t BOOL; typedef int32_t LONG; typedef uint32_t DWORD;
typedef struct { LONG left, top, right, bottom; } RECT;
typedef struct { LONG x, y; } POINT;
IMP HWND WINAPI FindWindowA(const char *, const char *);
IMP HMENU WINAPI GetMenu(HWND);
IMP BOOL WINAPI GetWindowRect(HWND, RECT *);
IMP BOOL WINAPI GetClientRect(HWND, RECT *);
IMP BOOL WINAPI ClientToScreen(HWND, POINT *);
IMP BOOL WINAPI GetMenuItemRect(HWND, HMENU, uint32_t, RECT *);
IMP BOOL WINAPI SetCursorPos(int32_t, int32_t);
IMP void WINAPI mouse_event(DWORD, DWORD, DWORD, DWORD, uintptr_t);
IMP void WINAPI keybd_event(uint8_t, uint8_t, DWORD, uintptr_t);
IMP void WINAPI Sleep(DWORD);
#define MOUSEEVENTF_LEFTDOWN 0x0002u
#define MOUSEEVENTF_LEFTUP 0x0004u
#define KEYEVENTF_KEYUP 0x0002u
#define VK_RETURN 0x0Du
#define VK_END 0x23u
#define VK_RIGHT 0x27u
static void click_xy(int x,int y){SetCursorPos(x,y);mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);}
static void press_key(uint8_t key){keybd_event(key,0u,0u,0);keybd_event(key,0u,KEYEVENTF_KEYUP,0);Sleep(120u);}
static HWND wait_menu(void){int i;HWND h;for(i=0;i<80;i++){h=FindWindowA("#32768",0);if(h)return h;Sleep(50u);}return 0;}
static int open_first_top(HWND w){RECT r;POINT p;HMENU m=GetMenu(w);if(!m)return -1;if(GetMenuItemRect(w,m,0,&r)){click_xy((r.left+r.right)/2,(r.top+r.bottom)/2);}else{if(!GetClientRect(w,&r))return -2;p.x=0;p.y=0;if(!ClientToScreen(w,&p))return -4;click_xy((int)p.x+20,(int)p.y-10);}Sleep(250u);return wait_menu()?1:-3;}
static int choose_row(HWND popup,int row,DWORD hold){RECT r;if(!popup||!GetWindowRect(popup,&r))return 0;Sleep(hold);click_xy(r.left+48,r.top+12+row*22);Sleep(500u);return 1;}
int mainCRTStartup(void){HWND w,p;HMENU root;RECT r;int o;w=FindWindowA(0,"Spec Win32 v033 MenuBar");if(!w)return 2;root=GetMenu(w);if(!root)return 3;if(!GetWindowRect(w,&r))return 4;
 o=open_first_top(w);if(o<0)return 50-o;p=wait_menu();if(!choose_row(p,0,2500u))return 6;
 o=open_first_top(w);if(o<0)return 60-o;p=wait_menu();if(!choose_row(p,1,150u))return 8;
 o=open_first_top(w);if(o<0)return 70-o;press_key(VK_END);press_key(VK_RIGHT);press_key(VK_RETURN);Sleep(500u);
 Sleep(1800u);
 o=open_first_top(w);if(o<0)return 80-o;p=wait_menu();if(!choose_row(p,0,150u))return 10;
 return 0;}
