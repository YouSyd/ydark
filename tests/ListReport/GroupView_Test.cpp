/*
GroupView改造
1.滚动条.
2.闪烁处理
*/
#include<windows.h>
#include<windowsx.h>
#include<math.h>
#include<stdio.h>
#include<commctrl.h>
#include<Richedit.h>
#include<gdiplus.h>

#include "../../include/listbox.h"
#pragma comment(lib,"../../lib/listbox.lib")

using namespace Gdiplus; 

//启用comctl32的visual style
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"gdiplus.lib")

int msgid=0;

typedef struct _STRUCT_GROUPVIEW_STYLE_ {
    COLORREF color_bk;
    COLORREF color_text;
    COLORREF color_border;
    HBRUSH brush;
    HBRUSH brush_border;
    HFONT font;
    WNDPROC proc;
    WNDPROC pre_proc;
    int margin_border;
    int font_cy;//字体高度,当创建字体时，会自动依据字体调整。
    
    int taborder;
    
    POINT dragpt;
    POINT dragpos;
    
    //放弃原有的index
    int index_Sel;
    int index_hover;
    
}RGroupStyle,*pGroupStyle;

int Group_InitialSettings(HWND hwnd);
pGroupStyle Group_GetSettings(HWND hwnd);
int Group_ClearSettings(HWND hwnd);
int Group_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Group_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL IsAllign_TopLeft);
LRESULT CALLBACK GroupOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK GroupOwnerProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    pGroupStyle gs=Group_GetSettings(hwnd);
    if(!gs) return 0;
    
    switch(message){
    case WM_NCHITTEST: {
    } break;
    //case TCM_HITTEST: {
    //} break;
    case WM_PAINT: {
        //Tab_Paint(hwnd,wParam,lParam);
        //return 0;
    } break;
    case WM_LBUTTONDOWN: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    } break;
    case WM_MOUSEMOVE: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    } break;
    case WM_SIZE: {
    } break;
    }
    
    return CallWindowProc(gs->pre_proc,hwnd,message,wParam,lParam);   
}

int Group_InitialSettings(HWND hwnd)
{
    pGroupStyle gs=(pGroupStyle)calloc(sizeof(RGroupStyle),1);
    if(!gs) return -1;
    
    gs->font_cy=16;
    gs->font=CreateFont(gs->font_cy,0,0,0,
                        FW_MEDIUM,//FW_SEMIBOLD,
                        FALSE,FALSE,FALSE,
                        DEFAULT_CHARSET,
                        OUT_OUTLINE_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, 
                        VARIABLE_PITCH,
                        "Courier New");
    SendMessage(hwnd,WM_SETFONT,(WPARAM)gs->font,NULL);
    
    gs->color_bk=RGB(15,15,15);
    gs->color_text=RGB(0,125,250);
    gs->brush=CreateSolidBrush(gs->color_bk);
    
    gs->margin_border=1;
    
    gs->color_border=RGB(70,70,70);
    gs->brush_border=CreateSolidBrush(gs->color_border);
    
    gs->proc=GroupOwnerProc;
    gs->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)gs->proc);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)gs);
    return 0;
}

pGroupStyle Group_GetSettings(HWND hwnd)
{
    return (pGroupStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int Group_ClearSettings(HWND hwnd)
{
    pGroupStyle gs=Group_GetSettings(hwnd);
    if(!gs) return -1;
    
    DeleteObject(gs->font);
    DeleteObject(gs->brush);
    DeleteObject(gs->brush_border);
    
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)gs->pre_proc);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,0);
    
    free(gs);
    return 0;
}

