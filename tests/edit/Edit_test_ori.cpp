/*
Edit控件的部分改造
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
#define IDC_EDIT_0A 0x000A

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
    
    EScrollState scroll_state;
    int scroll_offset;
}REditStyle,*pEditStyle;

int Edit_InitialSettings(HWND hwnd,char*,char*);
pEditStyle Edit_GetSettings(HWND hwnd);
int Edit_SetInternalStyle(HWND hwnd,int style);
int Edit_ClearSettings(HWND hwnd);
int Edit_NCCalcSize(HWND hwnd,LPNCCALCSIZE_PARAMS calc);
int Edit_NCPaint(HWND hwnd);
int Edit_UpdateValue(HWND hwnd);

int Edit_MultiLineAppend(HWND hwnd_trigger,HWND hwnd,char* appendstr);
BOOL Edit_IsShowVScroll(HWND hwnd);
BOOL Edit_IsShowHScroll(HWND hwnd);
int Edit_GetVScrollRect(HWND hwnd,LPRECT prc);
int Edit_GetHScrollRect(HWND hwnd,LPRECT prc);
int Edit_DrawHScrollBar(HWND hwnd);
int Edit_DrawVScrollBar(HWND hwnd);

LRESULT CALLBACK EditOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int Menu_drawitem(LPDRAWITEMSTRUCT draw);

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
								800, 500,
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
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_06),HWND_BOTTOM,5,155,750,350,SWP_SHOWWINDOW|SWP_FRAMECHANGED);
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_0A),HWND_BOTTOM,780,5,400,500,SWP_SHOWWINDOW|SWP_FRAMECHANGED);
		
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
	        if(ES_MULTILINE&GetWindowLongPtr(ctrl,GWL_STYLE)) {
	            RECT ctrl_rc={0};
	            GetWindowRect(ctrl,&ctrl_rc);
	            POINT pt1={ctrl_rc.left,ctrl_rc.top},pt2={ctrl_rc.right,ctrl_rc.bottom};
	            ScreenToClient(hwnd,&pt1);
	            ScreenToClient(hwnd,&pt2);
	            SetWindowPos(ctrl,0,pt1.x,pt1.y,pt2.x-pt1.x,pt2.y-pt1.y,SWP_SHOWWINDOW|SWP_FRAMECHANGED|SWP_NOZORDER);
	            Edit_DrawVScrollBar(ctrl);
	        }
	        
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
    client_rect_new = {rect_new.left,//-(Edit_IsShowVScroll(hwnd)?SCROLLBAR_PIXLS:0),
                       rect_new.top,
                       rect_new.right-(Edit_IsShowVScroll(hwnd)?SCROLLBAR_PIXLS:0),
                       rect_new.bottom};
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
        Edit_DrawVScrollBar(hwnd);
        
        //Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_NCPAINT/WM_NCACTIVATE\r\n");
        
        return 0;
    } break;
    case WM_MOUSEWHEEL: {
        Edit_DrawVScrollBar(hwnd);
        //Edit_NCPaint(hwnd);
        //Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_MOUSEWHEEL\r\n");
    } break;
    case WM_KEYDOWN: {
        Edit_DrawVScrollBar(hwnd);
        Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_KEYDOWN\r\n");
    } break;
    case WM_VSCROLL: {//垂直
        Edit_DrawVScrollBar(hwnd);
        MultiScrollTest(GetDlgItem(GetParent(hwnd),IDC_EDIT_06));
        Edit_SetText(GetDlgItem(GetParent(hwnd),IDC_EDIT_09),"WM_VSCROLL");
        Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_VSCROLL\r\n");
    } break;
    case WM_HSCROLL: {
        Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_HSCROLL\r\n");
    } break;
    case WM_NCHITTEST: {
        
        Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_NCHITTEST\r\n");
        LRESULT result=CallWindowProc(es->pre_proc,hwnd,msg,wParam,lParam);
        POINT hit={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        ScreenToClient(hwnd,&hit);
        if(ES_MULTILINE&GetWindowLongPtr(hwnd,GWL_STYLE)) {
            if(Edit_IsShowVScroll(hwnd)) {
                RECT hrc[4]={0};
                Edit_GetVScrollRect(hwnd,hrc);
                if(PtInRect(&hrc[0],hit)) {
                    Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"滚动条区域.\r\n");
                    return HTVSCROLL;
                }
            }
        }
        char text[256]="";
        sprintf(text,"%d\r\n",result);
        Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),text);
        return result;
    } break;
    case WM_NCLBUTTONDOWN: {
        Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_NCLBUTTONDOWN\r\n");
        //判断鼠标是否在滑块区域， 并SetCapture;
        RECT rc[4]={0};
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        ScreenToClient(hwnd,&pt);
        Edit_GetVScrollRect(hwnd,rc);
        
        if(ES_MULTILINE&GetWindowLongPtr(hwnd,GWL_STYLE)) {
            if(PtInRect(&rc[2],pt)) {
                es->scroll_state=THUMB_VCLICK;
                es->scroll_offset=pt.y-rc[2].top;
                //SetCapture(hwnd);
            }
        }
    } break;
    case WM_SETCURSOR: {
        Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_SETCURSOR\r\n");
        //设置滚动状态,并根据需要启动长按计时
        UINT hit_pos=LOWORD(lParam);
        UINT trigger_msg=HIWORD(lParam);
        
        //if(hit_pos!=HTVSCROLL) return CallWindowProc(es->pre_proc,hwnd,msg,wParam,lParam);
        
        if(trigger_msg==WM_LBUTTONDOWN||trigger_msg==WM_NCLBUTTONDOWN) {
            POINT pt={0};
            RECT rc[4]={0};
            
            GetCursorPos(&pt);
            ScreenToClient(hwnd,&pt);
            Edit_GetVScrollRect(hwnd,rc);
            
            if(PtInRect(&rc[0],pt)) {
                SetCapture(hwnd);
                if(PtInRect(&rc[1],pt)) {//上箭头
                    es->scroll_state=TOP_ARROW;
                    SetTimer(hwnd,IDT_SCROLL_MONITOR_LONGPRESS,LONG_PRESS_MILLISECS,NULL);
                }
                else if(PtInRect(&rc[2],pt)) {//滑块
                    es->scroll_state=THUMB_VCLICK;
                    Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_SETFOCUS/滑块VSCROLL\r\n");
                }
                else if(PtInRect(&rc[3],pt)) {//下箭头
                    es->scroll_state=BOTTOM_ARROW;
                    SetTimer(hwnd,IDT_SCROLL_MONITOR_LONGPRESS,LONG_PRESS_MILLISECS,NULL);
                }
                else { //page
                    if(pt.y<rc[2].top) es->scroll_state=TOP_PAGE;
                    else es->scroll_state=BOTTOM_PAGE;
                    SetTimer(hwnd,IDT_SCROLL_MONITOR_LONGPRESS,LONG_PRESS_MILLISECS,NULL);
                }
            }
        }
    } break;
    case WM_NCLBUTTONUP:
    case WM_LBUTTONUP: {
        Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_NCLBUTTONUP/WM_LBUTTONUP\r\n");
        KillTimer(hwnd,IDT_SCROLL_MONITOR_LONGPRESS);
        KillTimer(hwnd,IDT_SCROLL_MONITOR_REPEATCLICK);
        ReleaseCapture();
    } break;
    case WM_NCMOUSEMOVE: {
        Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_NCMOUSEMOVE\r\n");
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        RECT rc[4]={{0,0,0,0}};
        Edit_GetVScrollRect(hwnd,rc);
        ScreenToClient(hwnd,&pt);
        
        //if(GetCapture()==hwnd) {
            if(es->scroll_state==THUMB_VCLICK) {
                //垂直滚动
                SCROLLINFO si={0};
                si.cbSize=sizeof(si);
                si.fMask=SIF_ALL;
                GetScrollInfo(hwnd,SB_VERT,&si);
                Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_MOUSEMOVE/处理滑块滑动\r\n");
                int cur_pos=(pt.y-es->scroll_offset-rc[1].bottom)*1.0/(rc[3].top-rc[1].bottom-(rc[2].bottom-rc[2].top))*(si.nMax-si.nMin+1-si.nPage);
                Edit_Scroll(hwnd,(cur_pos-si.nPos),0);
                
                Edit_DrawVScrollBar(hwnd);
            }
            
        //}
        //else es->scroll_state=SCROLL_NONE;
    } break;
    case WM_TIMER: {
        //响应长按
        if(wParam==IDT_SCROLL_MONITOR_LONGPRESS) {
            //结束长按计时，开始重复计时
            Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_TIMER/开始长摁\r\n");
            KillTimer(hwnd,IDT_SCROLL_MONITOR_LONGPRESS);
            SetTimer(hwnd,IDT_SCROLL_MONITOR_REPEATCLICK,REPEAT_CLICK_MILLSECS,NULL);
        }
        else if(wParam==IDT_SCROLL_MONITOR_REPEATCLICK) { 
            Edit_MultiLineAppend(hwnd,GetDlgItem(GetParent(hwnd),IDC_EDIT_0A),"WM_TIMER/长摁消息\r\n");
            SCROLLINFO si={0};
            si.cbSize=sizeof(si);
            si.fMask=SIF_ALL;
            
            POINT pt={0};
            RECT rc={0,0,0,0};
            
            GetClientRect(hwnd,&rc);
            
            if(GetCapture()!=hwnd) {
                KillTimer(hwnd,IDT_SCROLL_MONITOR_REPEATCLICK);
                es->scroll_state=SCROLL_NONE;
                return 0;
            }
            
            GetCursorPos(&pt);
            ScreenToClient(hwnd,&pt);
                        
            switch(es->scroll_state) {
            case TOP_ARROW: {   
                //滚动一个单位  
                GetScrollInfo(hwnd,SB_VERT,&si);
                if(si.nPos<=si.nMin) {
                    KillTimer(hwnd,IDT_SCROLL_MONITOR_REPEATCLICK);
                    ReleaseCapture();
                } 
                else {
                    Edit_Scroll(hwnd,-1,0);
                    Edit_DrawVScrollBar(hwnd);
                }           
            }
            break;
            case TOP_PAGE:{  
                //滚动一页
                GetScrollInfo(hwnd,SB_VERT,&si);
                Edit_Scroll(hwnd,-si.nPage,0);
                Edit_DrawVScrollBar(hwnd);
                
                RECT rcs[4]={0};
                Edit_GetVScrollRect(hwnd,rcs);
                if(PtInRect(&rcs[2],pt)) KillTimer(hwnd,IDT_SCROLL_MONITOR_REPEATCLICK);
            }
            break;
            case BOTTOM_PAGE: {   
                //滚动一页
                GetScrollInfo(hwnd,SB_VERT,&si);
                Edit_Scroll(hwnd,si.nPage,0);
                Edit_DrawVScrollBar(hwnd);
                
                RECT rcs[4]={0};
                Edit_GetVScrollRect(hwnd,rcs);
                if(PtInRect(&rcs[2],pt)) KillTimer(hwnd,IDT_SCROLL_MONITOR_REPEATCLICK);
            }
            break;
            case BOTTOM_ARROW:{ 
                GetScrollInfo(hwnd,SB_VERT,&si);
                if(si.nPos>si.nMax-si.nPage) {
                    KillTimer(hwnd,IDT_SCROLL_MONITOR_REPEATCLICK);
                    ReleaseCapture();
                }
                else {
                    Edit_Scroll(hwnd,1,0);
                    Edit_DrawVScrollBar(hwnd);
                }
            }
            break;
            }
        }
    } break;
    case WM_NCDESTROY: {
        Edit_ClearSettings(hwnd);
    }
    }
    return CallWindowProc(es->pre_proc,hwnd,msg,wParam,lParam);
}

int Edit_MultiLineAppend(HWND hwnd_trigger,HWND hwnd,char* appendstr)
{
    if(hwnd_trigger==GetDlgItem(GetParent(hwnd),IDC_EDIT_06)) {
        int multi_length=Edit_GetTextLength(hwnd);
        Edit_SetSel(hwnd,multi_length,multi_length);
        Edit_ReplaceSel(hwnd,appendstr);
    }
    return 0;
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
    Edit_InitialSettings(edit2,"标题","测试Text标题");
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
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|WS_VSCROLL/*|WS_HSCROLL*/,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_06,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit6,"多行模式","Multi-Line Edit control.");
    SetWindowLongPtr(edit6,GWL_EXSTYLE,WS_EX_LEFTSCROLLBAR|GetWindowLongPtr(edit6,GWL_EXSTYLE));
    
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
    Edit_GetSettings(edit9)->margin_left=110;
    
    HWND edita = CreateWindowEx(NULL,
    						   WC_EDIT, "Edit control test.",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|WS_VSCROLL,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_0A,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edita,"WM_VSCROLL","测试Text标题");
    Edit_SetReadOnly(edita,TRUE);
    SetWindowLongPtr(edita,GWL_EXSTYLE,WS_EX_LEFTSCROLLBAR|GetWindowLongPtr(edita,GWL_EXSTYLE));
    
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

