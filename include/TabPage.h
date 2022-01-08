#include "ydark.h"

typedef struct _STRUCT_TABITEM_ {
    int index;
    HWND hwnd;
}RTabItem,*pTabItem;

typedef enum TABPAGE_POS{
    TAB_TOP,
    TAB_LEFT,
} ETabPagePos;
typedef struct _STRUCT_TABCTRL_STYLE_ {
    COLORREF color_bk;
    COLORREF color_text;
    COLORREF color_border;
    HBRUSH brush;
    HBRUSH brush_border;
    HFONT font;
    WNDPROC proc;
    WNDPROC pre_proc;
    int margin_border;
    int font_cy;//����߶�,����������ʱ�����Զ��������������
    
    int item_cx;
    int item_cy;
    
    int taborder;
    
    POINT dragpt;
    POINT dragpos;
    
    //����ԭ�е�index
    int index_Sel;
    int index_hover;
    
    //pTabItem pitem;
    ETabPagePos tab_pos;
}RTabStyle,*pTabStyle;

int Tab_InitialSettings(HWND hwnd);
pTabStyle Tab_GetSettings(HWND hwnd);
int Tab_ClearSettings(HWND hwnd);
int Tab_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Tab_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL IsAllign_TopLeft);
int Tab_AppendPage(HWND tab,HWND page,char* title);
HWND Tab_GetUpDown(HWND hwnd);
LRESULT CALLBACK TabCtrlOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef struct _RSEARCHPARAM_ {
    HWND parent;
    char class_name[256];
    HWND hwnd;
}RSearchParam,*pSearchParam;
