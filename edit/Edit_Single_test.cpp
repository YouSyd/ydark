/*
测试原生的win32 SysListView32的数据加载能力
*/
#include<windows.h>
#include<windowsx.h>
#include<math.h>
#include<stdio.h>
#include<commctrl.h>
#include<Richedit.h>
#include<gdiplus.h>
using namespace Gdiplus; 

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"gdiplus.lib")

#define IDC_EDIT_01 0x0001
#define IDC_EDIT_02 0x0002
#define IDC_EDIT_03 0x0003
#define IDC_EDIT_04 0x0004
#define IDC_EDIT_05 0x0005
#define IDC_EDIT_06 0x0006
#define IDC_EDIT_07 0x0007
#define IDC_EDIT_08 0x0008
#define IDC_EDIT_09 0x0009

#define WINDOW_CLASS_NAME "ListGRID"
#define RWIDTH(A)   abs(A.right - A.left)   
#define RHEIGHT(A)  abs(A.bottom - A.top)  

HINSTANCE instance;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int EditCtrl_Test(HWND parent);
int MultiScrollTest(HWND hwnd);

/*
调整Win32 Edit 控件的边距思路：
通过调整WM_NCCALCSIZE重新设置非客户区，将text area挤压至特定区域.
 */
#define SCROLLBAR_PIXLS 13
#define THUMB_MIN_PIXLS 30
#define IDT_SCROLL_MONITOR_LONGPRESS 0x0001
#define IDT_SCROLL_MONITOR_REPEATCLICK 0x0002
#define LONG_PRESS_MILLISECS 150
#define REPEAT_CLICK_MILLSECS 10
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
    int font_cy;//字体高度,当创建字体时，会自动依据字体调整。
    char title[256];//带标签的输入框
    UINT internal_style;//0-字符型， 1-数值型，2-浮点数
    BOOL valid;
    int taborder;
    
}REditStyle,*pEditStyle;

int Edit_InitialSettings(HWND hwnd,char*,char*);
pEditStyle Edit_GetSettings(HWND hwnd);
int Edit_SetInternalStyle(HWND hwnd,int style);
int Edit_ClearSettings(HWND hwnd);
int Edit_NCCalcSize(HWND hwnd,LPNCCALCSIZE_PARAMS calc);
int Edit_NCPaint(HWND hwnd);
int Edit_UpdateValue(HWND hwnd);
LRESULT CALLBACK EditOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR szcmdLine, int icmdshow)
{
	HWND hwnd;
	MSG msg;
	WNDCLASSEX winclass;
	InitCommonControls();
	instance = hinstance;

	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&winclass)) return 0;

	if (!(hwnd = CreateWindowEx(NULL,
								WINDOW_CLASS_NAME,
								"Edit test",
								WS_OVERLAPPEDWINDOW | WS_VISIBLE,
								240, 262,
								780,400,
								NULL,
								NULL,
								hinstance,
								NULL)))
		return 0;

	while (GetMessage(&msg, NULL, 0, 0)) {
	    if(!IsDialogMessage(hwnd,&msg)) { //保证tabstop消息正常发送
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	    }
	}

	return(msg.wParam);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CREATE: {
	    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        EditCtrl_Test(hwnd);
	} break;
	case WM_SIZE: {
		int height = HIWORD(lparam);
		int width = LOWORD(lparam);
		//设置Z序
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_01),HWND_BOTTOM,5,5,240,25,SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_02),HWND_BOTTOM,5,35,240,25,SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_03),HWND_BOTTOM,5,65,240,25,SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_04),HWND_BOTTOM,5,95,240,25,SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_05),HWND_BOTTOM,5,125,240,25,SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_07),HWND_BOTTOM,255,5,500,25,SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_08),HWND_BOTTOM,255,35,500,25,SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_09),HWND_BOTTOM,255,65,500,25,SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_06),HWND_BOTTOM,5,155,750,200,SWP_SHOWWINDOW|SWP_FRAMECHANGED);
	}break;
	case WM_DESTROY: {
	    GdiplusShutdown(gdiplusToken);
		PostQuitMessage(0);
		return (0);
	} break;
	case WM_COMMAND: {
	    HWND ctrl=(HWND)lparam;
	    UINT code=HIWORD(wparam);
	    if(code==EN_CHANGE) {
	        Edit_UpdateValue(ctrl);
	        InvalidateRect(ctrl,NULL,FALSE);
	        
	        if(ctrl==GetDlgItem(hwnd,IDC_EDIT_06)) {
	            //MultiScroll Test
	            MultiScrollTest(ctrl);
	        }
	    }
	} break;
	case WM_NOTIFY: {
	
	} break;
	case WM_CTLCOLOREDIT: {
	    HDC hdc=(HDC)wparam;
	    HWND edit=(HWND)lparam;
	    pEditStyle es=Edit_GetSettings(edit);
	    if(!es) break;
	    SetTextColor(hdc,es->color_text);
	    SetBkColor(hdc,es->valid?es->color_bk:es->color_bk_error);
	    return (HRESULT)(es->valid?es->brush:es->brush_error);
	} break;
	case WM_CTLCOLORSTATIC: { //Edit 在只读模式ES_READONLY下发送该消息
	    HDC hdc=(HDC)wparam;
	    HWND edit=(HWND)lparam;
	    pEditStyle es=Edit_GetSettings(edit);
	    if(!es) break;
	    SetTextColor(hdc,es->color_text_readonly);
	    SetBkColor(hdc,es->valid?es->color_bk_readonly:es->color_bk_error);
	    return (HRESULT)(es->valid?es->brush_readonly:es->brush_error);
	} break;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int Edit_InitialSettings(HWND hwnd,char* title,char* text)
{
    pEditStyle es=(pEditStyle)calloc(sizeof(REditStyle),1);
    if(!es) return -1;
    
    es->internal_style=0;
    es->valid=TRUE;
    
    es->font_cy=17;
    es->font=CreateFont(es->font_cy,0,0,0,
                    FW_MEDIUM,//FW_SEMIBOLD,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "Courier New");
    SendMessage(hwnd,WM_SETFONT,(WPARAM)es->font,NULL);
    es->font_title=CreateFont(es->font_cy,0,0,0,
                    FW_MEDIUM,//FW_SEMIBOLD,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "微软雅黑");
    es->color_bk=RGB(15,15,15);
    es->color_text=RGB(0,125,250);
    es->brush=CreateSolidBrush(es->color_bk);
    
    es->color_bk_error=RGB(250,0,0);
    es->brush_error=CreateSolidBrush(es->color_bk_error);
    
    es->margin_left=70;
    es->color_title=RGB(140,140,140);
    strcpy(es->title,title);
    strcat(es->title,"：");
    
    Edit_SetText(hwnd,text);
    
    //数值型右对齐
    if(GetWindowLongPtr(hwnd,GWL_STYLE)&ES_NUMBER||
       es->internal_style==1||es->internal_style==2) 
        SetWindowLongPtr(hwnd,GWL_STYLE,ES_RIGHT|GetWindowLongPtr(hwnd,GWL_STYLE));
    
    es->color_bk_readonly=RGB(35,35,35);
    es->color_text_readonly=RGB(100,100,100);
    es->brush_readonly=CreateSolidBrush(es->color_bk_readonly);
    
    es->color_border=RGB(70,70,70);
    es->brush_border=CreateSolidBrush(es->color_border);
    
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)es);
    es->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)EditOwnerProc);
    return 0;
}

