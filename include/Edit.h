#include "ydark.h"

#define SCROLLBAR_PIXLS 12
#define THUMB_MIN_PIXLS 50
#define IDT_SCROLL_MONITOR_LONGPRESS 0x0001
#define IDT_SCROLL_MONITOR_REPEATCLICK 0x0002
#define LONG_PRESS_MILLISECS 150
#define REPEAT_CLICK_MILLSECS 10
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

typedef struct _STRUCT_EDIT_STYLE_ {
    COLORREF color_bk;
    COLORREF color_bk_readonly;
    COLORREF color_text;
    COLORREF color_text_readonly;
    COLORREF color_title;
    COLORREF color_border;
    COLORREF color_bk_error;
    HBRUSH brush;
    HBRUSH brush_error;
    HBRUSH brush_readonly;
    HBRUSH brush_border;
    HFONT font;
    HFONT font_title;
    WNDPROC proc;
    WNDPROC pre_proc;
    int margin_left;
    int font_cy;//����߶�,����������ʱ�����Զ��������������
    char title[256];//����ǩ�������
    UINT internal_style;//0-�ַ��ͣ� 1-��ֵ�ͣ�2-������
    BOOL valid;
    int taborder;
    
    int margin_border;//����ģʽ
    EScrollState scroll_state;
    int scroll_offset;
    POINT dragpos;
    POINT dragpt;
}REditStyle,*pEditStyle;

int Edit_InitialSettings(HWND hwnd,char*,char*);
pEditStyle Edit_GetSettings(HWND hwnd);
int Edit_SetInternalStyle(HWND hwnd,int style);
int Edit_ClearSettings(HWND hwnd);
int Edit_NCCalcSize(HWND hwnd,LPNCCALCSIZE_PARAMS calc);
int Edit_NCPaint(HWND hwnd);
int Edit_UpdateValue(HWND hwnd);

int Edit_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL is_allignuptop);
int Edit_MultiLineAppend(HWND hwnd,char* appendstr);
BOOL Edit_IsShowVScroll(HWND hwnd);
BOOL Edit_IsShowHScroll(HWND hwnd);
int Edit_DrawHScrollBar(HWND hwnd);
int Edit_DrawVScrollBar(HWND hwnd);

LRESULT CALLBACK EditOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

