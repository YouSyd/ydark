#include "../../include/statusbar.h"

#pragma comment(lib,"../../lib/statusbar.lib")

#define ID_STATUSBAR_01 0x0001

#define WINDOW_CLASS_NAME "Status bar Test"
#define RWIDTH(A)   abs(A.right - A.left)   
#define RHEIGHT(A)  abs(A.bottom - A.top)  

HINSTANCE instance;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

LRESULT CALLBACK WindowProc(HWND,UINT,WPARAM,LPARAM);
 
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR szcmdLine, int icmdshow)
{
    HWND hwnd;
    MSG msg;
    WNDCLASSEX winclass;
    InitCommonControls();
    instance=hinstance;
     
    winclass.cbSize=sizeof(WNDCLASSEX);
    winclass.style=CS_HREDRAW|CS_VREDRAW;//NULL;//CS_DBLCLKS
    winclass.lpfnWndProc=WindowProc;
    winclass.cbClsExtra=0;
    winclass.cbWndExtra=0;
    winclass.hInstance=hinstance;
    winclass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    winclass.hCursor=LoadCursor(NULL,IDC_ARROW);
    winclass.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
    winclass.lpszMenuName=NULL;
    winclass.lpszClassName=WINDOW_CLASS_NAME;
    winclass.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
 
    if(!RegisterClassEx(&winclass)) return (0);
    
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
           
    if(!(hwnd = CreateWindowEx(NULL,
                               WINDOW_CLASS_NAME,
                               "Statusbar Test",
                               WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                               240,262,
                               800,500,
                               NULL, 
                               NULL, 
                               hinstance, 
                               NULL)))
        return (0);
    
    while(GetMessage(&msg,NULL,0,0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    GdiplusShutdown(gdiplusToken);
    
    return(msg.wParam);
}
 
LRESULT CALLBACK WindowProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg) {
        case WM_CREATE: {
             
            HWND statusbar=CreateWindowEx(0,
            STATUSCLASSNAME,NULL,
            WS_CHILD|WS_VISIBLE|SBARS_SIZEGRIP,
            20,40,400,150,
            hwnd,(HMENU)ID_STATUSBAR_01,
            (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),NULL);                       
            
            static char txtItem[][256]={"Item 1:第一个status item",
                                "Item 2:第二个Status item.",
                                "Item 3:第三个status_item.",
                                "最后一个item,这个item得长点，用于测试，当然不能超过256个字符，否则需要修改结构。"};
            static int coordItem[]={150,150,100,250};
            
            int idx=0,count=sizeof(txtItem)/sizeof(char[256]);
            for(;idx<count;idx++) {
                coordItem[idx]=(idx==0?0:coordItem[idx-1])+coordItem[idx];
            }
            coordItem[idx]=-1;
            
            SendMessage(statusbar,SB_SETPARTS,(WPARAM)count,(LPARAM)&coordItem);
            
            for(idx=0;idx<count;idx++) {
                SendMessage(statusbar,SB_SETTEXT,(WPARAM)idx,(LPARAM)TEXT(txtItem[idx]));
            }
            StatusBar_InitialSettings(statusbar);
        } break;
        case WM_SIZE: {
            UINT width=LOWORD(lParam);
            UINT height=HIWORD(lParam);
            HWND stabar=GetDlgItem(hwnd,ID_STATUSBAR_01);
            RECT rcStabar;
            GetWindowRect(stabar,&rcStabar);
            OffsetRect(&rcStabar,-rcStabar.left,-rcStabar.top);
            MoveWindow(stabar,0,height-rcStabar.bottom,width,rcStabar.bottom,TRUE);
        } break;
        case WM_DESTROY: {
            PostQuitMessage(0);
            return (0);
        } break;
    }
    return DefWindowProc(hwnd,msg,wParam,lParam);
}