#include "ydark.h"

/*
调整Win32 Edit 控件的边距思路：
通过调整WM_NCCALCSIZE重新设置非客户区，将text area挤压至特定区域.
 */
#define SCROLLBAR_PIXLS 13
#define THUMB_MIN_PIXLS 50
typedef enum SCROLLSTATE{
    LEFT_ARROW,
    LEFT_PAGE,
    THUMB_HCLICK,//水平滚动
    RIGHT_PAGE,
    RIGHT_ARROW,
    TOP_ARROW,
    TOP_PAGE,
    THUMB_VCLICK,//垂直滚动
    BOTTOM_PAGE,
    BOTTOM_ARROW,
    SCROLL_NONE
} EScrollState;

typedef struct _STRUCT_LISTBOX_STYLE_ {
    COLORREF color_bk;
    COLORREF color_text;
    COLORREF color_border;
    HBRUSH brush;
    HBRUSH brush_border;
    HFONT font;
    WNDPROC proc;
    WNDPROC pre_proc;
    int margin_border;
    int font_cy;//字体高度,当创建字体时，会自动依据字体调整。
    
    int taborder;
    
    EScrollState scroll_state;
    POINT dragpt;
    POINT dragpos;
}RLBStyle,*pLBStyle;

int ListBox_InitialSettings(HWND hwnd);
pLBStyle ListBox_GetSettings(HWND hwnd);
int Edit_SetInternalStyle(HWND hwnd,int style);
int ListBox_ClearSettings(HWND hwnd);
int ListBox_NCCalcSize(HWND hwnd,LPNCCALCSIZE_PARAMS calc);
int ListBox_NCPaint(HWND hwnd);
int ListBox_DrawScrollBar(HWND hwnd);
LRESULT CALLBACK ListBoxOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);