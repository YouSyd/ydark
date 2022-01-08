/*
DateTimePicker的基本情况：
1、可以显示日期和时间(DTS_TIMEFORMAT)
简单改造：
1、

注意：
1、
进一步：

*/
#include"../../include/datepick.h"

#include<math.h>
#include<stdio.h>
#include<gdiplus.h>
using namespace Gdiplus;

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib,"../../lib/datepick.lib")
#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib,"UxTheme.lib")

#define IDC_DATETIMEPICK_01 0x0001
#define IDC_DATETIMEPICK_02 0x0002
#define IDC_DATETIMEPICK_03 0x0003

#define WINDOW_CLASS_NAME "DatePick"
#define RWIDTH(A) abs(A.right - A.left)   
#define RHEIGHT(A) abs(A.bottom - A.top)  

HINSTANCE instance;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int DatePickCtrl_Test(HWND parent);

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR szcmdLine, int icmdshow)
{
	HWND hwnd;
	MSG msg;
	WNDCLASSEX winclass;
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
								"DATEPICK Test",
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
	    INITCOMMONCONTROLSEX icex={sizeof(icex),ICC_DATE_CLASSES};
        InitCommonControlsEx(&icex);
        DatePickCtrl_Test(hwnd);
	} break;
	case WM_SIZE: {
		int height = HIWORD(lparam);
		int width = LOWORD(lparam);
	}break;
	case WM_DESTROY: {
	    GdiplusShutdown(gdiplusToken);
		PostQuitMessage(0);
		return (0);
	} break;
	case WM_COMMAND: {
	    HWND ctrl=(HWND)lparam;
	    UINT code=HIWORD(wparam);
	} break;
	case WM_NOTIFY: {
        DatePick_Notify(hwnd,wparam,lparam);
	} break;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int DatePickCtrl_Test(HWND parent)
{
    HWND date1=CreateWindowEx(0,
                             DATETIMEPICK_CLASS,
                             "DateTime Test",
                             WS_CHILD|WS_VISIBLE|DTS_SHOWNONE,
                             5,5,180,25,
                             parent,
                             (HMENU)IDC_DATETIMEPICK_01,
                             (HINSTANCE)GetWindowLongPtr(parent,GWLP_HINSTANCE),
                             NULL);
    DatePick_InitialSettings(date1);
    
    HWND date2=CreateWindowEx(0,
                             DATETIMEPICK_CLASS,
                             "DateTime Test",
                             WS_CHILD|WS_VISIBLE|DTS_SHOWNONE,
                             5,35,180,25,
                             parent,
                             (HMENU)IDC_DATETIMEPICK_02,
                             (HINSTANCE)GetWindowLongPtr(parent,GWLP_HINSTANCE),
                             NULL);
    DatePick_InitialSettings(date2);
    
    HWND date3=CreateWindowEx(0,
                             DATETIMEPICK_CLASS,
                             "DateTime Test",
                             WS_CHILD|WS_VISIBLE|DTS_SHOWNONE,
                             5,65,180,25,
                             parent,
                             (HMENU)IDC_DATETIMEPICK_03,
                             (HINSTANCE)GetWindowLongPtr(parent,GWLP_HINSTANCE),
                             NULL);
    DatePick_InitialSettings(date3);
    
    return 0;
}