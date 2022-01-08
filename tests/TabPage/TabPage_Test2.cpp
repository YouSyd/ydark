/*
TabView改造
1.滚动条.
2.闪烁处理
*/
#include "../../include/tabpage.h"
#include "../../include/listbox.h"
#include "../../include/container.h"
#include "../../include/treegrid.h"
#pragma comment(lib,"../../lib/tabpage.lib")
#pragma comment(lib,"../../lib/listbox.lib")
#pragma comment(lib,"../../lib/treegrid.lib")
#pragma comment(lib,"../../lib/container.lib")

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int msgid=0;

/**********************************************************************************/
/********************************TabCtrl Test**************************************/
/**********************************************************************************/

#define WINDOW_TEST_NAME "TabCtrlTest"

HINSTANCE instance;
char szCmdLine[256]="";

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int TabCtrl_Test(HWND parent);

#define IDC_TabView_01 0x0001
#define IDC_LISTBOX_01 0x0002

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR szcmdLine, int icmdshow)
{
    HWND hwnd;
    MSG msg;
    WNDCLASSEX winclass;
    InitCommonControls();
    instance=hinstance;
    strcpy(szCmdLine,szcmdLine);
    
    winclass.cbSize=sizeof(WNDCLASSEX);
    winclass.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
    winclass.lpfnWndProc=WindowProc;
    winclass.cbClsExtra=0;
    winclass.cbWndExtra=0;
    winclass.hInstance=hinstance;
    winclass.hIcon=LoadIcon(NULL, IDI_APPLICATION);
    winclass.hCursor=LoadCursor(NULL,IDC_ARROW);//LoadCursor(instance,"IDC_CURSOR");
    winclass.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
    winclass.lpszMenuName=NULL;
    winclass.lpszClassName=WINDOW_TEST_NAME;
    winclass.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
    if(!RegisterClassEx(&winclass)) return 0;
        
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    if (!(hwnd=CreateWindowEx(NULL,WINDOW_TEST_NAME,"TabCtrl test",WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_BORDER,\
        240,262,800,500,NULL,NULL,hinstance,NULL)))
    return 0;
    
    MessageBox(hwnd,szCmdLine,WINDOW_TEST_NAME,MB_OK|MB_ICONINFORMATION);
    
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
    pFrameStyle fs=Frame_GetSettings(hwnd);
    LRESULT ret;
    if(fs&&fs->proc) ret=CallWindowProc(FrameProc,hwnd,msg,wParam,lParam);
	else ret=DefWindowProc(hwnd,msg,wParam,lParam);
	    
    switch (msg){
	case WM_CREATE: {
	    TabCtrl_Test(hwnd);
	} break;
	case WM_SIZE: {
		int height=HIWORD(lParam);
		int width=LOWORD(lParam);
		//设置Z序
		int height_tab=5;
		SetWindowPos(GetDlgItem(hwnd,IDC_TabView_01),NULL,0,0,width,height,SWP_SHOWWINDOW);
	}break;
	case WM_DESTROY: {
		PostQuitMessage(0);
		return (0);
	} break;
	case WM_COMMAND: {
	    HWND ctrl=(HWND)lParam;
	    UINT code=HIWORD(wParam);
	    if(code==EN_CHANGE) {
	    }
	} break;
	case WM_NOTIFY: {/*
	    LPNMHDR phdr=(LPNMHDR)lParam;
	    switch(phdr->code) {
	    case TCN_SELCHANGE: {// 当前选中项目TabCtrl_GetCurSel
	        int index_Sel= TabCtrl_GetCurSel(phdr->hwndFrom);
	        
	        TCITEM ti={0};
            ti.mask=TCIF_PARAM;
            TabCtrl_GetItem(hwnd,index_Sel,&ti);
	        pTabItem pti=(pTabItem)ti.lParam;
	        
	        UINT style=(UINT)GetWindowLongPtr(pti->hwnd,GWL_STYLE);
	        if((style&WS_VISIBLE)!=WS_VISIBLE) {
	            SetWindowLongPtr(pti->hwnd,GWL_STYLE,WS_VISIBLE|style);
	        }
	        RECT rc;
	        GetClientRect(hwnd,&rc);
	        rc.left+250;
	        SetWindowPos(pti->hwnd,NULL,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
	        //RECT rcItem;
	        //TabCtrl_GetItemRect(phdr->hwndFrom,index_Sel,&rcItem);
	    } break;
	    case TCN_SELCHANGING:// 当前选中项目将要变化
	        BOOL Stop_Change=FALSE;
	        int index_Sel= TabCtrl_GetCurSel(phdr->hwndFrom);
	        
	        TCITEM ti={0};
            ti.mask=TCIF_PARAM;
            TabCtrl_GetItem(hwnd,index_Sel,&ti);
	        pTabItem pti=(pTabItem)ti.lParam;
	        
	        UINT style=(UINT)GetWindowLongPtr(pti->hwnd,GWL_STYLE);
	        if((style&WS_VISIBLE)==WS_VISIBLE) {
	            SetWindowLongPtr(pti->hwnd,GWL_STYLE,(~WS_VISIBLE)&style);
	        }
	        return Stop_Change;//是否阻止改变
	    break;	    
	    }*/
	} break;
	case WM_CTLCOLORLISTBOX: {
	    HDC hdc=(HDC)wParam;
	    HWND lb=(HWND)lParam;
	    
	    HWND tb=GetDlgItem(hwnd,IDC_TabView_01);
	    pTabStyle ts=Tab_GetSettings(tb);
	    if(!ts) break;
	    SetTextColor(hdc,ts->color_text);
	    SetBkColor(hdc,ts->color_bk);
	    return (HRESULT)(ts->brush);
	} break;
	default: break;
    }
    
    return ret;
}