int Edit_GetVScrollRect(HWND hwnd,LPRECT prc)
{
    SCROLLINFO si={0};
    //垂直滚动条
    BOOL vscroll_flag=(WS_VSCROLL)&GetWindowLongPtr(hwnd,GWL_STYLE);
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_VERT,&si);
    if(si.nPage>=si.nMax-si.nMin+1) {//无需绘制垂直滚动条
        return 1;
    }
    else {
        RECT rc={0},vrc={0};
        GetWindowRect(hwnd,&rc);
        OffsetRect(&rc,-rc.left,-rc.top);
        vrc.left=rc.right-SCROLLBAR_PIXLS;
        vrc.right=rc.right;
        vrc.top=rc.top;
        vrc.bottom=rc.bottom;
        CopyRect(prc,&vrc);
        CopyRect(prc+1,prc);
        CopyRect(prc+3,prc);
        prc[1].bottom=prc[0].top+SCROLLBAR_PIXLS;
        prc[3].top=prc[0].bottom-SCROLLBAR_PIXLS;
        
        int thumb_pixls=si.nPage*1.0/(si.nMax-si.nMin+1)*(vrc.bottom-vrc.top-2*SCROLLBAR_PIXLS);
        thumb_pixls=(thumb_pixls<THUMB_MIN_PIXLS)?THUMB_MIN_PIXLS:thumb_pixls;
        int thumb_pos=si.nPos*1.0/(si.nMax-si.nMin+1-si.nPage)*(vrc.bottom-vrc.top-2*SCROLLBAR_PIXLS-thumb_pixls);
        CopyRect(prc+2,prc);
        prc[2].top=prc[1].bottom+thumb_pos;
        prc[2].bottom=prc[2].top+thumb_pixls;
    }
    
    return 0;   
}

