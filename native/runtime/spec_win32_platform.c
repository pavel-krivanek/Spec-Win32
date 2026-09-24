/*
 * Spec Win32 native runtime, ABI v0.39.
 *
 * Design constraints:
 * - x86-64 Windows, CRT-free
 * - all HWND creation/manipulation happens on one dedicated native UI thread
 * - Pharo sees stable uint64 object IDs, never HWNDs
 * - top-level windows and child controls are owned by the native UI thread
 * - Pharo->Win32 operations are serialized through the UI thread's message queue
 * - Win32->Pharo notifications are normalized into a small SPSC event queue
 * - no callback from a WndProc into Pharo
 *
 * This file intentionally does not depend on the Windows SDK yet.  The small
 * ABI declarations below are temporary until the MinGW-w64 SDK payload is
 * added to Project Sources.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

#if defined(_WIN32)
# define SPW_EXPORT __declspec(dllexport)
# define SPW_IMPORT __declspec(dllimport)
# define WINAPI __stdcall
#else
# define SPW_EXPORT
# define SPW_IMPORT
# define WINAPI
#endif

/* Compiler support kept local so the DLL remains CRT-free even when
 * large Win32 structs are zero-initialized. */
void *memset(void *destination, int value, size_t count) {
    volatile uint8_t *bytes = (volatile uint8_t *)destination;
    size_t i;
    for (i = 0u; i < count; ++i) bytes[i] = (uint8_t)value;
    return destination;
}

/* Minimal Win32 ABI ------------------------------------------------------ */
typedef void *HANDLE;
typedef void *HINSTANCE;
typedef void *HMODULE;
typedef void *HWND;
typedef void *HMENU;
typedef void *HICON;
typedef void *HCURSOR;
typedef void *HBRUSH;
typedef void *HGDIOBJ;
typedef void *HBITMAP;
typedef void *HIMAGELIST;
typedef void *HDWP;
typedef void *HDC;
typedef void *HMONITOR;
typedef unsigned short WCHAR;
typedef uint32_t UINT;
typedef uint32_t DWORD;
typedef int32_t BOOL;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
typedef intptr_t LONG_PTR;
typedef uint16_t ATOM;
typedef uint16_t WORD;
typedef int16_t SHORT;
typedef uint8_t BYTE;
typedef DWORD COLORREF;