int Group_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam)
{    
    pGroupStyle gs=Group_GetSettings(hwnd);
    if(!gs) return -1;
    
    PAINTSTRUCT ps={0};
    BeginPaint(hwnd,&ps);
    
    HDC hdc=ps.hdc;
    HDC memdc=CreateCompatibleDC(hdc);
    RECT rcMem;
    CopyRect(&rcMem,&ps.rcPaint);
    OffsetRect(&rcMem,-rcMem.left,-rcMem.top);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,rcMem.right,rcMem.bottom);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    COLORREF color_bk=RGB(15,15,15);
    HBRUSH brush_bk=CreateSolidBrush(color_bk);
    FillRect(memdc,&rcMem,brush_bk);
    
    POINT pt_org;
    OffsetViewportOrgEx(memdc,-ps.rcPaint.left,-ps.rcPaint.top,&pt_org);
     
    SelectObject(memdc,(HFONT)SendMessage(hwnd,WM_GETFONT,0,0));
    SetBkMode(memdc,TRANSPARENT);
    SetTextColor(memdc,RGB(90,90,90));
    HBRUSH brush=CreateSolidBrush(RGB(30,30,30));
    HBRUSH brush_Sel=CreateSolidBrush(RGB(18,18,18));
    HPEN pen=CreatePen(PS_SOLID,1,RGB(53,53,53));
    SelectObject(memdc,brush);
    SelectObject(memdc,pen);
    
    DeleteObject(brush);
    DeleteObject(brush_Sel);
    DeleteObject(pen);    
    
    SetViewportOrgEx(memdc,pt_org.x,pt_org.y,NULL);
    
    BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,rcMem.right,rcMem.bottom,memdc,0,0,SRCCOPY);
    
    DeleteObject(brush_bk);
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    
    EndPaint(hwnd,&ps);
    return 0;
}

int Group_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL IsAllign_TopLeft)
{
    UINT style=GetWindowLongPtr(hwnd,GWL_STYLE);
    pGroupStyle gs=Group_GetSettings(hwnd);
    RECT rc,rc_client;
    
    if(!gs) return -1;
        
    GetWindowRect(hwnd,&rc);
    GetClientRect(hwnd,&rc_client);
    
    switch(zone) {
    case ZVSCROLL: {
    } break;
    case ZVSTHUMB: {  
    } break;
    }
    if(IsAllign_TopLeft) OffsetRect(prc,-rc.left,-rc.top);
    return 0;
}

/**********************************************************************************/
/********************************GroupView Test**************************************/
/**********************************************************************************/

#define WINDOW_CLASS_NAME "GroupViewTest"

HINSTANCE instance;

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int GroupView_Test(HWND parent);