BOOL Edit_IsShowVScroll(HWND hwnd)
{
    SCROLLINFO si={0};
    //垂直滚动条
    BOOL vscroll_flag=(WS_VSCROLL)&GetWindowLongPtr(hwnd,GWL_STYLE);
    if(!vscroll_flag) return FALSE;
    
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_VERT,&si);
    if(si.nPage>=si.nMax-si.nMin+1) {//无需绘制垂直滚动条
        return FALSE;
    }
    else return TRUE;
}

int Edit_GetHScrollRect(HWND hwnd,LPRECT prc)
{
    SCROLLINFO si={0};
    //水平滚动条
    BOOL hscroll_flag=(WS_HSCROLL)&GetWindowLongPtr(hwnd,GWL_STYLE);
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_HORZ,&si);
    if(si.nPage>=si.nMax-si.nMin+1) {//无需绘制水平滚动条
        return 1;
    }
    else {
        RECT rc={0},hrc={0};
        GetClientRect(hwnd,&rc);
        hrc.left=rc.left;
        hrc.right=rc.right;
        hrc.top=rc.bottom;
        hrc.bottom=hrc.top+SCROLLBAR_PIXLS;
        CopyRect(prc,&hrc);
        CopyRect(prc+1,prc);
        CopyRect(prc+3,prc);
        prc[1].right=prc[1].left+SCROLLBAR_PIXLS;
        prc[3].left=prc[0].right-SCROLLBAR_PIXLS;
        
        int thumb_pixls=si.nPage*1.0/(si.nMax-si.nMin+1)*(hrc.right-hrc.left-2*SCROLLBAR_PIXLS);
        thumb_pixls=(thumb_pixls<THUMB_MIN_PIXLS)?THUMB_MIN_PIXLS:thumb_pixls;
        int thumb_pos=si.nPos*1.0/(si.nMax-si.nMin+1-si.nPage)*(hrc.right-hrc.left-2*SCROLLBAR_PIXLS-thumb_pixls);
        CopyRect(prc+2,prc);
        prc[2].left=prc[1].right+thumb_pos;
        prc[2].right=prc[2].left+thumb_pixls;
    }
    
    return 0;   
}