typedef struct { int32_t x; int32_t y; } POINT;
typedef struct {
    UINT cbSize;
    DWORD dwMask;
    DWORD dwEffects;
    int32_t yHeight;
    int32_t yOffset;
    COLORREF crTextColor;
    BYTE bCharSet;
    BYTE bPitchAndFamily;
    WCHAR szFaceName[32];
} CHARFORMATW;
typedef struct {
    UINT cbSize;
    DWORD dwMask;
    DWORD dwEffects;
    int32_t yHeight;
    int32_t yOffset;
    COLORREF crTextColor;
    BYTE bCharSet;
    BYTE bPitchAndFamily;
    WCHAR szFaceName[32];
    WORD wWeight;
    SHORT sSpacing;
    COLORREF crBackColor;
    DWORD lcid;
    DWORD dwReserved;
    SHORT sStyle;
    WORD wKerning;
    BYTE bUnderlineType;
    BYTE bAnimation;
    BYTE bRevAuthor;
    BYTE bUnderlineColor;
} CHARFORMAT2W;
typedef struct { int32_t left; int32_t top; int32_t right; int32_t bottom; } RECT;
typedef struct {
    DWORD cbSize;
    RECT rcMonitor;
    RECT rcWork;
    DWORD dwFlags;
} MONITORINFO;
typedef struct { int32_t cx; int32_t cy; } SIZE;
typedef struct {
    int32_t tmHeight;
    int32_t tmAscent;
    int32_t tmDescent;
    int32_t tmInternalLeading;
    int32_t tmExternalLeading;
    int32_t tmAveCharWidth;
    int32_t tmMaxCharWidth;
    int32_t tmWeight;
    int32_t tmOverhang;
    int32_t tmDigitizedAspectX;
    int32_t tmDigitizedAspectY;
    WCHAR tmFirstChar;
    WCHAR tmLastChar;
    WCHAR tmDefaultChar;
    WCHAR tmBreakChar;
    BYTE tmItalic;
    BYTE tmUnderlined;
    BYTE tmStruckOut;
    BYTE tmPitchAndFamily;
    BYTE tmCharSet;
} TEXTMETRICW;
typedef struct {
    UINT cbSize;
    UINT fMask;
    int32_t nMin;
    int32_t nMax;
    UINT nPage;
    int32_t nPos;
    int32_t nTrackPos;
} SCROLLINFO;
typedef struct {
    DWORD biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;
typedef struct { uint8_t rgbBlue, rgbGreen, rgbRed, rgbReserved; } RGBQUAD;
typedef struct { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[1]; } BITMAPINFO;
typedef struct { uint8_t BlendOp, BlendFlags, SourceConstantAlpha, AlphaFormat; } BLENDFUNCTION;
typedef struct {
    HIMAGELIST himl;
    RECT margin;
    UINT uAlign;
} BUTTON_IMAGELIST;
typedef struct {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
    DWORD lPrivate;
} MSG;

typedef LRESULT (WINAPI *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef LRESULT (WINAPI *SUBCLASSPROC)(HWND, UINT, WPARAM, LPARAM, uintptr_t, uintptr_t);
typedef DWORD (WINAPI *LPTHREAD_START_ROUTINE)(void *);

typedef struct {
    UINT cbSize;
    UINT style;
    WNDPROC lpfnWndProc;
    int32_t cbClsExtra;
    int32_t cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    const WCHAR *lpszMenuName;
    const WCHAR *lpszClassName;
    HICON hIconSm;
} WNDCLASSEXW;
typedef struct {
    DWORD dwSize;
    DWORD dwICC;
} INITCOMMONCONTROLSEX;
typedef struct {
    DWORD cbSize;
    DWORD dwFlags;
    HWND hwndTrack;
    DWORD dwHoverTime;
} TRACKMOUSEEVENT;
typedef struct {
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    uintptr_t uId;
    RECT rect;
    HINSTANCE hinst;
    WCHAR *lpszText;
    LPARAM lParam;
    void *lpReserved;
} TOOLINFOW;
typedef struct {
    HWND hwndFrom;
    uintptr_t idFrom;
    UINT code;
} NMHDR;
typedef struct {
    UINT mask;
    DWORD dwState;
    DWORD dwStateMask;
    WCHAR *pszText;
    int32_t cchTextMax;
    int32_t iImage;
    LPARAM lParam;
} TCITEMW;
typedef struct {
    UINT mask;
    int32_t fmt;
    int32_t cx;
    WCHAR *pszText;
    int32_t cchTextMax;
    int32_t iSubItem;
} LVCOLUMNW;
typedef struct {
    UINT mask;
    int32_t iItem;
    int32_t iSubItem;
    UINT state;
    UINT stateMask;
    WCHAR *pszText;
    int32_t cchTextMax;
    int32_t iImage;
    LPARAM lParam;
} LVITEMW;
typedef struct {
    NMHDR hdr;
    int32_t iItem;
    int32_t iSubItem;
    UINT uNewState;
    UINT uOldState;
    UINT uChanged;
    POINT ptAction;
    LPARAM lParam;
} NMLISTVIEW;
typedef struct {
    NMHDR hdr;
    int32_t iItem;
    int32_t iSubItem;
    UINT uNewState;
    UINT uOldState;
    UINT uChanged;
    POINT ptAction;
    LPARAM lParam;
    UINT uKeyFlags;
} NMITEMACTIVATE;
typedef void *HTREEITEM;
typedef struct {
    UINT mask;
    HTREEITEM hItem;
    UINT state;
    UINT stateMask;
    WCHAR *pszText;
    int32_t cchTextMax;
    int32_t iImage;
    int32_t iSelectedImage;
    int32_t cChildren;
    LPARAM lParam;
} TVITEMW;
typedef struct {
    HTREEITEM hParent;
    HTREEITEM hInsertAfter;
    TVITEMW item;
} TVINSERTSTRUCTW;
typedef struct {
    NMHDR hdr;
    UINT action;
    TVITEMW itemOld;
    TVITEMW itemNew;
    POINT ptDrag;
} NMTREEVIEWW;
typedef struct {
    DWORD lStructSize;
    HWND hwndOwner;
    HINSTANCE hInstance;
    const WCHAR *lpstrFilter;
    WCHAR *lpstrCustomFilter;
    DWORD nMaxCustFilter;
    DWORD nFilterIndex;
    WCHAR *lpstrFile;
    DWORD nMaxFile;
    WCHAR *lpstrFileTitle;
    DWORD nMaxFileTitle;
    const WCHAR *lpstrInitialDir;
    const WCHAR *lpstrTitle;
    DWORD Flags;
    WORD nFileOffset;
    WORD nFileExtension;
    const WCHAR *lpstrDefExt;
    LPARAM lCustData;
    void *lpfnHook;
    const WCHAR *lpTemplateName;
    void *pvReserved;
    DWORD dwReserved;
    DWORD FlagsEx;
} OPENFILENAMEW;
typedef int32_t (WINAPI *BFFCALLBACK)(HWND, UINT, LPARAM, LPARAM);
typedef struct {
    HWND hwndOwner;
    const void *pidlRoot;
    WCHAR *pszDisplayName;
    const WCHAR *lpszTitle;
    UINT ulFlags;
    BFFCALLBACK lpfn;
    LPARAM lParam;
    int32_t iImage;
} BROWSEINFOW;

_Static_assert(sizeof(void *) == 8, "Spec Win32 runtime requires Win64");
_Static_assert(sizeof(MSG) == 48, "Unexpected Win64 MSG layout");
_Static_assert(sizeof(CHARFORMATW) == 92, "Unexpected Win64 CHARFORMATW layout");
_Static_assert(sizeof(CHARFORMAT2W) == 116, "Unexpected Win64 CHARFORMAT2W layout");
_Static_assert(sizeof(WNDCLASSEXW) == 80, "Unexpected Win64 WNDCLASSEXW layout");
_Static_assert(sizeof(INITCOMMONCONTROLSEX) == 8, "Unexpected INITCOMMONCONTROLSEX layout");
_Static_assert(sizeof(TRACKMOUSEEVENT) == 24, "Unexpected Win64 TRACKMOUSEEVENT layout");
_Static_assert(sizeof(MONITORINFO) == 40, "Unexpected Win64 MONITORINFO layout");
_Static_assert(sizeof(TOOLINFOW) == 72, "Unexpected Win64 TOOLINFOW layout");
_Static_assert(sizeof(NMHDR) == 24, "Unexpected Win64 NMHDR layout");
_Static_assert(sizeof(BUTTON_IMAGELIST) == 32, "Unexpected Win64 BUTTON_IMAGELIST layout");
_Static_assert(sizeof(TCITEMW) == 40, "Unexpected Win64 TCITEMW layout");
_Static_assert(sizeof(LVCOLUMNW) == 32, "Unexpected Win64 LVCOLUMNW prefix layout");
_Static_assert(offsetof(LVITEMW, pszText) == 24, "Unexpected Win64 LVITEMW text offset");
_Static_assert(offsetof(NMLISTVIEW, lParam) == 56, "Unexpected Win64 NMLISTVIEW layout");
_Static_assert(sizeof(OPENFILENAMEW) == 152, "Unexpected Win64 OPENFILENAMEW layout");
_Static_assert(sizeof(BROWSEINFOW) == 64, "Unexpected Win64 BROWSEINFOW layout");

SPW_IMPORT HANDLE WINAPI CreateThread(void *, size_t, LPTHREAD_START_ROUTINE, void *, DWORD, DWORD *);
SPW_IMPORT DWORD WINAPI WaitForSingleObject(HANDLE, DWORD);
SPW_IMPORT HANDLE WINAPI CreateEventW(void *, BOOL, BOOL, const WCHAR *);
SPW_IMPORT BOOL WINAPI SetEvent(HANDLE);
SPW_IMPORT BOOL WINAPI CloseHandle(HANDLE);
SPW_IMPORT DWORD WINAPI GetLastError(void);
SPW_IMPORT HMODULE WINAPI GetModuleHandleW(const WCHAR *);
SPW_IMPORT HMODULE WINAPI LoadLibraryW(const WCHAR *);
SPW_IMPORT int WINAPI MultiByteToWideChar(UINT, DWORD, const char *, int, WCHAR *, int);
SPW_IMPORT int WINAPI WideCharToMultiByte(UINT, DWORD, const WCHAR *, int, char *, int, const char *, BOOL *);
SPW_IMPORT BOOL WINAPI GetOpenFileNameW(OPENFILENAMEW *);
SPW_IMPORT BOOL WINAPI GetSaveFileNameW(OPENFILENAMEW *);
SPW_IMPORT DWORD WINAPI CommDlgExtendedError(void);
SPW_IMPORT void * WINAPI SHBrowseForFolderW(BROWSEINFOW *);
SPW_IMPORT BOOL WINAPI SHGetPathFromIDListW(const void *, WCHAR *);
SPW_IMPORT void WINAPI CoTaskMemFree(void *);

SPW_IMPORT ATOM WINAPI RegisterClassExW(const WNDCLASSEXW *);
SPW_IMPORT HWND WINAPI CreateWindowExW(DWORD, const WCHAR *, const WCHAR *, DWORD,
                                       int32_t, int32_t, int32_t, int32_t,
                                       HWND, HMENU, HINSTANCE, void *);
SPW_IMPORT LRESULT WINAPI DefWindowProcW(HWND, UINT, WPARAM, LPARAM);
SPW_IMPORT BOOL WINAPI GetMessageW(MSG *, HWND, UINT, UINT);
SPW_IMPORT BOOL WINAPI TranslateMessage(const MSG *);
SPW_IMPORT LRESULT WINAPI DispatchMessageW(const MSG *);
SPW_IMPORT BOOL WINAPI PostMessageW(HWND, UINT, WPARAM, LPARAM);
SPW_IMPORT void WINAPI PostQuitMessage(int32_t);
SPW_IMPORT BOOL WINAPI DestroyWindow(HWND);
SPW_IMPORT BOOL WINAPI ShowWindow(HWND, int32_t);
SPW_IMPORT BOOL WINAPI UpdateWindow(HWND);
SPW_IMPORT BOOL WINAPI SetWindowTextW(HWND, const WCHAR *);
SPW_IMPORT int WINAPI GetWindowTextLengthW(HWND);
SPW_IMPORT int WINAPI GetWindowTextW(HWND, WCHAR *, int);
SPW_IMPORT BOOL WINAPI MoveWindow(HWND, int32_t, int32_t, int32_t, int32_t, BOOL);
SPW_IMPORT BOOL WINAPI SetWindowPos(HWND, HWND, int32_t, int32_t, int32_t, int32_t, UINT);
SPW_IMPORT int WINAPI SetScrollInfo(HWND, int, const void *, BOOL);
SPW_IMPORT BOOL WINAPI GetScrollInfo(HWND, int, void *);
SPW_IMPORT BOOL WINAPI ShowScrollBar(HWND, int, BOOL);
SPW_IMPORT BOOL WINAPI IsWindowVisible(HWND);
SPW_IMPORT BOOL WINAPI SetForegroundWindow(HWND);
SPW_IMPORT BOOL WINAPI GetWindowRect(HWND, RECT *);
SPW_IMPORT BOOL WINAPI ScreenToClient(HWND, POINT *);
SPW_IMPORT BOOL WINAPI GetCursorPos(POINT *);
SPW_IMPORT HWND WINAPI GetForegroundWindow(void);
SPW_IMPORT BOOL WINAPI IsIconic(HWND);
SPW_IMPORT BOOL WINAPI IsZoomed(HWND);
SPW_IMPORT HMONITOR WINAPI MonitorFromWindow(HWND, DWORD);
SPW_IMPORT BOOL WINAPI GetMonitorInfoW(HMONITOR, MONITORINFO *);
SPW_IMPORT HBRUSH WINAPI GetSysColorBrush(int32_t);
SPW_IMPORT HMENU WINAPI CreatePopupMenu(void);
SPW_IMPORT HMENU WINAPI CreateMenu(void);
SPW_IMPORT BOOL WINAPI DestroyMenu(HMENU);
SPW_IMPORT BOOL WINAPI SetMenu(HWND, HMENU);
SPW_IMPORT BOOL WINAPI DrawMenuBar(HWND);
SPW_IMPORT BOOL WINAPI AppendMenuW(HMENU, UINT, uintptr_t, const WCHAR *);
SPW_IMPORT BOOL WINAPI SetMenuItemBitmaps(HMENU, UINT, UINT, HBITMAP, HBITMAP);
SPW_IMPORT UINT WINAPI TrackPopupMenu(HMENU, UINT, int32_t, int32_t, int32_t, HWND, const RECT *);
SPW_IMPORT HCURSOR WINAPI LoadCursorW(HINSTANCE, const WCHAR *);
SPW_IMPORT HCURSOR WINAPI SetCursor(HCURSOR);
SPW_IMPORT HCURSOR WINAPI GetCursor(void);
SPW_IMPORT int32_t WINAPI MessageBoxW(HWND, const WCHAR *, const WCHAR *, UINT);
SPW_IMPORT BOOL WINAPI MessageBeep(UINT);
SPW_IMPORT BOOL WINAPI EnableWindow(HWND, BOOL);
SPW_IMPORT BOOL WINAPI IsWindowEnabled(HWND);
SPW_IMPORT HWND WINAPI SetFocus(HWND);
SPW_IMPORT HWND WINAPI GetFocus(void);
SPW_IMPORT HWND WINAPI SetCapture(HWND);
SPW_IMPORT BOOL WINAPI ReleaseCapture(void);
SPW_IMPORT SHORT WINAPI GetKeyState(int32_t);
SPW_IMPORT BOOL WINAPI TrackMouseEvent(TRACKMOUSEEVENT *);
SPW_IMPORT BOOL WINAPI IsDialogMessageW(HWND, MSG *);
SPW_IMPORT BOOL WINAPI GetClientRect(HWND, RECT *);
SPW_IMPORT HDWP WINAPI BeginDeferWindowPos(int32_t);
SPW_IMPORT HDWP WINAPI DeferWindowPos(HDWP, HWND, HWND, int32_t, int32_t, int32_t, int32_t, UINT);
SPW_IMPORT BOOL WINAPI EndDeferWindowPos(HDWP);
SPW_IMPORT LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
SPW_IMPORT HWND WINAPI GetParent(HWND);
SPW_IMPORT HGDIOBJ WINAPI GetStockObject(int32_t);
SPW_IMPORT HGDIOBJ WINAPI SelectObject(HDC, HGDIOBJ);
SPW_IMPORT BOOL WINAPI GetTextMetricsW(HDC, TEXTMETRICW *);
SPW_IMPORT BOOL WINAPI GetTextExtentPoint32W(HDC, const WCHAR *, int32_t, SIZE *);
SPW_IMPORT BOOL WINAPI RoundRect(HDC, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
SPW_IMPORT BOOL WINAPI MoveToEx(HDC, int32_t, int32_t, POINT *);
SPW_IMPORT BOOL WINAPI LineTo(HDC, int32_t, int32_t);
SPW_IMPORT HDC WINAPI CreateCompatibleDC(HDC);
SPW_IMPORT BOOL WINAPI DeleteDC(HDC);
SPW_IMPORT HBITMAP WINAPI CreateDIBSection(HDC, const BITMAPINFO *, UINT, void **, HANDLE, DWORD);
SPW_IMPORT BOOL WINAPI DeleteObject(HGDIOBJ);
SPW_IMPORT COLORREF WINAPI SetTextColor(HDC, COLORREF);
SPW_IMPORT int WINAPI SetBkMode(HDC, int);
SPW_IMPORT BOOL WINAPI AlphaBlend(HDC, int32_t, int32_t, int32_t, int32_t,
                                  HDC, int32_t, int32_t, int32_t, int32_t, BLENDFUNCTION);
SPW_IMPORT HDC WINAPI GetDC(HWND);
SPW_IMPORT int32_t WINAPI ReleaseDC(HWND, HDC);
SPW_IMPORT BOOL WINAPI InvalidateRect(HWND, const RECT *, BOOL);
SPW_IMPORT int WINAPI FillRect(HDC, const RECT *, HBRUSH);
SPW_IMPORT int WINAPI DrawTextW(HDC, const WCHAR *, int32_t, RECT *, UINT);
SPW_IMPORT uintptr_t WINAPI SetTimer(HWND, uintptr_t, UINT, void *);
SPW_IMPORT BOOL WINAPI KillTimer(HWND, uintptr_t);
SPW_IMPORT LONG_PTR WINAPI GetWindowLongPtrW(HWND, int32_t);
SPW_IMPORT LONG_PTR WINAPI SetWindowLongPtrW(HWND, int32_t, LONG_PTR);
SPW_IMPORT BOOL WINAPI InitCommonControlsEx(const INITCOMMONCONTROLSEX *);
SPW_IMPORT BOOL WINAPI SetWindowSubclass(HWND, SUBCLASSPROC, uintptr_t, uintptr_t);
SPW_IMPORT BOOL WINAPI RemoveWindowSubclass(HWND, SUBCLASSPROC, uintptr_t);
SPW_IMPORT LRESULT WINAPI DefSubclassProc(HWND, UINT, WPARAM, LPARAM);
SPW_IMPORT HIMAGELIST WINAPI ImageList_Create(int32_t, int32_t, UINT, int32_t, int32_t);
SPW_IMPORT int32_t WINAPI ImageList_Add(HIMAGELIST, HBITMAP, HBITMAP);
SPW_IMPORT BOOL WINAPI ImageList_Destroy(HIMAGELIST);

#define FALSE_VALUE 0
#define TRUE_VALUE 1
#define INFINITE_VALUE 0xffffffffu
#define WAIT_OBJECT_0_VALUE 0u
#define WAIT_TIMEOUT_VALUE 258u

#define CP_UTF8_VALUE 65001u
#define ERROR_CLASS_ALREADY_EXISTS 1410u

#define WM_NULL_VALUE 0x0000u
#define WM_USER_VALUE 0x0400u
#define WM_PAINT_VALUE 0x000fu
#define WM_ERASEBKGND_VALUE 0x0014u
#define WM_CTLCOLORBTN_VALUE 0x0135u
#define WM_SIZE_VALUE 0x0005u
#define WM_ACTIVATE_VALUE 0x0006u
#define WM_SETFOCUS_VALUE 0x0007u
#define WM_KILLFOCUS_VALUE 0x0008u
#define WM_SETFONT_VALUE 0x0030u
#define WM_NOTIFY_VALUE 0x004eu
#define WM_COMMAND_VALUE 0x0111u
#define WM_TIMER_VALUE 0x0113u
#define WM_GETFONT_VALUE 0x0031u
#define WM_HSCROLL_VALUE 0x0114u
#define WM_VSCROLL_VALUE 0x0115u
#define SB_HORZ_VALUE 0
#define SB_VERT_VALUE 1
#define SB_LINEUP_VALUE 0u
#define SB_LINEDOWN_VALUE 1u
#define SB_PAGEUP_VALUE 2u
#define SB_PAGEDOWN_VALUE 3u
#define SB_THUMBPOSITION_VALUE 4u
#define SB_THUMBTRACK_VALUE 5u
#define SB_TOP_VALUE 6u
#define SB_BOTTOM_VALUE 7u
#define SIF_RANGE_VALUE 0x0001u
#define SIF_PAGE_VALUE 0x0002u
#define SIF_POS_VALUE 0x0004u
#define SIF_TRACKPOS_VALUE 0x0010u
#define SIF_ALL_VALUE (SIF_RANGE_VALUE | SIF_PAGE_VALUE | SIF_POS_VALUE | SIF_TRACKPOS_VALUE)
#define WM_USER_VALUE 0x0400u
#define WM_CLOSE_VALUE 0x0010u
#define WM_SETCURSOR_VALUE 0x0020u
#define WM_NCDESTROY_VALUE 0x0082u
#define WA_INACTIVE_VALUE 0u
#define WM_APP_VALUE 0x8000u
#define SPW_WM_COMMAND (WM_APP_VALUE + 0x231u)

#define MONITOR_DEFAULTTONEAREST_VALUE 0x00000002u
#define GWLP_HWNDPARENT_VALUE (-8)
#define WS_OVERLAPPEDWINDOW_VALUE 0x00cf0000u
#define WS_EX_TOOLWINDOW_VALUE 0x00000080u
#define WS_POPUP_VALUE 0x80000000u
#define WS_CHILD_VALUE 0x40000000u
#define WS_VISIBLE_VALUE 0x10000000u
#define WS_TABSTOP_VALUE 0x00010000u
#define WS_BORDER_VALUE 0x00800000u
#define BS_PUSHBUTTON_VALUE 0x00000000u
#define BS_GROUPBOX_VALUE 0x00000007u
#define BS_AUTOCHECKBOX_VALUE 0x00000003u
#define BS_PUSHLIKE_VALUE 0x00001000u
#define BS_RADIOBUTTON_VALUE 0x00000004u
#define SS_LEFT_VALUE 0x00000000u
#define SS_NOTIFY_VALUE 0x00000100u
#define ES_MULTILINE_VALUE 0x00000004u
#define ES_AUTOVSCROLL_VALUE 0x00000040u
#define ES_AUTOHSCROLL_VALUE 0x00000080u
#define ES_WANTRETURN_VALUE 0x00001000u
#define CBS_DROPDOWNLIST_VALUE 0x00000003u
#define CBS_HASSTRINGS_VALUE 0x00000200u
#define TBS_HORZ_VALUE 0x0000u
#define TBS_AUTOTICKS_VALUE 0x0001u
#define TBS_VERT_VALUE 0x0002u
#define PBS_SMOOTH_VALUE 0x0001u
#define PBS_MARQUEE_VALUE 0x0008u
#define WS_CLIPSIBLINGS_VALUE 0x04000000u
#define WS_VSCROLL_VALUE 0x00200000u
#define WS_HSCROLL_VALUE 0x00100000u
#define WS_CLIPCHILDREN_VALUE 0x02000000u
#define WS_CLIPSIBLINGS_VALUE 0x04000000u
#define BN_CLICKED_VALUE 0u
#define EN_CHANGE_VALUE 0x0300u
#define EN_VSCROLL_VALUE 0x0602u
#define CBN_SELCHANGE_VALUE 1u
#define CB_ADDSTRING_VALUE 0x0143u
#define CB_GETCURSEL_VALUE 0x0147u
#define CB_RESETCONTENT_VALUE 0x014bu
#define CB_SETCURSEL_VALUE 0x014eu
#define CB_ERR_VALUE ((LRESULT)-1)
#define CB_ERRSPACE_VALUE ((LRESULT)-2)
#define TBM_GETPOS_VALUE (WM_USER_VALUE + 0u)
#define TBM_SETPOS_VALUE (WM_USER_VALUE + 5u)
#define TBM_SETRANGEMIN_VALUE (WM_USER_VALUE + 7u)
#define TBM_SETRANGEMAX_VALUE (WM_USER_VALUE + 8u)
#define TBM_SETTICFREQ_VALUE (WM_USER_VALUE + 20u)
#define PBM_SETPOS_VALUE (WM_USER_VALUE + 2u)
#define PBM_SETRANGE32_VALUE (WM_USER_VALUE + 6u)
#define PBM_SETMARQUEE_VALUE (WM_USER_VALUE + 10u)
#define ICC_BAR_CLASSES_VALUE 0x00000004u
#define ICC_TAB_CLASSES_VALUE 0x00000008u
#define ICC_PROGRESS_CLASS_VALUE 0x00000020u
#define ICC_LINK_CLASS_VALUE 0x00008000u
#define TCIF_TEXT_VALUE 0x0001u
#define TCM_FIRST_VALUE 0x1300u
#define TCM_DELETEALLITEMS_VALUE (TCM_FIRST_VALUE + 9u)
#define TCM_GETCURSEL_VALUE (TCM_FIRST_VALUE + 11u)
#define TCM_SETCURSEL_VALUE (TCM_FIRST_VALUE + 12u)
#define TCM_ADJUSTRECT_VALUE (TCM_FIRST_VALUE + 40u)
#define TCM_INSERTITEMW_VALUE (TCM_FIRST_VALUE + 62u)
#define TCN_FIRST_VALUE (-550)
#define TCN_SELCHANGE_VALUE (TCN_FIRST_VALUE - 1)
#define ICC_LISTVIEW_CLASSES_VALUE 0x00000001u
#define ICC_TREEVIEW_CLASSES_VALUE 0x00000002u
#define LVS_REPORT_VALUE 0x0001u
#define LVS_SINGLESEL_VALUE 0x0004u
#define LVS_SHOWSELALWAYS_VALUE 0x0008u
#define LVS_NOCOLUMNHEADER_VALUE 0x4000u
#define LVS_EX_SUBITEMIMAGES_VALUE 0x00000002u
#define LVS_EX_FULLROWSELECT_VALUE 0x00000020u
#define LVS_EX_DOUBLEBUFFER_VALUE 0x00010000u
#define LVCF_FMT_VALUE 0x0001u
#define LVCF_WIDTH_VALUE 0x0002u
#define LVCF_TEXT_VALUE 0x0004u
#define LVCF_SUBITEM_VALUE 0x0008u
#define LVCFMT_LEFT_VALUE 0x0000u
#define LVCFMT_RIGHT_VALUE 0x0001u
#define LVCFMT_CENTER_VALUE 0x0002u
#define LVIF_TEXT_VALUE 0x0001u
#define LVIF_IMAGE_VALUE 0x0002u
#define LVIF_STATE_VALUE 0x0008u
#define LVIS_SELECTED_VALUE 0x0002u
#define LVNI_SELECTED_VALUE 0x0002u
#define LVM_FIRST_VALUE 0x1000u
#define LVM_SETIMAGELIST_VALUE (LVM_FIRST_VALUE + 3u)
#define LVM_SETITEMSTATE_VALUE (LVM_FIRST_VALUE + 43u)
#define LVM_INSERTITEMW_VALUE (LVM_FIRST_VALUE + 77u)
#define LVM_DELETEALLITEMS_VALUE (LVM_FIRST_VALUE + 9u)
#define LVM_GETNEXTITEM_VALUE (LVM_FIRST_VALUE + 12u)
#define LVM_ENSUREVISIBLE_VALUE (LVM_FIRST_VALUE + 19u)
#define LVM_INSERTCOLUMNW_VALUE (LVM_FIRST_VALUE + 97u)
#define LVM_SETCOLUMNW_VALUE (LVM_FIRST_VALUE + 96u)
#define LVM_SETCOLUMNWIDTH_VALUE (LVM_FIRST_VALUE + 30u)
#define LVM_DELETECOLUMN_VALUE (LVM_FIRST_VALUE + 28u)
#define LVM_GETHEADER_VALUE (LVM_FIRST_VALUE + 31u)
#define LVM_GETCOLUMNWIDTH_VALUE (LVM_FIRST_VALUE + 29u)
#define LVM_GETITEMRECT_VALUE (LVM_FIRST_VALUE + 14u)
#define LVM_GETTOPINDEX_VALUE (LVM_FIRST_VALUE + 39u)
#define LVIR_BOUNDS_VALUE 0u
#define LVM_SETITEMW_VALUE (LVM_FIRST_VALUE + 76u)
#define LVM_SETEXTENDEDLISTVIEWSTYLE_VALUE (LVM_FIRST_VALUE + 54u)
#define LVSIL_SMALL_VALUE 1u
#define LVN_FIRST_VALUE (-100)
#define LVN_ITEMCHANGED_VALUE (LVN_FIRST_VALUE - 1)
#define LVN_COLUMNCLICK_VALUE (LVN_FIRST_VALUE - 8)
#define NM_FIRST_VALUE 0
#define NM_CLICK_VALUE (NM_FIRST_VALUE - 2)
#define NM_DBLCLK_VALUE (NM_FIRST_VALUE - 3)
#define NM_RETURN_VALUE (NM_FIRST_VALUE - 4)
#define HDS_NOSIZING_VALUE 0x0800u
#define TVS_HASBUTTONS_VALUE 0x0001u
#define TVS_HASLINES_VALUE 0x0002u
#define TVS_LINESATROOT_VALUE 0x0004u
#define TVS_SHOWSELALWAYS_VALUE 0x0020u
#define TVS_NOSCROLL_VALUE 0x2000u
#define TVIF_TEXT_VALUE 0x0001u
#define TVIF_IMAGE_VALUE 0x0002u
#define TVIF_PARAM_VALUE 0x0004u
#define TVIF_SELECTEDIMAGE_VALUE 0x0020u
#define TVIF_CHILDREN_VALUE 0x0040u
#define TVE_COLLAPSE_VALUE 0x0001u
#define TVE_EXPAND_VALUE 0x0002u
#define TVIS_EXPANDED_VALUE 0x0020u
#define TVGN_ROOT_VALUE 0x0000u
#define TVGN_NEXT_VALUE 0x0001u
#define TVGN_CHILD_VALUE 0x0004u
#define TVGN_FIRSTVISIBLE_VALUE 0x0005u
#define TVGN_NEXTVISIBLE_VALUE 0x0006u
#define TVGN_CARET_VALUE 0x0009u
#define TV_FIRST_VALUE 0x1100u
#define TVM_INSERTITEMW_VALUE (TV_FIRST_VALUE + 50u)
#define TVM_DELETEITEM_VALUE (TV_FIRST_VALUE + 1u)
#define TVM_EXPAND_VALUE (TV_FIRST_VALUE + 2u)
#define TVM_SETIMAGELIST_VALUE (TV_FIRST_VALUE + 9u)
#define TVM_GETITEMW_VALUE (TV_FIRST_VALUE + 62u)
#define TVM_GETNEXTITEM_VALUE (TV_FIRST_VALUE + 10u)
#define TVM_SELECTITEM_VALUE (TV_FIRST_VALUE + 11u)
#define TVM_ENSUREVISIBLE_VALUE (TV_FIRST_VALUE + 20u)
#define TVM_SETITEMHEIGHT_VALUE (TV_FIRST_VALUE + 27u)
#define TVI_ROOT_VALUE ((HTREEITEM)(intptr_t)-0x10000)
#define TVI_LAST_VALUE ((HTREEITEM)(intptr_t)-0x0fffe)
#define TVSIL_NORMAL_VALUE 0u
#define I_IMAGENONE_VALUE (-2)
#define TVN_FIRST_VALUE (-400)
#define TVN_SELCHANGEDW_VALUE (TVN_FIRST_VALUE - 51)
#define TVN_ITEMEXPANDEDW_VALUE (TVN_FIRST_VALUE - 55)
#define WM_KEYDOWN_VALUE 0x0100u
#define WM_KEYUP_VALUE 0x0101u
#define WM_CHAR_VALUE 0x0102u
#define WM_SYSKEYDOWN_VALUE 0x0104u
#define WM_SYSKEYUP_VALUE 0x0105u
#define WM_MOUSEMOVE_VALUE 0x0200u
#define WM_LBUTTONDOWN_VALUE 0x0201u
#define WM_MOUSEWHEEL_VALUE 0x020au
#define WM_LBUTTONUP_VALUE 0x0202u
#define WM_LBUTTONDBLCLK_VALUE 0x0203u
#define WM_RBUTTONDOWN_VALUE 0x0204u
#define WM_RBUTTONUP_VALUE 0x0205u
#define WM_RBUTTONDBLCLK_VALUE 0x0206u
#define WM_MBUTTONDOWN_VALUE 0x0207u
#define WM_MBUTTONUP_VALUE 0x0208u
#define WM_MBUTTONDBLCLK_VALUE 0x0209u
#define WM_MOUSELEAVE_VALUE 0x02a3u
#define WM_CONTEXTMENU_VALUE 0x007bu
#define WM_CUT_VALUE 0x0300u
#define WM_COPY_VALUE 0x0301u
#define WM_PASTE_VALUE 0x0302u
#define VK_RETURN_VALUE 0x0du
#define VK_UP_VALUE 0x26u
#define VK_DOWN_VALUE 0x28u
#define VK_SHIFT_VALUE 0x10u
#define VK_CONTROL_VALUE 0x11u
#define VK_MENU_VALUE 0x12u
#define VK_LWIN_VALUE 0x5bu
#define VK_RWIN_VALUE 0x5cu
#define TME_LEAVE_VALUE 0x00000002u
#define TTS_ALWAYSTIP_VALUE 0x01u
#define TTS_NOPREFIX_VALUE 0x02u
#define TTF_IDISHWND_VALUE 0x0001u
#define TTF_SUBCLASS_VALUE 0x0010u
#define TTM_ADDTOOLW_VALUE (WM_USER_VALUE + 50u)
#define MF_GRAYED_VALUE 0x00000001u
#define MF_CHECKED_VALUE 0x00000008u
#define MF_SEPARATOR_VALUE 0x00000800u
#define MF_STRING_VALUE 0x00000000u
#define MF_POPUP_VALUE 0x00000010u
#define MF_BYPOSITION_VALUE 0x00000400u
#define TPM_RIGHTBUTTON_VALUE 0x0002u
#define TPM_RETURNCMD_VALUE 0x0100u
#define SPW_MENU_SEPARATOR 0x0001u
#define SPW_MENU_ENABLED 0x0002u
#define SPW_MENU_CHECKED 0x0004u
#define SPW_MENU_SUBMENU 0x0008u
#define SPW_NORMALIZED_MAX 1000000
#define SPW_MAX_TABLE_COLUMNS 64u
#define SPW_MAX_MENU_ITEMS 256u
#define BM_GETCHECK_VALUE 0x00f0u
#define BM_SETCHECK_VALUE 0x00f1u
#define BST_UNCHECKED_VALUE 0u
#define BST_CHECKED_VALUE 1u
#define EM_GETSEL_VALUE 0x00b0u
#define EM_GETLINECOUNT_VALUE 0x00bau
#define EM_LINEINDEX_VALUE 0x00bbu
#define EM_SETSEL_VALUE 0x00b1u
#define EM_GETRECT_VALUE 0x00b2u
#define EM_SETRECT_VALUE 0x00b3u
#define EM_SETRECTNP_VALUE 0x00b4u
#define EM_LINESCROLL_VALUE 0x00b6u
#define EM_REPLACESEL_VALUE 0x00c2u
#define EM_SETLIMITTEXT_VALUE 0x00c5u
#define EM_UNDO_VALUE 0x00c7u
#define EM_EMPTYUNDOBUFFER_VALUE 0x00cdu
#define EM_GETFIRSTVISIBLELINE_VALUE 0x00ceu
#define EM_POSFROMCHAR_RICH_VALUE (WM_USER_VALUE + 38u)
#define EM_GETEVENTMASK_VALUE (WM_USER_VALUE + 59u)
#define EM_SETBKGNDCOLOR_VALUE (WM_USER_VALUE + 67u)
#define EM_SETCHARFORMAT_VALUE (WM_USER_VALUE + 68u)
#define EM_SETEVENTMASK_VALUE (WM_USER_VALUE + 69u)
#define ENM_CHANGE_VALUE 0x00000001u
#define ENM_SCROLL_VALUE 0x00000004u
#define EM_SETTARGETDEVICE_VALUE (WM_USER_VALUE + 72u)
#define EM_SETPASSWORDCHAR_VALUE 0x00ccu
#define EM_SETREADONLY_VALUE 0x00cfu
#define EM_SETMARGINS_VALUE 0x00d3u
#define EC_LEFTMARGIN_VALUE 0x0001u
#define EC_RIGHTMARGIN_VALUE 0x0002u
#define EM_SETCUEBANNER_VALUE 0x1501u
#define SWP_NOSIZE_VALUE 0x0001u
#define SWP_NOMOVE_VALUE 0x0002u
#define SWP_NOZORDER_VALUE 0x0004u
#define SWP_NOACTIVATE_VALUE 0x0010u
#define SWP_FRAMECHANGED_VALUE 0x0020u
#define SWP_NOCOPYBITS_VALUE 0x0100u
#define CW_USEDEFAULT_VALUE ((int32_t)0x80000000u)
#define COLOR_WINDOW_BRUSH ((HBRUSH)(uintptr_t)6u)
#define COLOR_BTNFACE_BRUSH ((HBRUSH)(uintptr_t)16u)
#define IDC_ARROW_VALUE ((const WCHAR *)(uintptr_t)32512u)
#define IDC_WAIT_VALUE ((const WCHAR *)(uintptr_t)32514u)
#define DEFAULT_GUI_FONT_VALUE 17
#define GWL_STYLE_VALUE (-16)
#define HWND_MESSAGE_VALUE ((HWND)(intptr_t)-3)

#define SW_HIDE_VALUE 0
#define SW_SHOWNOACTIVATE_VALUE 4
#define SW_SHOW_VALUE 5
#define SW_MINIMIZE_VALUE 6
#define SW_MAXIMIZE_VALUE 3
#define SW_RESTORE_VALUE 9

#define SPW_OBJECT_CAPACITY 1024u
#define SPW_EVENT_CAPACITY 1024u
#define SPW_MAX_TITLE_WCHARS 1024
#define SPW_MAX_CONTROL_TEXT_WCHARS 16384
#define SPW_MAX_TOOLTIP_WCHARS 2048
#define SPW_MAX_PLACEHOLDER_WCHARS 1024
#define SPW_MAX_DIALOG_PATH_WCHARS 32768
#define SPW_MAX_DIALOG_FILTER_WCHARS 8192

/* Public ABI constants -------------------------------------------------- */
#define SPW_ABI_VERSION 0x00290000u

#define SPW_STATUS_OK 0
#define SPW_STATUS_NOT_RUNNING (-1)
#define SPW_STATUS_BAD_ARGUMENT (-2)
#define SPW_STATUS_NOT_FOUND (-3)
#define SPW_STATUS_WIN32_ERROR (-4)
#define SPW_STATUS_CAPACITY (-5)
#define SPW_STATUS_START_FAILED (-6)

#define SPW_EVENT_CLOSE_REQUESTED 1u
#define SPW_EVENT_RESIZED 2u
#define SPW_EVENT_DESTROYED 3u
#define SPW_EVENT_CLICKED 4u
#define SPW_EVENT_BARRIER 5u
#define SPW_EVENT_TOGGLED 6u
#define SPW_EVENT_TEXT_CHANGED 7u
#define SPW_EVENT_SELECTION_CHANGED 8u
#define SPW_EVENT_VALUE_CHANGED 9u
#define SPW_EVENT_FOCUS_RECEIVED 10u
#define SPW_EVENT_FOCUS_LOST 11u
#define SPW_EVENT_LIST_SELECTION_CHANGED 12u
#define SPW_EVENT_ACTIVATED 13u
#define SPW_EVENT_COLUMN_CLICKED 14u
#define SPW_EVENT_TREE_SELECTION_CHANGED 15u
#define SPW_EVENT_TREE_EXPANSION_CHANGED 16u
#define SPW_EVENT_KEY_DOWN 17u
#define SPW_EVENT_KEY_UP 18u
#define SPW_EVENT_MOUSE_DOWN 19u
#define SPW_EVENT_MOUSE_UP 20u
#define SPW_EVENT_MOUSE_MOVE 21u
#define SPW_EVENT_MOUSE_ENTER 22u
#define SPW_EVENT_MOUSE_LEAVE 23u
#define SPW_EVENT_MOUSE_DOUBLE_CLICK 24u
#define SPW_EVENT_CONTEXT_MENU_REQUESTED 25u
#define SPW_EVENT_NUMBER_STEP 26u
#define SPW_EVENT_POPUP_DISMISS_REQUESTED 27u
#define SPW_EVENT_MENU_COMMAND 28u
#define SPW_EVENT_PANED_POSITION_CHANGED 29u
#define SPW_EVENT_SCROLL_POSITION_CHANGED 30u

#define SPW_MOD_SHIFT 0x0001u
#define SPW_MOD_CONTROL 0x0002u
#define SPW_MOD_ALT 0x0004u
#define SPW_MOD_META 0x0008u
#define SPW_MOUSE_BUTTON_NONE 0u
#define SPW_MOUSE_BUTTON_PRIMARY 1u
#define SPW_MOUSE_BUTTON_SECONDARY 2u
#define SPW_MOUSE_BUTTON_MIDDLE 3u

#define COLOR_WINDOW_VALUE 5
#define COLOR_HIGHLIGHT_VALUE 13
#define COLOR_BTNFACE_VALUE 15
#define COLOR_3DSHADOW_VALUE 16
#define TRANSPARENT_VALUE 1
#define DT_RIGHT_VALUE 0x00000002u
#define DT_VCENTER_VALUE 0x00000004u
#define DT_SINGLELINE_VALUE 0x00000020u
#define DT_NOPREFIX_VALUE 0x00000800u
#define SCF_SELECTION_VALUE 0x0001u
#define SCF_ALL_VALUE 0x0004u
#define CFM_BOLD_VALUE 0x00000001u
#define CFM_ITALIC_VALUE 0x00000002u
#define CFM_UNDERLINE_VALUE 0x00000004u
#define CFM_SIZE_VALUE 0x80000000u
#define CFM_COLOR_VALUE 0x40000000u
#define CFM_FACE_VALUE 0x20000000u
#define CFM_BACKCOLOR_VALUE 0x04000000u
#define CFE_BOLD_VALUE 0x00000001u
#define CFE_ITALIC_VALUE 0x00000002u
#define CFE_UNDERLINE_VALUE 0x00000004u
#define SPW_CODE_STYLE_BOLD 1u
#define SPW_CODE_STYLE_ITALIC 2u
#define SPW_CODE_STYLE_UNDERLINE 4u
#define SPW_CODE_GUTTER_WIDTH 48
#define SPW_CODE_MARK_GUTTER_WIDTH 24
#define SPW_MAX_CODE_MARKS 512u
#define SPW_CODE_MARK_INSERT 1u
#define SPW_CODE_MARK_DELETE 2u
#define SPW_CODE_MARK_CHANGE 3u
#define SPW_CODE_MARK_OTHER 4u
#define BLACK_PEN_VALUE 7
#define NULL_PEN_VALUE 8
#define SPW_EDIT_AFFORDANCE_WIDTH 18
#define SPW_PANED_SPLITTER_WIDTH 6
#define SPW_TEXT_COMMAND_COPY 1
#define SPW_TEXT_COMMAND_CUT 2
#define SPW_TEXT_COMMAND_PASTE 3
#define SPW_TEXT_COMMAND_UNDO 4
#define SPW_TEXT_COMMAND_CLEAR_UNDO 5
#define SPW_FILE_DIALOG_OPEN 1
#define SPW_FILE_DIALOG_SAVE 2
#define SPW_FILE_DIALOG_DIRECTORY 3
#define SPW_MESSAGE_INFO 1
#define SPW_MESSAGE_ERROR 2
#define MB_OK_VALUE 0x00000000u
#define MB_ICONERROR_VALUE 0x00000010u
#define MB_ICONINFORMATION_VALUE 0x00000040u
#define MB_SETFOREGROUND_VALUE 0x00010000u
#define OFN_OVERWRITEPROMPT_VALUE 0x00000002u
#define OFN_HIDEREADONLY_VALUE 0x00000004u
#define OFN_NOCHANGEDIR_VALUE 0x00000008u
#define OFN_PATHMUSTEXIST_VALUE 0x00000800u
#define OFN_FILEMUSTEXIST_VALUE 0x00001000u
#define OFN_EXPLORER_VALUE 0x00080000u
#define BIF_RETURNONLYFSDIRS_VALUE 0x0001u
#define BIF_EDITBOX_VALUE 0x0010u
#define BIF_NEWDIALOGSTYLE_VALUE 0x0040u
#define BFFM_INITIALIZED_VALUE 1u
#define BFFM_SETSELECTIONW_VALUE (WM_USER_VALUE + 103u)
#define BCM_SETIMAGELIST_VALUE 0x1602u
#define BUTTON_IMAGELIST_ALIGN_LEFT_VALUE 0u
#define BUTTON_IMAGELIST_ALIGN_CENTER_VALUE 4u
#define ILC_MASK_VALUE 0x00000001u
#define ILC_COLOR32_VALUE 0x00000020u

#define SPW_CONTROL_LABEL 1u
#define SPW_CONTROL_BUTTON 2u
#define SPW_CONTROL_CHECKBOX 3u
#define SPW_CONTROL_RADIOBUTTON 4u
#define SPW_CONTROL_TEXT_INPUT 5u
#define SPW_CONTROL_DROP_LIST 6u
#define SPW_CONTROL_SLIDER_HORIZONTAL 7u
#define SPW_CONTROL_SLIDER_VERTICAL 8u
#define SPW_CONTROL_PROGRESS 9u
#define SPW_CONTROL_NOTEBOOK 10u
#define SPW_CONTROL_LIST 11u
#define SPW_CONTROL_TABLE 12u
#define SPW_CONTROL_TREE 13u
#define SPW_CONTROL_TOGGLE_BUTTON 14u
#define SPW_CONTROL_SWITCH 15u
#define SPW_CONTROL_SPINNER 16u
#define SPW_CONTROL_LINK 17u
#define SPW_CONTROL_NUMBER_INPUT 18u
#define SPW_CONTROL_SEARCH_INPUT 19u
#define SPW_CONTROL_IMAGE 20u
#define SPW_CONTROL_SCROLL_VIEWPORT 21u
#define SPW_CONTROL_SCROLL_CONTENT 22u
#define SPW_CONTROL_PANED 23u
#define SPW_CONTROL_PANED_CONTENT 24u
#define SPW_CONTROL_TAB_CONTENT 25u
#define SPW_CONTROL_FRAME 26u
#define SPW_CONTROL_TEXT_AREA 27u
#define SPW_CONTROL_TREE_COLUMN 28u
#define SPW_CONTROL_COMPONENT_ROW 29u
#define SPW_CONTROL_CODE 30u

#define SPW_SCROLL_POLICY_DISABLED 0u
#define SPW_SCROLL_POLICY_AUTOMATIC 1u
#define SPW_SCROLL_POLICY_HIDDEN 2u
#define SPW_SCROLL_POLICY_ALWAYS 3u

#define BI_RGB_VALUE 0u
#define DIB_RGB_COLORS_VALUE 0u
#define AC_SRC_OVER_VALUE 0u
#define AC_SRC_ALPHA_VALUE 1u

#define SPW_OBJECT_WINDOW 1u
#define SPW_OBJECT_CONTROL 2u

