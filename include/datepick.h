#include<windows.h>
#include<windowsx.h>
#include<commctrl.h>
#include<uxtheme.h>
#include<stdio.h>

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"UxTheme.lib")

typedef struct _STRUCT_DATEPICK_STYLE_ {
   HFONT font;
   WNDPROC proc;
   WNDPROC pre_proc;
   COLORREF clrbkgnd;
}RDatePickStyle,*pDatePickStyle;
int DatePick_InitialSettings(HWND hwnd);
int DatePick_ClearSettings(HWND hwnd);
pDatePickStyle DatePick_GetSettings(HWND hwnd);

LRESULT CALLBACK DTPOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int DatePick_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam);
void DatePick_Notify(HWND hwnd,WPARAM wParam,LPARAM lParam);