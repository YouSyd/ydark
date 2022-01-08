/*
ListBox改造
1.滚动条.
*/
#include "../../include/listbox.h"

#pragma comment(lib,"../../lib/listbox.lib")

/***********************************************************************/
/***********************   ListBox Test.     ***************************/
/***********************************************************************/
#define IDC_LISTBOX_01 0x0001
#define WINDOW_CLASS_NAME "ListGRID" 

HINSTANCE instance;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int ListBoxCtrl_Test(HWND parent);

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

	if (!(hwnd = CreateWindowEx(NULL,WINDOW_CLASS_NAME,"ListBox test",
	WS_OVERLAPPEDWINDOW|WS_VISIBLE,240,262,800,500,NULL,NULL,hinstance,NULL)))
    return 0;

	while(GetMessage(&msg,NULL,0,0)) {
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
        ListBoxCtrl_Test(hwnd);
	} break;
	case WM_SIZE: {
		int height = HIWORD(lparam);
		int width = LOWORD(lparam);
		//设置Z序
		SetWindowPos(GetDlgItem(hwnd, IDC_LISTBOX_01),HWND_BOTTOM,5,5,width-5*2,height-5*2,SWP_SHOWWINDOW);
		
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
	    }
	} break;
	case WM_NOTIFY: {
	
	} break;
	case WM_CTLCOLORLISTBOX: { //Edit 在只读模式ES_READONLY下发送该消息
	    HDC hdc=(HDC)wparam;
	    HWND lb=(HWND)lparam;
	    pLBStyle ls=ListBox_GetSettings(lb);
	    if(!ls) break;
	    SetTextColor(hdc,ls->color_text);
	    SetBkColor(hdc,ls->color_bk);
	    return (HRESULT)(ls->brush);
	} break;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int ListBoxCtrl_Test(HWND hwnd)
{
    INITCOMMONCONTROLSEX init={0};
    init.dwSize=sizeof(init); 
    init.dwICC=ICC_WIN95_CLASSES|ICC_STANDARD_CLASSES;  
    InitCommonControlsEx(&init);
    
    HWND lb=CreateWindowEx(NULL,
    					   WC_LISTBOX, "",
    					   WS_TABSTOP|WS_CHILD|WS_VISIBLE|LBS_STANDARD,
    					   0, 0, 400, 300,
    					   hwnd, (HMENU)IDC_LISTBOX_01,
    					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
    ListBox_InitialSettings(lb);
    
    for(int index=0;index<50;index++) {
        char buff[256]="";
        sprintf(buff,"<%05d> Row index %d",index,index);
        ListBox_AddString(lb,buff);
    }
    
    return 0;
}
