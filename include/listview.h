#include "ydark.h"

#define SCROLLBAR_PIXLS 12
#define THUMB_MIN_PIXLS 50

#define WMYU_LISTVIEW (UINT)(WM_USER+0x0010)

typedef struct _RSTRUCT_LISTVIEWINFO_ {
    WNDPROC pre_proc;
    WNDPROC proc;
    HFONT font;
    COLORREF color_text;//文字颜色
    COLORREF rgb_even;//偶数行背景色
    COLORREF rgb_odd;//奇数行背景色
    COLORREF rgb_sel;//选中行背景色
    COLORREF rgb_focus;//聚焦行背景色
    int content_height;
    POINT drag_pos;
    POINT drag_pt;
    POINT ptCur;
    
    int col_index;
    int col_width;
    
    COLORREF color_sel;
    COLORREF color_even;
    COLORREF color_odd;
    
    HIMAGELIST imglist;
    
#ifdef FUCKOFFSWITCH    
    BOOL fuckoff_switch;
    char fuckoff_sentence[256];
#endif
#define WMYU_DRAWLISTVIEWITEM ((UINT)(WMYU_LISTVIEW+0x01))
}RListViewStyle,*pListViewStyle;

int ListView_InitialSettings(HWND hwnd);
inline pListViewStyle ListView_GetSettings(HWND hwnd);
int ListView_ClearSettings(HWND hwnd);
LRESULT CALLBACK LVCProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
int LVC_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam);//改写WM_PAINT
inline int LVC_PaintItem(HWND hwnd,HDC hdc,LPRECT prc,UINT itemid,int subitem);//subItem的改写
int LVC_DrawItem(HWND hwnd,WPARAM wParam,LPARAM lParam);//已抛弃
int LVC_NCCalcSize(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_Size(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL isallign_top_left);
int LVC_NCDrawScrollBar(HWND hwnd,UINT scrolltype);
int LVC_Scroll(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_FindItem(HWND hwnd,WPARAM wParam,LPARAM lParam);
BOOL LVC_EnsureVisible(HWND hwnd,WPARAM wParam,LPARAM lParam);
BOOL LVC_GetItemRect(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_HitTest(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_ScrollBefore(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_Scrolling(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_ScrollDone(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_SetState(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_SizeColumns(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_ResetScrollPos(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
int LVC_LButtonDown(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_LButtonUp(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_SetCursor(HWND hwnd,WPARAM wParam,LPARAM lParam);
int LVC_MouseMove(HWND hwnd,WPARAM wParam,LPARAM lParam);

typedef struct _RSTRUCT_LISTVIEWHEAD_ {
    WNDPROC pre_proc;
    WNDPROC proc;
    HFONT font;
    COLORREF color_text;//文字颜色
#define WMYU_MEASURELISTVIEWITEM ((UINT)(WMYU_LISTVIEW+0x02))   
    int title_height;
}RLVHeadStyle,*pLVHeadStyle;

int LVHead_InitialSettings(HWND hwnd);
pLVHeadStyle LVHead_GetSettings(HWND hwnd);
int LVHead_ClearSettings(HWND hwnd);
LRESULT CALLBACK LVHProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
//int LVHPaint(HWND hwnd,HDC hdc,LPVOID param);
int LVHPaint(HWND hwnd,WPARAM wParam,LPARAM lParem);
void GradientRect2(HDC hdc,RECT rc_tmp,COLORREF c1,COLORREF c2,int verical_or_horizen);