BOOL Edit_IsShowHScroll(HWND hwnd)
{
    SCROLLINFO si={0};
    //垂直滚动条
    BOOL hscroll_flag=(WS_HSCROLL)&GetWindowLongPtr(hwnd,GWL_STYLE);
    if(!hscroll_flag) return FALSE;
    
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_HORZ,&si);
    if(si.nPage>=si.nMax-si.nMin+1) {//无需绘制垂直滚动条
        return FALSE;
    }
    else return TRUE;
}

int Edit_DrawHScrollBar(HWND hwnd)
{
    RECT rc={0};
    RECT thumb_rc={0},brc={0},erc={0};
    HDC hdc,memdc;
    HBITMAP bmp,pre_bmp;
    HBRUSH brush,pre_brush,thumb_brush;
    HPEN pen,pre_pen;
    COLORREF thumb_color;
    LPRECT prc=NULL;
    RECT rc_scroll[4]={0};
    
    BOOL is_hscrollbar=Edit_IsShowHScroll(hwnd);
    
    if(!is_hscrollbar) return 0;
    if(prc->bottom-prc->top==0) return 0;
        
    Edit_GetHScrollRect(hwnd,rc_scroll);
    prc=rc_scroll;
    
    hdc=GetWindowDC(hwnd);
    memdc=CreateCompatibleDC(hdc);
    CopyRect(&rc,prc);
    OffsetRect(&rc,-prc->left,-prc->top);
    bmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
    pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    brush=CreateSolidBrush(RGB(30,30,30));
    pre_brush=(HBRUSH)SelectObject(memdc,brush);
    pen=CreatePen(PS_SOLID,1,RGB(90,90,90));
    pre_pen=(HPEN)SelectObject(memdc,pen);
    
    Rectangle(memdc,rc.left,rc.top,rc.right,rc.bottom);
    thumb_color=RGB(45,45,45);
    CopyRect(&thumb_rc,prc+2);
    CopyRect(&brc,prc+1);
    CopyRect(&erc,prc+3);
    OffsetRect(&thumb_rc,-prc->left,-prc->top);
    OffsetRect(&brc,-prc->left,-prc->top);
    OffsetRect(&erc,-prc->left,-prc->top);
    DeleteObject(SelectObject(memdc,pre_brush));
    thumb_brush=CreateSolidBrush(thumb_color);
    pre_brush=(HBRUSH)SelectObject(memdc,thumb_brush);
    InflateRect(&thumb_rc,0,-1);
        
    if(is_hscrollbar) {
        Graphics graphic(memdc);
        graphic.SetSmoothingMode(SmoothingModeHighQuality);
        GraphicsPath path;
        
        LinearGradientBrush pbrush(Rect(thumb_rc.left,thumb_rc.top,SCROLLBAR_PIXLS-2,SCROLLBAR_PIXLS-2),
                                  Color(255,40,40,40),Color(255,90,90,90),LinearGradientModeVertical);
        path.AddArc(thumb_rc.left,thumb_rc.top,SCROLLBAR_PIXLS-2,SCROLLBAR_PIXLS-2,
                    90,180);
        path.AddArc(thumb_rc.right-SCROLLBAR_PIXLS,thumb_rc.top,SCROLLBAR_PIXLS-2,SCROLLBAR_PIXLS-2,
                    -90,180);
        graphic.FillPath(&pbrush,&path);
    }
    
    DeleteObject(SelectObject(memdc,pre_brush));
    thumb_brush=CreateSolidBrush(RGB(55,55,55));
    pre_brush=(HBRUSH)SelectObject(memdc,thumb_brush);
    Rectangle(memdc,
              brc.left,brc.top,
              brc.right,brc.bottom);
    Rectangle(memdc,
              erc.left,erc.top,
              erc.right,erc.bottom);
    DeleteObject(thumb_brush);
    
    BitBlt(hdc,prc->left,prc->top,rc.right,rc.bottom,
           memdc,0,0,SRCCOPY);
    
    DeleteObject(SelectObject(memdc,pre_pen));
    DeleteObject(SelectObject(memdc,pre_brush));
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    ReleaseDC(hwnd,hdc);
    
    return 0;
}