pEditStyle Edit_GetSettings(HWND hwnd)
{
    return (pEditStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int Edit_ClearSettings(HWND hwnd)
{
    pEditStyle es=Edit_GetSettings(hwnd);
    if(!es) return -1;
    
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)es->pre_proc);
    DeleteObject(es->font);
    DeleteObject(es->font_title);
    DeleteObject(es->brush);
    DeleteObject(es->brush_readonly);
    DeleteObject(es->brush_border);
    DeleteObject(es->brush_error);
    
    free(es);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,0);
    return 0;
}

int Edit_SetInternalStyle(HWND hwnd,int style)
{
    pEditStyle es=Edit_GetSettings(hwnd);
    
    es->internal_style=style;
    if(style==1||style==2) {
        SetWindowLongPtr(hwnd,GWL_STYLE,ES_RIGHT|GetWindowLongPtr(hwnd,GWL_STYLE));
        Edit_UpdateValue(hwnd);
    }
    return 0;
}

int Edit_UpdateValue(HWND hwnd) 
{
    char value_text[256]="";
    char* pcheck=NULL;
    pEditStyle es=Edit_GetSettings(hwnd);
    
    if(!es) return 0;
    if(es->internal_style!=1&&es->internal_style!=2) {
        es->valid=TRUE;
        return 0;//非数值型不做检查
    }
    Edit_GetText(hwnd,value_text,sizeof(value_text));
    if(strlen(value_text)<=0) {
        es->valid=TRUE;
        return 0;
    }
    else if(strlen(value_text)==1&&(value_text[0]=='+'||value_text[0]=='-')) { 
        es->valid=TRUE;
        return 0;
    }
    else if(es->internal_style==2) {
        double value_double;
        
        value_double=strtod(value_text,&pcheck);
        if(*pcheck!=0) {
            es->valid=FALSE;
            Edit_SetSel(hwnd,pcheck-value_text,strlen(value_text));
            
            //LPWSTR pszText=TEXT("数值转化");
            //EDITBALLOONTIP btip={0};
            //btip.cbStruct=sizeof(EDITBALLOONTIP);
            //btip.pszTitle=NULL;//pszText;
            //btip.pszText=NULL;//LTEXT("输入字符无法处理为有效的浮点数值，请检查。");
            //btip.ttiIcon=TTI_INFO;
            //Edit_ShowBalloonTip(hwnd,&btip);
        }
        else {
            es->valid=TRUE;
        }
    }
    else {
        long value_long;
        value_long=strtol(value_text,&pcheck,10);
        if(*pcheck!=0) {
            es->valid=FALSE;
            Edit_SetSel(hwnd,pcheck-value_text,strlen(value_text));
        }
        else {
            es->valid=TRUE;
        }
    }
    
    return (es->valid?0:-1); 
}