#define IDC_GROUPVIEW_01 0x0001
#define IDC_LISTBOX_01 0x0002

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR szcmdLine, int icmdshow)
{
    HWND hwnd;
    MSG msg;
    WNDCLASSEX winclass;
    InitCommonControls();
    instance=hinstance;
    
    winclass.cbSize=sizeof(WNDCLASSEX);
    winclass.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
    winclass.lpfnWndProc=WindowProc;
    winclass.cbClsExtra=0;
    winclass.cbWndExtra=0;
    winclass.hInstance=hinstance;
    winclass.hIcon=LoadIcon(NULL, IDI_APPLICATION);
    winclass.hCursor=LoadCursor(NULL,IDC_ARROW);
    winclass.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
    winclass.lpszMenuName=NULL;
    winclass.lpszClassName=WINDOW_CLASS_NAME;
    winclass.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
    if(!RegisterClassEx(&winclass)) return 0;
        
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    if (!(hwnd=CreateWindowEx(NULL,WINDOW_CLASS_NAME,"GroupView test",WS_OVERLAPPEDWINDOW|WS_VISIBLE,\
        240,262,800,500,NULL,NULL,hinstance,NULL)))
    return 0;
    
    while(GetMessage(&msg,NULL,0,0)) {
        if(!IsDialogMessage(hwnd,&msg)) { //保证tabstop消息正常发送
    	    TranslateMessage(&msg);
    	    DispatchMessage(&msg);
        }
    }
    
    GdiplusShutdown(gdiplusToken);
    
    return(msg.wParam);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg){
	case WM_CREATE: {
	    GroupView_Test(hwnd);
	} break;
	case WM_SIZE: {
		int height=HIWORD(lParam);
		int width=LOWORD(lParam);
		//设置Z序
		int height_tab=200;
		SetWindowPos(GetDlgItem(hwnd,IDC_GROUPVIEW_01),HWND_BOTTOM,5,5,width-5*2,height-5-height_tab,SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hwnd,IDC_LISTBOX_01),HWND_BOTTOM,5,height-height_tab,width-5*2,height_tab-5,SWP_SHOWWINDOW);
	}break;
	case WM_DESTROY: {
		PostQuitMessage(0);
		return (0);
	} break;
	case WM_COMMAND: {
	    HWND ctrl=(HWND)lParam;
	    UINT code=HIWORD(wParam);
	} break;
	case WM_NOTIFY: {
	    LPNMHDR phdr=(LPNMHDR)lParam;
	    //switch(phdr->code) {   
	    //}
	} break;
	case WM_CTLCOLORLISTBOX: {
	    HDC hdc=(HDC)wParam;
	    HWND lb=(HWND)lParam;
	    
	    HWND gv=GetDlgItem(hwnd,IDC_GROUPVIEW_01);
	    pGroupStyle gs=Group_GetSettings(gv);
	    if(!gs) break;
	    SetTextColor(hdc,gs->color_text);
	    SetBkColor(hdc,gs->color_bk);
	    return (HRESULT)(gs->brush);
	} break;
	default: break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int GroupView_Test(HWND hwnd)
{
    HINSTANCE instance=(HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);
    HIMAGELIST ImgList=ImageList_Create(48,48,ILC_MASK|ILC_COLOR32,1,1);
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_1"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_2"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_3"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_4"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_5"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_6"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_7"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_8"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_9"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_a"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_b"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_c"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_d"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_e"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_f"));
    
    HWND gv=CreateWindowEx(NULL,WC_LISTVIEW,"",WS_TABSTOP|WS_CHILD|WS_VISIBLE/*|LVS_REPORT*/,
    					   0,0,400,300,hwnd,(HMENU)IDC_GROUPVIEW_01,
    					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
    Group_InitialSettings(gv);
    ListView_SetImageList(gv,ImgList,LVSIL_NORMAL);
    
    HIMAGELIST img_header=ImageList_Create(20,20,ILC_MASK|ILC_COLOR32,1,1);
    ImageList_AddIcon(img_header,LoadIcon(instance,"IDC_ICON_1"));
    ImageList_AddIcon(img_header,LoadIcon(instance,"IDC_ICON_2"));
    ImageList_AddIcon(img_header,LoadIcon(instance,"IDC_ICON_3"));
    ListView_SetImageList(gv,img_header,LVSIL_GROUPHEADER);
    
    ListView_SetBkColor(gv,RGB(25,25,25));
    ListView_SetTextBkColor(gv,RGB(25,25,25));
    ListView_SetTextColor(gv,RGB(150,150,150));
    
    SendMessage(gv, LVM_ENABLEGROUPVIEW, TRUE, 0);
    
    wchar_t title[][256]={L"第一组",L"第二组",L"第三组",L"这是一组footer"};    
    LVGROUP grp={ 0 };
    grp.cbSize=sizeof(grp);
    grp.mask=LVGF_STATE|LVGF_HEADER|LVGF_FOOTER|LVGF_GROUPID|LVGF_TITLEIMAGE;
    grp.iGroupId=0;
    grp.pszHeader=title[0];
    grp.cchHeader=256;
    grp.pszFooter=title[3];
    grp.cchFooter=256;
    grp.stateMask=LVGS_NORMAL;
    grp.state=LVGS_NORMAL;
    grp.iTitleImage=1;
    ListView_InsertGroup(gv,-1,&grp);
    
    grp.iGroupId=1;
    grp.pszHeader=title[1];
    grp.cchHeader=256;
    grp.iTitleImage=2;
    grp.stateMask=LVGS_COLLAPSIBLE;
    grp.state=LVGS_COLLAPSIBLE;
    ListView_InsertGroup(gv,-1,&grp);
    
    grp.iGroupId=2;
    grp.pszHeader=title[2];
    grp.cchHeader=256;
    grp.iTitleImage=0;
    ListView_InsertGroup(gv,-1,&grp);
    
    //创建项目
	char item_text[][256]={"例程",
	"介绍",
	"对接",
	"自动化任务",
	"数据管理",
	"Windows",
	"配置",
	"报表",
	"审计",
	"模组管理",
	"关联流程",
	"节点",
	"网络",
	"服务与应用",
	"更多..."};
	LVITEM lvi={0};
	for(int index=0;index<sizeof(item_text)/sizeof(char[256]);index++) {
    	lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_GROUPID;
    	lvi.pszText=item_text[index];
    	lvi.iItem=index;
    	lvi.iImage=index;
    	lvi.iGroupId=index%3;
    	SendMessage(gv,LVM_INSERTITEM,0,(LPARAM)&lvi);
    }
        
    HWND lb=CreateWindowEx(NULL,WC_LISTBOX, "",
    					   WS_TABSTOP|WS_CHILD|WS_VISIBLE|LBS_STANDARD,
    					   0, 0, 400, 300,
    					   hwnd, (HMENU)IDC_LISTBOX_01,
    					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
    ListBox_InitialSettings(lb);
    
    return 0;
}
