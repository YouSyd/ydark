#pragma once
#include "ydark.h"

#define WINDOW_CLASS_NAME "FRAME BACK PANEL"

typedef struct _RSTRUCT_VIEWSRCOLLINFO_ {
    POINT drag_pt;
    POINT drag_pos;
    POINT view_offset;
}RViewSI,*pViewSI;

typedef struct _STRUCT_FRAME_STYLE_ {
   EFNCZone czone;
   HFONT font;
   COLORREF color_mibk;//menu item back color
   HBRUSH brush_mibk;
   COLORREF color_border;
   HBRUSH brush_border;
   
   COLORREF color_active;
   COLORREF color_deactive;
   HBRUSH brush_cap_active;
   HBRUSH brush_cap_deactive;
   
   COLORREF color_title;
   COLORREF color_text;
   COLORREF color_bk;
   
   COLORREF color_scroll;
   HBRUSH brush_scroll;
   
   HBRUSH brush_btnclose;
   HBRUSH brush_btn;
   
   HDC memdc;//mem dc double buffering flicker.
   HDC hdc;
   HBITMAP bmp;
   HPEN pen;
   
   WNDPROC proc;
   WNDPROC pre_proc;
   
   pViewSI pvsi;
   HWND hint;
}RFrameStyle,*pFrameStyle;

#define WM_DOCKINGONTHEWAY (UINT)(WM_USER+0x1001)
typedef enum DOCKER_STYLE {
    DOCK_ALDNXB,//东南西北
    DOCK_ALTOP2BOT,//从上到下
    DOCK_ALLEFT2RIGHT,//从左到右
    DOCK_ALLRTRB,//左、右上，右下
    DOCK_ALTLTRTTRBB,//上左、上右上，上右下，下
}EDockStyle;

typedef enum DOCKER_POS {
    AL_NONE,
    
    ALDNXB_D,
    ALDNXB_N,
    ALDNXB_X,
    ALDNXB_B,
    ALDNXB_Z,//东南西北中
    
    ALTOP2BOT_T,
    ALTOP2BOT_B,
    
    ALLEFT2RIGHT_L,
    ALLEFT2RIGHT_R,
    
    ALLRTRB_L, //左
    ALLRTRB_RT,//右上
    ALLRTRB_RB,//右下
    
    ALTLTRTTRBB_TL,
    ALTLTRTTRBB_TRT,
    ALTLTRTTRBB_TRB,
    ALTLTRTTRBB_B,
}EDockerPos;

typedef int (*LPHintGetArray)(HWND hint,LPPOINT pt,int* pcc);//获取hint区域的坐标
typedef EDockerPos (*LPHintHitTest)(HWND hint,POINT pt);
typedef int (*LPHintGetPoly)(HWND hwnd,EDockerPos pos,LPPOINT plist);
typedef int (*LPHintDock)(HWND holder,HWND drag_wnd,HWND hint);
typedef int (*LPHintDockSize)(HWND holder);
typedef struct _STRUCT_HINTINFO_ {
    EDockStyle style;

#define IsBitSet(value,n) ((((value)&(1<<((n)-1)))==(1<<((n)-1)))?1:0)
#define SetBit(value,n) (value)=((value)|(1<<((n)-1)))
#define ClearBit(value,n) (value)=((value)&(~((1<<((n)-1)))))   
    unsigned int dockbits;
    
    HWND* dockarray;
    
    EDockerPos pos;
    
    WNDPROC proc;
    WNDPROC pre_proc;
    
    LPHintGetArray getarray;
    LPHintHitTest hittest; 
    LPHintGetPoly getpoly;
    LPHintDock hintdock;
    LPHintDockSize docksize;
}RHintInfo,*pHintInfo;

int Frame_Register(HINSTANCE instance);
HWND Frame_Create(HINSTANCE instance,int width,int height);
int Frame_Run(HWND hwnd);
LRESULT CALLBACK FrameProc(HWND, UINT, WPARAM, LPARAM);