int Edit_NCPaint(HWND hwnd)
{
    pEditStyle es=Edit_GetSettings(hwnd);
    
    if(!es) return -1;
    //NC背景用client的DC进行填充
    HDC hdc=GetWindowDC(hwnd);
    RECT wnd_rc={0},title_rc={0},text_rc={0};
    //获取字体高度信息
    int cy=0;
    TEXTMETRIC tm={0};
    GetTextMetrics(hdc,&tm);
    cy=tm.tmHeight+tm.tmExternalLeading;
    
    GetWindowRect(hwnd,&wnd_rc);
    OffsetRect(&wnd_rc,-wnd_rc.left,-wnd_rc.top);
    if(!(GetWindowLongPtr(hwnd,GWL_STYLE)&ES_MULTILINE)) {
        if(GetWindowLongPtr(hwnd,GWL_STYLE)&ES_READONLY)
            FillRect(hdc,&wnd_rc,es->brush_readonly);
        else FillRect(hdc,&wnd_rc,es->brush);
        
        CopyRect(&title_rc,&wnd_rc);
        title_rc.right=title_rc.left+es->margin_left;
        SetTextColor(hdc,es->color_title);
        SetBkMode(hdc,TRANSPARENT);
        SelectObject(hdc,es->font_title);
        //获取父窗口背景色
        HWND parent=GetParent(hwnd);
        if(parent) {
            //获取父窗口的背景色。通过WM_ERASEBKGND获取         
            //HDC parentdc=GetDC(parent);
            HBRUSH brush_title=CreateSolidBrush(RGB(0,0,0));//(HBRUSH)SendMessage(parent,WM_ERASEBKGND,(WPARAM)parentdc,0);
            FillRect(hdc,&title_rc,brush_title);
            DeleteObject(brush_title);
            //ReleaseDC(parent,parentdc);
        }
        DrawText(hdc,es->title,-1,&title_rc,DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
    }
    else {
        //绘制滚动条
    }
    
    CopyRect(&text_rc,&wnd_rc);
    text_rc.left=title_rc.right;
    FrameRect(hdc,&text_rc,es->brush_border);
    
    ReleaseDC(hwnd,hdc);
    InvalidateRect(hwnd,NULL,TRUE);
    return 0;
}

int Edit_NCCalcSize(HWND hwnd,LPNCCALCSIZE_PARAMS calc)
{
    RECT rect_new;
    RECT rect_old;
    RECT client_rect_new;
    RECT client_rect_old;
    pEditStyle es=Edit_GetSettings(hwnd);   
    //调整非客户区的位置和大小
    //处理前
    //0:新 1:老 2:老客户区
    CopyRect(&rect_new,&(calc->rgrc[0]));
    CopyRect(&rect_old,&(calc->rgrc[1]));
    CopyRect(&client_rect_old,&(calc->rgrc[2]));
    
        
    HDC hdc=GetWindowDC(hwnd);
    RECT wnd_rc={0};
    //获取字体高度信息
    int cy=0,top_margin=0;;
    TEXTMETRIC tm={0};
    GetTextMetrics(hdc,&tm);
    cy=tm.tmHeight+tm.tmExternalLeading;
    top_margin=((rect_new.bottom-rect_new.top)-es->font_cy)>>1;
    if(top_margin<=0) top_margin=0;
    //更新font_cy.
    es->font_cy=cy;
        
    //处理后
    //0:新客户区 1:新 2:老
    client_rect_new = {rect_new.left+es->margin_left+2,
                       rect_new.top+top_margin,
                       rect_new.right-2,
                       rect_new.bottom-top_margin};
    CopyRect(&(calc->rgrc[0]),&client_rect_new);
    CopyRect(&(calc->rgrc[1]),&rect_new);
    CopyRect(&(calc->rgrc[2]),&rect_old);
    
    return WVR_VALIDRECTS;
}

int Edit_NCCalSize_MultiLines(HWND hwnd,LPNCCALCSIZE_PARAMS calc)
{
    RECT rect_new;
    RECT rect_old;
    RECT client_rect_new;
    RECT client_rect_old;
    //pEditStyle es=Edit_GetSettings(hwnd);   
    //调整非客户区的位置和大小
    //处理前
    //0:新 1:老 2:老客户区
    CopyRect(&rect_new,&(calc->rgrc[0]));
    CopyRect(&rect_old,&(calc->rgrc[1]));
    CopyRect(&client_rect_old,&(calc->rgrc[2]));
    
    //处理后
    //0:新客户区 1:新 2:老
    client_rect_new = {rect_new.left+2,
                       rect_new.top+2,
                       rect_new.right-2,
                       rect_new.bottom-2};
    CopyRect(&(calc->rgrc[0]),&client_rect_new);
    CopyRect(&(calc->rgrc[1]),&rect_new);
    CopyRect(&(calc->rgrc[2]),&rect_old);
    
    return WVR_VALIDRECTS;
}

LRESULT CALLBACK EditOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    pEditStyle es=Edit_GetSettings(hwnd);
    switch(msg) {
    case WM_NCCALCSIZE: {
        if(wParam==TRUE) {
            LPNCCALCSIZE_PARAMS calc_param = (LPNCCALCSIZE_PARAMS)lParam;
            
            //多行模式的调整则不同
            if(GetWindowLongPtr(hwnd,GWL_STYLE)&ES_MULTILINE) 
                return Edit_NCCalSize_MultiLines(hwnd,calc_param);
            else 
                return Edit_NCCalcSize(hwnd,calc_param);
        }
    } break;
    case WM_NCPAINT:
    case WM_NCACTIVATE: {
        Edit_NCPaint(hwnd);
        return 0;
    } break;
    case WM_NCDESTROY: {
        Edit_ClearSettings(hwnd);
    }
    }
    return CallWindowProc(es->pre_proc,hwnd,msg,wParam,lParam);
}

