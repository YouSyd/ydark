#include "ydark.h"

/*
����Win32 Edit �ؼ��ı߾�˼·��
ͨ������WM_NCCALCSIZE�������÷ǿͻ�������text area��ѹ���ض�����.
 */
#define SCROLLBAR_PIXLS 13
#define THUMB_MIN_PIXLS 50
typedef enum SCROLLSTATE{
    LEFT_ARROW,
    LEFT_PAGE,
    THUMB_HCLICK,//ˮƽ����
    RIGHT_PAGE,
    RIGHT_ARROW,
    TOP_ARROW,
    TOP_PAGE,
    THUMB_VCLICK,//��ֱ����
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
    int font_cy;//����߶�,����������ʱ�����Զ��������������
    
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