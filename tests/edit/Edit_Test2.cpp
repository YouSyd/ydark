#include "../../include/Edit.h"

#pragma comment(lib,"../../lib/edit.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WINDOW_CLASS_NAME "Edit_Test"

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

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
HINSTANCE instance;

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR szcmdLine, int icmdshow);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
int EditCtrl_Test(HWND hwnd);
int MultiScrollTest(HWND hwnd);

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR szcmdLine, int icmdshow)
{
	HWND hwnd;
	MSG msg;
	WNDCLASSEX winclass;
	InitCommonControls();
	instance = hinstance;

	winclass.cbSize=sizeof(WNDCLASSEX);
	winclass.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
	winclass.lpfnWndProc=WindowProc;
	winclass.cbClsExtra=0;
	winclass.cbWndExtra=0;
	winclass.hInstance=hinstance;
	winclass.hIcon=LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor=LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName=NULL;
	winclass.lpszClassName=WINDOW_CLASS_NAME;
	winclass.hIconSm=LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&winclass)) return 0;

	if(!(hwnd=CreateWindowEx(NULL,WINDOW_CLASS_NAME,"Edit test",\
	    WS_OVERLAPPEDWINDOW|WS_VISIBLE,240,262,800,500,NULL,NULL,hinstance,NULL)))
		return 0;

	while(GetMessage(&msg, NULL, 0, 0)) {
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
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT_0A),HWND_BOTTOM,760,5,400,500,SWP_SHOWWINDOW|SWP_FRAMECHANGED);
		
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
	        if(ES_MULTILINE==(ES_MULTILINE&(UINT)GetWindowLongPtr(ctrl,GWL_STYLE))) {
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


int EditCtrl_Test(HWND hwnd)
{    
    HWND edit1 = CreateWindowEx(NULL,
    						   WC_EDIT, "",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_01,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit1,"单行测试","");
    wchar_t string_buffer[256]={0};
    swprintf(string_buffer,L"%s",L"请输入文本");
    Edit_SetCueBannerText(edit1,string_buffer);//需要 enable visual style,需要编译器指示
    
    HWND edit2 = CreateWindowEx(NULL,
    						   WC_EDIT, "Edit control test.",
    						   WS_TABSTOP|WS_CHILD|WS_VISIBLE,
    						   0, 0, 100, 100,
    						   hwnd, (HMENU)IDC_EDIT_02,
    						   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    Edit_InitialSettings(edit2,"标题","测试Text标题");
    Edit_SetInternalStyle(edit2,2);
    swprintf(string_buffer,L"%s",L"请输浮点数值");
    Edit_SetCueBannerText(edit2,string_buffer);
    
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
    swprintf(string_buffer,L"%s",L"请输入整型数值");
    Edit_SetCueBannerText(edit5,string_buffer);
    
    HWND edit6 = CreateWindowEx(NULL,
    						   WC_EDIT, "",
    						   WS_TABSTOP/*|WS_BORDER*/|WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_WANTRETURN/*|ES_AUTOVSCROLL*/|WS_VSCROLL,
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