#include "../../include/listreport.h"
#pragma comment(lib,"../../lib/listreport.lib")

#define ID_LISTREPORT 0x00000001
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow);
void regist_window(char* class_name,WNDPROC proc,HINSTANCE instance);
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
void command_proc(HWND hwnd,WPARAM wParam,LPARAM lParam);
void notify_proc(HWND hwnd,WPARAM wParam,LPARAM lParam);
int InitialWindow(HWND hwnd);
int ListReport_Test(HWND hwnd);

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
    char szClassName[]="ListReport test";
    HWND hwnd;//main hwnd;
    MSG msg={0};
    DWORD pre_tickcount;
    
    HMENU menu=LoadMenu(hInstance,"IDR_MAINFRAME");
    
    regist_window(szClassName,MainWndProc,hInstance);
    hwnd = CreateWindowEx(0,szClassName,szClassName,
                          WS_THICKFRAME|WS_CAPTION|WS_OVERLAPPEDWINDOW|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CLIPCHILDREN,
                          500,200,
                          450,450,
                          NULL,
                          menu,
                          hInstance,
                          NULL);
    if(hwnd == NULL) {
        MessageBox(NULL,"create windows error !","error",MB_OK);
        return -1;
    }

    ShowWindow(hwnd,nCmdShow);
    UpdateWindow(hwnd);
        
    pre_tickcount=GetTickCount();
    while (msg.message!=WM_QUIT) {
        if(PeekMessage(&msg,0,0,0,PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } 
    }

    return (int) msg.wParam;
}

void regist_window(char* class_name,WNDPROC proc,HINSTANCE instance)
{
    WNDCLASSEX wndclass;

    wndclass.cbSize=sizeof(wndclass);
    wndclass.style = CS_HREDRAW|CS_VREDRAW;
    wndclass.lpfnWndProc = proc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = instance;
    wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(250,250,250));
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = class_name;
    wndclass.hIconSm = NULL;

    RegisterClassEx(&wndclass);
}

LRESULT CALLBACK MainWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    switch(message) {
        case WM_CREATE: {
            InitialWindow(hwnd);
        } break;
        case WM_SIZE: {
            int width=LOWORD(lParam);
            int height=HIWORD(lParam);
            
            HWND report=GetDlgItem(hwnd,ID_LISTREPORT);
            MoveWindow(report,5,5,width-10,height-10,TRUE);
        } break;
        case WM_COMMAND: {
            command_proc(hwnd,wParam,lParam);
        } break;
        case WM_NOTIFY: {
            notify_proc(hwnd,wParam,lParam);
        } break;        
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        } break;
    }
    return DefWindowProc(hwnd,message,wParam,lParam);   
}

void command_proc(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    UINT item_id=(UINT)LOWORD(wParam);
    
    switch(item_id) {
        case 2001:
        {

        }
        break;
    }
    InvalidateRect(hwnd,NULL,FALSE);
}

void notify_proc(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    HWND ctrl=((LPNMHDR)lParam)->hwndFrom;
    UINT ctrl_id=((LPNMHDR)lParam)->idFrom;
    UINT code=((LPNMHDR)lParam)->code;
    
    if(ctrl==GetDlgItem(hwnd,ID_LISTREPORT)) {
        switch(code){
        case NM_CLICK: {
            LPNMITEMACTIVATE lpitem=(LPNMITEMACTIVATE)lParam;
            int item_index=lpitem->iItem;
            int subitem_index=lpitem->iSubItem;
            if(item_index<0) break;
                
            char message[256]="";
            sprintf(message,"ListView/Report Item click:点选了%d Item。",item_index);
            MessageBox(hwnd,message,"Item click test",MB_OK|MB_ICONINFORMATION);
        } break;
        }
    }
}

int InitialWindow(HWND hwnd)
{
    ListReport_Test(hwnd);
    return 0;
}

