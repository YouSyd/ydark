#include "ydark.h"

typedef struct __RS_STATUSBAR_STYLE__ {
    WNDPROC pre_proc;
    WNDPROC proc;
    HFONT font;
    
    int cy;
    
    int off_flow;
    
    COLORREF txt_colr;
    COLORREF bk_colr;
    COLORREF border_colr;
    int active_index;
}RStatusStyle,*pStatusStyle;

#define TIMER_TRIGGERMSGFLOW 0x0001
#define TIMER_MSGFLOW 0x0002

#define WMYU_HITITEM (UINT)(WM_USER+0x0020)

int StatusBar_InitialSettings(HWND hwnd);
pStatusStyle StatusBar_GetSettings(HWND hwnd);
int StatusBar_ClearSettings(HWND hwnd);
LRESULT CALLBACK StatusBarProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
int StatusBar_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam);