int Edit_DrawVScrollBar(HWND hwnd)
{
    RECT rc={0};
    RECT thumb_rc={0},brc={0},erc={0};
    HDC hdc,memdc;
    HBITMAP bmp,pre_bmp;
    HBRUSH brush,pre_brush,thumb_brush;
    HPEN pen,pre_pen;
    COLORREF thumb_color;
    LPRECT prc=NULL;
    RECT rc_scroll[4]={0};
    pEditStyle es=Edit_GetSettings(hwnd);
    BOOL is_vscrollbar=Edit_IsShowVScroll(hwnd);
    
    if(!is_vscrollbar) return 0;
        
    if(0!=Edit_GetVScrollRect(hwnd,rc_scroll)) return 0;
    prc=&rc_scroll[0];
    
    hdc=GetWindowDC(hwnd);
    memdc=CreateCompatibleDC(hdc);
    CopyRect(&rc,prc);
    OffsetRect(&rc,-prc->left,-prc->top);
    bmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
    pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    brush=CreateSolidBrush(es->color_bk);
    pre_brush=(HBRUSH)SelectObject(memdc,brush);
    pen=CreatePen(PS_SOLID,1,es->color_bk);
    pre_pen=(HPEN)SelectObject(memdc,pen);
    
    Rectangle(memdc,rc.left,rc.top,rc.right,rc.bottom);
    
    thumb_color=RGB(45,45,45);
    CopyRect(&thumb_rc,prc+2);
    CopyRect(&brc,prc+1);
    CopyRect(&erc,prc+3);
    OffsetRect(&thumb_rc,-prc->left,-prc->top);
    OffsetRect(&brc,-prc->left,-prc->top);
    OffsetRect(&erc,-prc->left,-prc->top);
    DeleteObject(SelectObject(memdc,pre_brush));
    thumb_brush=CreateSolidBrush(thumb_color);
    pre_brush=(HBRUSH)SelectObject(memdc,thumb_brush);
            
    if(is_vscrollbar) {
        Graphics graphic(memdc);
        graphic.SetSmoothingMode(SmoothingModeHighQuality);
        GraphicsPath path;
        
        LinearGradientBrush pbrush(Rect(thumb_rc.left,thumb_rc.top,SCROLLBAR_PIXLS-2,SCROLLBAR_PIXLS-2),
                                  Color(255,15,15,15),Color(255,40,40,40),LinearGradientModeHorizontal);
        path.AddArc(thumb_rc.left,thumb_rc.top,SCROLLBAR_PIXLS-2,SCROLLBAR_PIXLS-2,
                    -180,180);
        path.AddArc(thumb_rc.left,thumb_rc.bottom-SCROLLBAR_PIXLS,SCROLLBAR_PIXLS-2,SCROLLBAR_PIXLS-2,
                    0,180);
        graphic.FillPath(&pbrush,&path);
    }
    
    DeleteObject(SelectObject(memdc,pre_brush));
    thumb_brush=CreateSolidBrush(es->color_bk);
    pre_brush=(HBRUSH)SelectObject(memdc,thumb_brush);
    Rectangle(memdc,
              brc.left,brc.top,
              brc.right,brc.bottom);
    Rectangle(memdc,
              erc.left,erc.top,
              erc.right,erc.bottom);
    DeleteObject(thumb_brush);
    
    BitBlt(hdc,prc->left,prc->top,rc.right,rc.bottom,
           memdc,0,0,SRCCOPY);
    
    DeleteObject(SelectObject(memdc,pre_pen));
    DeleteObject(SelectObject(memdc,pre_brush));
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    ReleaseDC(hwnd,hdc);
    
    return 0;
}