int ListReport_Test(HWND hwnd)
{
    LVCOLUMN lvc={0};
    LVITEM lvi={0};
    HWND ListReport;
    HIMAGELIST ImgList;
    
    ListReport=CreateWindowEx(WS_EX_STATICEDGE,TEXT("SysListView32"),NULL,WS_CHILD|WS_BORDER|WS_VISIBLE|LV_VIEW_ICON|LVS_NOSCROLL,\
    10, 10, 400, 400, hwnd,(HMENU)ID_LISTREPORT,(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
    Report_InitialSettings(ListReport);
    
    HFONT pre_font=(HFONT)SendMessage(ListReport,WM_GETFONT,0,0);
    HFONT font;
    LOGFONT lf={0};
    GetObject(pre_font,sizeof(LOGFONT),&lf);
    strcpy(lf.lfFaceName,"Courier New");
    lf.lfHeight=14;
    font=CreateFontIndirect(&lf);
    SendMessage(ListReport, WM_SETFONT, (WPARAM)font, 0);

    //创建图片列表
    ImgList=ImageList_Create(48,48,ILC_MASK|ILC_COLOR32,1,1);
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_1"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_2"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_3"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_4"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_5"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_6"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_7"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_8"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_9"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_a"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_b"));
    ImageList_AddIcon(ImgList, LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_c"));
    ListView_SetImageList(ListReport,ImgList,LVSIL_NORMAL);
    
    //观察效果
    ListView_SetBkColor(ListReport,RGB(25,25,25));
    ListView_SetTextBkColor(ListReport,RGB(160,160,160));
    ListView_SetTextColor(ListReport,RGB(150,150,150));
	//创建列
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT;//掩码
	lvc.fmt=LVCFMT_LEFT;//左对齐
	lvc.cx=100;//列宽
	lvc.pszText=TEXT("Column-1");
	SendMessage(ListReport, LVM_INSERTCOLUMN,0,(LPARAM)&lvc);//创建列
	lvc.pszText=TEXT("Column-2");
	lvc.cx=200;
	SendMessage(ListReport,LVM_INSERTCOLUMN,1,(LPARAM)&lvc);
	lvc.pszText=TEXT("Column-3");
	SendMessage(ListReport,LVM_INSERTCOLUMN,2,(LPARAM)&lvc);
	lvc.pszText=TEXT("Column-4");
	SendMessage(ListReport,LVM_INSERTCOLUMN,3,(LPARAM)&lvc);
 
	//创建项目
	lvi.mask=LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT;
	lvi.pszText=TEXT("例程");
	lvi.iItem=0;//项目号
	lvi.iImage=0;//图片号
	lvi.iIndent=0;
	SendMessage(ListReport,LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("介绍");
	lvi.iItem=1;
	lvi.iImage=1;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("对接");
	lvi.iItem=2;
	lvi.iImage=2;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("自动化任务");
	lvi.iItem=3;
	lvi.iImage=3;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("数据管理");
	lvi.iItem=4;
	lvi.iImage=4;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("Windows");
	lvi.iItem=5;
	lvi.iImage=5;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("配置");
	lvi.iItem=6;
	lvi.iImage=6;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("报表");
	lvi.iItem=7;
	lvi.iImage=7;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("审计");
	lvi.iItem=8;
	lvi.iImage=8;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("模组管理");
	lvi.iItem=9;
	lvi.iImage=9;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("关联流程");
	lvi.iItem=10;
	lvi.iImage=10;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=TEXT("节点");
	lvi.iItem=11;
	lvi.iImage=11;
	SendMessage(ListReport, LVM_INSERTITEM,0,(LPARAM)&lvi);
 
	//创建子项
	lvi.mask=LVIF_TEXT | LVIF_IMAGE;
	lvi.iItem=1;
	lvi.iSubItem=1;
	lvi.iImage=-1;
	lvi.pszText=TEXT("子项");
	SendMessage(ListReport, LVM_SETITEM,0,(LPARAM)&lvi);
	lvi.iItem = 1;
	lvi.iSubItem = 2;
	lvi.pszText = TEXT("子项");
	SendMessage(ListReport, LVM_SETITEM,0,(LPARAM)&lvi);
	lvi.iItem = 1;
	lvi.iSubItem = 3;
	lvi.pszText = TEXT("子项");
	SendMessage(ListReport, LVM_SETITEM,0,(LPARAM)&lvi);

    return 0;
}