int Frame_InitialSettings(HWND hwnd);
int Frame_ClearSettings(HWND hwnd);
pFrameStyle Frame_GetSettings(HWND hwnd);

int Frame_GetMetrics(HWND hwnd,pMetrics mt);
int Frame_PopMenu(HWND hwnd,POINT pt);
BOOL Frame_IsMenuBarRect(HWND hwnd,POINT pt);
int Frame_GetMenuItemRect(HWND hwnd,int index,LPRECT prc);
int Frame_DrawMenuItem(LPDRAWITEMSTRUCT draw);
int Frame_NCDraw(HWND hwnd,LPVOID param);
int Frame_CopyBtnRect(HWND hwnd,LPRECT pbtn_rc,int btn_count);
int Frame_GetNCZoneRect(HWND hwnd,EFNCZone type,LPRECT prc,BOOL allign_topleft);
int Frame_NCCalcSize(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Frame_InitialMenuPopup(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Frame_NCHitTest(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Frame_TrackNCMouseLeave(HWND hwnd);
int Frame_StopNCMouseTrack(HWND hwnd);
int Frame_ScrollBarDraw(HWND hwnd);
pHintInfo Frame_GetHintSettings(HWND frame);
int Frame_SetHintWindow(HWND frame,HWND hint);
int Frame_AllignHintWindow(HWND frame);
HWND Frame_GetFrameFromCursor(HWND drag_hwnd);//返回停靠窗口句柄

int Frame_Dock(HWND frame);

int Frame_InitialViewSI(HWND hwnd);
pViewSI Frame_GetViewSI(HWND hwnd);
int Frame_ClearViewSI(HWND hwnd);
int Frame_SetViewHeight(HWND hwnd,int height);
int Frame_ViewHBeginDrag(HWND hwnd,POINT pt);
int Frame_ViewHDraging(HWND hwnd,POINT pt);
int Frame_ViewHStopDrag(HWND hwnd);
int Frame_ViewHSize(HWND hwnd,int h_offset);

void Frame_GradientRect(HDC hdc,RECT rc,COLORREF c1,COLORREF c2,int verical_or_horizen);

BOOL Hint_PtInPolygon(LPPOINT pl,int nvert,POINT pt);
pHintInfo Hint_InitialSettings(HWND hwnd,EDockStyle style);
pHintInfo Hint_GetSettings(HWND hwnd);
int Hint_ClearSettings(HWND hwnd);
HWND Hint_Create(HINSTANCE instance,EDockStyle style);//hint创建
LRESULT CALLBACK HintWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);//hint消息过程

//DOCK_ALDNXB
int Hint_GetPtArray_DNXB(HWND hint,LPPOINT pt,int* pcc);
EDockerPos Hint_HitTest_DNXB(HWND hint,POINT pt);
int Hint_GetPosPoly_DNXB(HWND hwnd,EDockerPos pos,LPPOINT plist);
int Hint_Dock_DNXB(HWND holder,HWND drag_wnd,HWND hint);

//DOCK_ALLRTRB
int Hint_GetPtArray_LRTRB(HWND hint,LPPOINT pt,int* pcc);
EDockerPos Hint_HitTest_LRTRB(HWND hint,POINT pt);
int Hint_GetPosPoly_LRTRB(HWND hwnd,EDockerPos pos,LPPOINT plist);
int Hint_Dock_LRTRB(HWND holder,HWND drag_wnd,HWND hint);
int Hint_DockSize_LRTRB(HWND frame);

//DOCK_ALTLTRTTRBB
int Hint_GetPtArray_TLTRTTRBB(HWND hint,LPPOINT pt,int* pcc);
EDockerPos Hint_HitTest_TLTRTTRBB(HWND hint,POINT pt);
int Hint_GetPosPoly_TLTRTTRBB(HWND hwnd,EDockerPos pos,LPPOINT plist);
int Hint_Dock_TLTRTTRBB(HWND holder,HWND drag_wnd,HWND hint);
int Hint_DockSize_TLTRTTRBB(HWND frame);