int TabCtrl_Test(HWND hwnd)
{
    Frame_InitialSettings(hwnd);
    pFrameStyle fs=Frame_GetSettings(hwnd);
    fs->proc=WindowProc;
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)WindowProc);

    HIMAGELIST ImgList=ImageList_Create(16,16, ILC_MASK|ILC_COLOR32,1,1);
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_1"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_2"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_3"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_4"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_5"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_6"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_7"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_8"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_9"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_a"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_b"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_c"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_d"));
    ImageList_AddIcon(ImgList,LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_e"));
    
    HWND tb=CreateWindowEx(NULL,WC_TABCONTROL,"",WS_TABSTOP|WS_CHILD|WS_VISIBLE|WS_BORDER,
    					   0,0,400,300,hwnd,(HMENU)IDC_TabView_01,
    					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
    Tab_InitialSettings(tb);
    pTabStyle ts=Tab_GetSettings(tb);
    if(strstr(szCmdLine,"left bar")) ts->tab_pos=TAB_LEFT;
    else if(strstr(szCmdLine,"top bar")) ts->tab_pos=TAB_TOP;
    TabCtrl_SetImageList(tb,ImgList);
    
    Frame_Register((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE));
    
    HWND page=NULL;
    TCITEM ti={0};
    char page_title[][256]={\
    "首选页-welcome!!!",
    "网络",
    "配置",
    "任务管理",
    "新功能",
    "查询管理",
    "new page.",
    "Page 8:为生民立命",
    "Page 9:为往圣继绝学",
    "Page 10:为万世开太平",
    "Page 11"};
    for(int index=0;index<sizeof(page_title)/sizeof(char[256]);index++) {
        
        page=CreateWindowEx(WS_EX_TOOLWINDOW,
             	  WINDOW_CLASS_NAME,page_title[index],
             	  WS_CHILD|WS_TABSTOP/*|WS_CLIPCHILDREN*/,//WS_CAPTION|WS_HSCROLL
             	  5,5,100,100,
             	  tb, (HMENU)NULL,//0x0001,
             	  (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        Frame_InitialSettings(page);
        SendMessage(page,WM_SETICON,(WPARAM)ICON_SMALL,(LPARAM)ImageList_GetIcon(ImgList,index,ILD_NORMAL));
        SetWindowLongPtr(page,GWL_STYLE,((~WS_VISIBLE)&(~WS_BORDER)&(UINT)GetWindowLongPtr(page,GWL_STYLE)));
    
        HWND lb=CreateWindowEx(NULL,WC_LISTBOX, "",
        					   WS_TABSTOP|WS_CHILD|WS_VISIBLE|LBS_STANDARD,
        					   5+index*5, 5, 400, 200,
        					   page,(HMENU)IDC_LISTBOX_01,
        					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
        ListBox_InitialSettings(lb);
        
        //HWND lb2=CreateWindowEx(NULL,WC_LISTBOX, "",
        //					   WS_TABSTOP|WS_CHILD|WS_VISIBLE|LBS_STANDARD,
        //					   5+index*5+250, 5, 400, 200,
        //					   tb,(HMENU)IDC_LISTBOX_01,
        //					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
        
        ti.mask=TCIF_TEXT|TCIF_IMAGE|TCIF_PARAM;
        ti.pszText=page_title[index];
        ti.cchTextMax=sizeof(page_title[index]);
        ti.iImage=index;
        pTabItem pti=(pTabItem)calloc(sizeof(RTabItem),1);
        
        pti->hwnd=page;
        pti->index=index;
        ti.lParam=(LPARAM)pti;
        TabCtrl_InsertItem(tb,index,&ti);
        TabCtrl_SetPadding(tb,15,1);
        RECT rect;
        GetClientRect(tb,&rect);
        
        TabCtrl_SetItemSize(tb,50,20);
        rect.left=rect.left+150;
        TabCtrl_AdjustRect(tb,TRUE,&rect);
        char text[256]="";
        int tab_count=TabCtrl_GetItemCount(tb);
        for(int index=0;index<tab_count;index++) {
            RECT rcTab;
            TabCtrl_GetItemRect(tb,index,&rcTab);
            
            sprintf(text,"<%05d> Item %d,RECT(%d,%d,%d,%d)",msgid++,index,rcTab.left,rcTab.top,rcTab.right,rcTab.bottom);
            ListBox_SetTopIndex(lb,ListBox_AddString(lb,text));
            //ListBox_SetTopIndex(lb2,ListBox_AddString(lb2,text));
        }
    }
    return 0;
}