int EditCtrl_Test(HWND hwnd)
{
    HWND edit1 = CreateWindowEx(NULL,
    						   WC_EDIT, "",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_01,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit1,"单行测试","Edit control test.");    
    
    HWND edit2 = CreateWindowEx(NULL,
    						   WC_EDIT, "Edit control test.",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_02,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit2,"浮点数值","测试Text标题");
    Edit_SetInternalStyle(edit2,2);
    
    HWND edit3 = CreateWindowEx(NULL,
    						   WC_EDIT, "",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_03,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit3,"只读模式","ES_READONLY Text");
    Edit_SetReadOnly(edit3,TRUE);
    
    HWND edit4 = CreateWindowEx(NULL,
    						   WC_EDIT, "",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE|ES_PASSWORD,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_04,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit4,"密码模式","ES_PASSWORD");
    
    HWND edit5 = CreateWindowEx(NULL,
    						   WC_EDIT, "",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE|ES_NUMBER,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_05,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit5,"数值模式","2729");
    
    HWND edit6 = CreateWindowEx(NULL,
    						   WC_EDIT, "",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|WS_VSCROLL|WS_HSCROLL,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_06,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit6,"多行模式","Multi-Line Edit control.");
    
    HWND edit7 = CreateWindowEx(NULL,
    						   WC_EDIT, "Edit control test.",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_07,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit7,"垂直滚动","测试Text标题");
    Edit_SetReadOnly(edit7,TRUE);
    HWND edit8 = CreateWindowEx(NULL,
    						   WC_EDIT, "Edit control test.",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_08,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit8,"水平滚动","测试Text标题");
    Edit_SetReadOnly(edit8,TRUE);
    HWND edit9 = CreateWindowEx(NULL,
    						   WC_EDIT, "Edit control test.",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_09,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit9,"WM_VSCROLL","测试Text标题");
    Edit_GetSettings(edit9)->margin_left=100;
    
    return 0;
}

int MultiScrollTest(HWND hwnd) 
{
    SCROLLINFO si={0};
    char text[256]="";
    
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_VERT,&si);
    
    sprintf(text,"SB_VERT\n nMin:%d,nMax:%d,nPos:%d,nPage:%d",
            si.nMin,si.nMax,si.nPos,si.nPage);
    Edit_SetText(GetDlgItem(GetParent(hwnd), IDC_EDIT_07),text);
    
    GetScrollInfo(hwnd,SB_HORZ,&si);
    sprintf(text,"SB_HORZ\n nMin:%d,nMax:%d,nPos:%d,nPage:%d",
            si.nMin,si.nMax,si.nPos,si.nPage);
    Edit_SetText(GetDlgItem(GetParent(hwnd), IDC_EDIT_08),text);
    
    return 0;
}

