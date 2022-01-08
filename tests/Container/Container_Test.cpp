/*
author: younth
date: 2021年3月30日
////
bug:
container 作为子控件拖拽的时候刷新重绘一团乱麻
*/
#include "../../include/container.h"
#include "../../include/TreeGrid.h"
#include "../../include/ListBox.h"
#include "../../include/TabPage.h"
#include "../../include/ListReport.h"
#include "../../include/ListView.h"
#include "../../include/statusbar.h"
#pragma comment(lib,"../../lib/container.lib")
#pragma comment(lib,"../../lib/TreeGrid.lib")
#pragma comment(lib,"../../lib/ListBox.lib")
#pragma comment(lib,"../../lib/TabPage.lib")
#pragma comment(lib,"../../lib/ListReport.lib")
#pragma comment(lib,"../../lib/ListView.lib")
#pragma comment(lib,"../../lib/statusbar.lib")
 
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define IDC_BUTTON_01 0x0101
#define IDC_BUTTON_02 0x0102
#define IDC_STATIC_01 0x0103
#define IDC_LISTBOX_01 0x0104
#define IDC_TREEGRID 0x0005
#define IDC_TabView_01 0x0006
#define ID_LISTREPORT_01 0x0007
#define IDC_LISTVIEW_01 0x0008
#define IDC_STATUSBAR_01 0x0009

HINSTANCE instance;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

int Frame_Register(HINSTANCE instance);
HWND Frame_Create(HINSTANCE instance,int width,int height);
int Frame_Run(HWND hwnd);
LRESULT CALLBACK TestProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int Frame_Test(HWND frame);
int TabCtrl_Test(HWND hwnd);
int ListReport_Test(HWND hwnd);
int ListView_Test(HWND hwnd);

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR szcmdLine, int icmdshow)
{
	HWND hwnd=NULL;
	
	InitCommonControls();
	instance=hinstance;
	
	if(Frame_Register(hinstance)!=0) {
	    MessageBox(NULL,"窗体注册失败","异常",MB_OK|MB_ICONERROR);
	    return 0;
    }
    if(NULL==(hwnd=Frame_Create(hinstance,1200,600))) return 0;
    
    Frame_Test(hwnd);
    
    return Frame_Run(hwnd);
}

int regist_window(char* class_name,HINSTANCE instance,WNDPROC proc)
{
    WNDCLASSEX wndclass;

    wndclass.cbSize=sizeof(wndclass);
    wndclass.style = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
    wndclass.lpfnWndProc = proc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = instance;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0,0,0));
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = class_name;
    wndclass.hIconSm = NULL;

    if(!RegisterClassEx(&wndclass)) return -1;
    return 0;
}

HWND Frame_Create(HINSTANCE instance,int width,int height)
{
    int cx,cy;
    POINT pt;
    
    cx=GetSystemMetrics(SM_CXSCREEN);
    cy=GetSystemMetrics(SM_CYSCREEN);
    
    pt.x=(cx-width)>>1;
    pt.y=(cy-height)>>1;
    
    HWND hwnd=CreateWindowEx(NULL,
	                         WINDOW_CLASS_NAME,
	                         "Docker Frame Capture",
	                         WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_HSCROLL|WS_CLIPCHILDREN,
							 pt.x,pt.y,width,height,NULL,
							 LoadMenu(instance,"IDR_FRMENU"),
							 instance,NULL);
    
	return hwnd;
}

int Frame_Run(HWND frame)
{
    MSG msg;

    while(GetMessage(&msg,NULL,0,0)) {
	    if(!IsDialogMessage(frame,&msg)) { //tabstop
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	    }
	}
    
    GdiplusShutdown(gdiplusToken);
    
	return(msg.wParam);
}

LRESULT CALLBACK TestProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    pFrameStyle fs=Frame_GetSettings(hwnd);
    LRESULT ret;
    if(fs&&fs->proc) ret=CallWindowProc(FrameProc,hwnd,msg,wParam,lParam);
	else ret=DefWindowProc(hwnd,msg,wParam,lParam);
	    
	if(msg==WM_DESTROY) {
	    PostQuitMessage(0);
	    return 0;
	}
    else if(msg==WM_SIZE) {	
        int height = HIWORD(lParam);
	    int width = LOWORD(lParam);
	    pFrameStyle fs=Frame_GetSettings(hwnd);
	    
        //更新高宽
        RECT rc_client;
        GetClientRect(hwnd,&rc_client);
        height=rc_client.bottom-rc_client.top;
        width=rc_client.right-rc_client.left;
        int height_statusbar=20;
        
		if(GetDlgItem(hwnd,IDC_LISTBOX_01)) {
	        SetWindowPos(GetDlgItem(hwnd,IDC_LISTBOX_01),NULL,3,3,width-5,height-5,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
	    }
		if(GetDlgItem(hwnd,IDC_TREEGRID)) {
	        SetWindowPos(GetDlgItem(hwnd,IDC_TREEGRID),NULL,0,0,width,height,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
	    }
	    if(GetDlgItem(hwnd,IDC_TabView_01)) {
	        SetWindowPos(GetDlgItem(hwnd,IDC_TabView_01),NULL,0,0,width,height,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
	    }
	    if(GetDlgItem(hwnd,ID_LISTREPORT_01)) {
	        SetWindowPos(GetDlgItem(hwnd,ID_LISTREPORT_01),NULL,0,0,width,height,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
	    }
	    if(GetDlgItem(hwnd,IDC_STATUSBAR_01)) {
	        SetWindowPos(GetDlgItem(hwnd,IDC_STATUSBAR_01),NULL,0,0,width,height,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
	    }
    }
	return ret;
}

LRESULT CALLBACK TabContentSize(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    pFrameStyle fs=Frame_GetSettings(hwnd);
    LRESULT ret;
    
    if(msg==WM_CLOSE) {
        if(IDYES==MessageBox(hwnd,"Are you sure to close page?","提示",MB_ICONQUESTION|MB_YESNO)) {
            if(fs&&fs->proc) CallWindowProc(FrameProc,hwnd,msg,wParam,lParam);
	        else DefWindowProc(hwnd,msg,wParam,lParam);
            return 0;
        }
        return 1;
    }
    else if(msg==WM_MEASUREITEM) {
        LPMEASUREITEMSTRUCT lpmis=(LPMEASUREITEMSTRUCT)lParam;
        if(lpmis->CtlType == ODT_LISTVIEW) {
            SendMessage(GetDlgItem(hwnd,lpmis->CtlID),WMYU_MEASURELISTVIEWITEM,wParam,lParam);
        }
    }
    
    if(fs&&fs->proc) ret=CallWindowProc(FrameProc,hwnd,msg,wParam,lParam);
	else ret=DefWindowProc(hwnd,msg,wParam,lParam);
    
    if(msg==WM_SIZE) {	
        int height=HIWORD(lParam);
	    int width=LOWORD(lParam);	    
        
        //更新高宽
	    if(GetDlgItem(hwnd,ID_LISTREPORT_01)) {
	        SetWindowPos(GetDlgItem(hwnd,ID_LISTREPORT_01),NULL,0,0,width,height,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
	    }
	    else if(GetDlgItem(hwnd,IDC_LISTVIEW_01)) {
	        SetWindowPos(GetDlgItem(hwnd,IDC_LISTVIEW_01),NULL,0,0,width,height,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
	    }
	    else {
	        HWND child=GetWindow(hwnd,GW_CHILD);
	        if(child) {
	            SetWindowPos(child,NULL,0,0,width,height,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
	        }
	    }
    }
	return ret;
}

int Frame_Test(HWND frame)
{
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    Frame_InitialSettings(frame);
    pFrameStyle fs=Frame_GetSettings(frame);
    HWND hint_hwnd=Hint_Create((HINSTANCE)GetWindowLongPtr(frame,GWLP_HINSTANCE),/*DOCK_ALDNXB*/DOCK_ALTLTRTTRBB);
    if(hint_hwnd==NULL) MessageBox(frame,"创建Hint窗体失败","异常",MB_OK|MB_ICONINFORMATION);
    Frame_SetHintWindow(frame,hint_hwnd);
    Frame_AllignHintWindow(frame);
    pHintInfo phint=Hint_GetSettings(hint_hwnd);
    SendMessage(frame,WM_SETFONT,(WPARAM)fs->font,0);
        
    HWND docker=CreateWindowEx(WS_EX_TOOLWINDOW,
             		   WINDOW_CLASS_NAME, "Docker window",
             		   WS_CAPTION|WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_CLIPCHILDREN,//WS_TABSTOP|WS_VISIBLE,
             		   15,15,350,170,
             		   frame, (HMENU)NULL,//0x0001,
             		   (HINSTANCE)GetWindowLongPtr(frame,GWLP_HINSTANCE), NULL);
    SetWindowLongPtr(docker,GWL_STYLE,(~WS_MAXIMIZEBOX)&GetWindowLongPtr(docker,GWL_STYLE));
    Frame_InitialSettings(docker);
    Frame_InitialViewSI(docker);
    Frame_SetViewHeight(docker,1500);
    SendMessage(docker,WM_SETICON,(WPARAM)ICON_SMALL,(LPARAM)LoadIcon((HINSTANCE)GetWindowLongPtr(docker,GWLP_HINSTANCE),"IDC_ICON_DOCKER"));
    phint->pos=ALTLTRTTRBB_TL;
    phint->hintdock(frame,docker,hint_hwnd);
    
    HWND docker2=CreateWindowEx(WS_EX_TOOLWINDOW,
             		   WINDOW_CLASS_NAME, "Docker window2",
             		   WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,//WS_TABSTOP|WS_VISIBLE,
             		   15,15,350,170,
             		   frame, (HMENU)NULL,//0x0001,
             		   (HINSTANCE)GetWindowLongPtr(frame, GWLP_HINSTANCE), NULL);
    SetWindowLongPtr(docker2,GWL_STYLE,(~WS_MAXIMIZEBOX)&GetWindowLongPtr(docker2,GWL_STYLE));
    Frame_InitialSettings(docker2);
    phint->pos=ALTLTRTTRBB_TRT;
    phint->hintdock(frame,docker2,hint_hwnd);
    fs=Frame_GetSettings(docker2);
    fs->proc=TestProc;
    SetWindowLongPtr(docker2,GWLP_WNDPROC,(LONG_PTR)TestProc);
    TabCtrl_Test(docker2);
    
    HWND docker3=CreateWindowEx(WS_EX_TOOLWINDOW,
             		   WINDOW_CLASS_NAME, "Docker window3",
             		   WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|WS_CLIPCHILDREN,//WS_CAPTION|WS_HSCROLL|
             		   15,15,350,170,
             		   frame, (HMENU)NULL,//0x0001,
             		   (HINSTANCE)GetWindowLongPtr(frame, GWLP_HINSTANCE), NULL);
    Frame_InitialSettings(docker3);
    phint->pos=ALTLTRTTRBB_TRB;
    phint->hintdock(frame,docker3,hint_hwnd);
    fs=Frame_GetSettings(docker3);
    fs->proc=TestProc;
    SetWindowLongPtr(docker3,GWLP_WNDPROC,(LONG_PTR)TestProc);
       
    HWND hmsg=CreateWindowEx(NULL,
             		   WC_LISTBOX, "Message Logger",
             		   WS_CHILD|WS_VISIBLE|LBS_STANDARD|LBS_HASSTRINGS|WS_VSCROLL|WS_BORDER,
             		   0,0,350,170,
             		   docker,(HMENU)IDC_LISTBOX_01,
             		   (HINSTANCE)GetWindowLongPtr(frame, GWLP_HINSTANCE), NULL);
    ListBox_InitialSettings(hmsg);
    
    SendMessage(hmsg,WM_SETFONT,(WPARAM)Frame_GetSettings(docker)->font,0);
    fs=Frame_GetSettings(docker);
    fs->proc=TestProc;
    SetWindowLongPtr(docker,GWLP_WNDPROC,(LONG_PTR)TestProc);
    
    regist_window(YCL_TREEGRID,(HINSTANCE)GetWindowLongPtr(frame,GWLP_HINSTANCE),TreeGridOwnProc);
    CreateWindowEx(0,YCL_TREEGRID,YCL_TREEGRID,
                   WS_CHILD|WS_VISIBLE|WS_BORDER,
                   0,0,1000,450,docker3,(HMENU)IDC_TREEGRID,
                   (HINSTANCE)GetWindowLongPtr(frame,GWLP_HINSTANCE),NULL);
                   
    HWND docker4=CreateWindowEx(WS_EX_TOOLWINDOW,
             		   WINDOW_CLASS_NAME, "Docker window4",
             		   WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|WS_CLIPCHILDREN,//WS_CAPTION|WS_HSCROLL|
             		   15,15,350,170,
             		   frame, (HMENU)NULL,//0x0001,
             		   (HINSTANCE)GetWindowLongPtr(frame, GWLP_HINSTANCE), NULL);
    Frame_InitialSettings(docker4);
    phint->pos=ALTLTRTTRBB_B;
    phint->hintdock(frame,docker4,hint_hwnd);
    fs=Frame_GetSettings(docker4);
    fs->proc=TestProc;
    SetWindowLongPtr(docker4,GWLP_WNDPROC,(LONG_PTR)TestProc);
    HWND statusbar=CreateWindowEx(0,
    STATUSCLASSNAME,NULL,
    WS_CHILD|WS_VISIBLE|SBARS_SIZEGRIP,
    20,40,400,150,
    docker4,(HMENU)IDC_STATUSBAR_01,
    (HINSTANCE)GetWindowLongPtr(docker4,GWLP_HINSTANCE),NULL);                       
    
    static char txtItem[][256]={"Item 1:第一个status item",
                        "Item 2:第二个Status item.",
                        "Item 3:第三个status_item.",
                        "最后一个item,这个item得长点，用于测试，当然不能超过256个字符，否则需要修改结构。"};
    static int coordItem[]={250,180,120,250};
    
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
    
    //调整尺寸
    RECT rc;
    GetWindowRect(frame,&rc);
    InflateRect(&rc,-5,-5);
    MoveWindow(frame,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,TRUE);
        
    return 0;
}

int msgid=0;
int TabCtrl_Test(HWND hwnd)
{
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
    
    HWND tb=CreateWindowEx(NULL,WC_TABCONTROL,"",WS_TABSTOP|WS_CHILD|WS_VISIBLE|WS_BORDER/*|TCS_OWNERDRAWFIXED*/,
    					   0,0,400,300,hwnd,(HMENU)IDC_TabView_01,
    					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
    Tab_InitialSettings(tb);
    pTabStyle ts=Tab_GetSettings(tb);
    ts->tab_pos=TAB_TOP;
    TabCtrl_SetImageList(tb,ImgList);
    
    HWND page=NULL;
    TCITEM ti={0};
    char page_title[][256]={\
    "Page Item 1:这是一个比较长的标签",
    "Page 2",
    "Page 3",
    "Page 4",
    "Page 5:为万世开太平",
    "Page 6",
    "Page 7:为天地立心",
    "Page 8:为生民立命",
    "Page 9:为往圣继绝学",
    "Page 10:为万世开太平",
    "Page 11"};
    for(int index=0;index<sizeof(page_title)/sizeof(char[256]);index++) {
        page=CreateWindowEx(WS_EX_TOOLWINDOW,
             	  WINDOW_CLASS_NAME,page_title[index],
             	  WS_CHILD|WS_TABSTOP,//WS_CAPTION|WS_HSCROLL|WS_CLIPCHILDREN
             	  5,5,100,100,
             	  tb, (HMENU)NULL,//0x0001,
             	  (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        Frame_InitialSettings(page);
        SendMessage(page,WM_SETICON,(WPARAM)ICON_SMALL,(LPARAM)ImageList_GetIcon(ImgList,index,ILD_NORMAL));
        SetWindowLongPtr(page,GWL_STYLE,((~WS_VISIBLE)&(~WS_BORDER)&(UINT)GetWindowLongPtr(page,GWL_STYLE)));
        
        pFrameStyle fs=Frame_GetSettings(page);
        fs->proc=TabContentSize;
        SetWindowLongPtr(page,GWLP_WNDPROC,(LONG_PTR)TabContentSize);
        if(index==0) {
            ListReport_Test(page);
        }
        else if(index==1) {      
            ListView_Test(page);
        }
        else {
            HWND lb=CreateWindowEx(NULL,WC_LISTBOX, "",
            					   WS_TABSTOP|WS_CHILD|WS_VISIBLE|LBS_STANDARD,
            					   5+index*5, 5, 400, 200,
            					   page,(HMENU)IDC_LISTBOX_01,
            					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
            ListBox_InitialSettings(lb);
            SetWindowLongPtr(lb,GWL_STYLE,((~WS_BORDER)&(UINT)GetWindowLongPtr(lb,GWL_STYLE)));
            char text[256]="";
            for(int i=0;i<50;i++) {
                RECT rcTab;
                TabCtrl_GetItemRect(tb,i,&rcTab);
                
                sprintf(text,"<%05d> %s:Item %d,RECT(%d,%d,%d,%d)",msgid++,page_title[index],i,rcTab.left,rcTab.top,rcTab.right,rcTab.bottom);
                ListBox_SetTopIndex(lb,ListBox_AddString(lb,text));
            }
        }
        Tab_AppendPage(tb,page,page_title[index]);
        TabCtrl_SetPadding(tb,15,0);
    }
    return 0;
}

int ListReport_Test(HWND hwnd)
{
    LVITEM lvi={0};
    HWND ListReport;
    HIMAGELIST ImgList;
    
    //创建Report
    ListReport=CreateWindowEx(WS_EX_STATICEDGE,TEXT("SysListView32"),NULL,\
    WS_CHILD/*|WS_BORDER*/|WS_VISIBLE|LV_VIEW_ICON|LVS_NOSCROLL,\
    10,10,400,400,hwnd,(HMENU)ID_LISTREPORT_01,\
    (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
    Report_InitialSettings(ListReport);
    
    //设置字体
    HFONT pre_font=(HFONT)SendMessage(ListReport,WM_GETFONT,0,0);
    HFONT font;
    LOGFONT lf={0};
    GetObject(pre_font,sizeof(LOGFONT),&lf);
    strcpy(lf.lfFaceName,"Courier New");
    lf.lfHeight=14;
    font=CreateFontIndirect(&lf);
    SendMessage(ListReport, WM_SETFONT, (WPARAM)font, 0);

    //创建图片列表
    HINSTANCE instance=(HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);
    ImgList=ImageList_Create(48,48,ILC_MASK|ILC_COLOR32,1,1);
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_a1"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_a2"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_a3"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_a4"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_a5"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_a6"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_a7"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_a8"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_a9"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_aa"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_ab"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_ac"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_ad"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_ae"));
    ImageList_AddIcon(ImgList,LoadIcon(instance,"IDC_ICON_af"));
    
    //设置参数
    ListView_SetImageList(ListReport,ImgList,LVSIL_NORMAL);
    ListView_SetBkColor(ListReport,RGB(15,15,15));
    ListView_SetTextBkColor(ListReport,RGB(160,160,160));
    ListView_SetTextColor(ListReport,RGB(150,150,150));
    
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
	for(int index=0;index<sizeof(item_text)/sizeof(char[256]);index++) {
    	lvi.mask=LVIF_TEXT|LVIF_IMAGE;
    	lvi.pszText=item_text[index];
    	lvi.iItem=index;
    	lvi.iImage=index;
    	SendMessage(ListReport,LVM_INSERTITEM,0,(LPARAM)&lvi);
    }
    
    return 0;
}

int ListView_Test(HWND hwnd)
{
    HIMAGELIST list=ImageList_Create(32,32,ILC_MASK|ILC_COLOR32,1,1);
    
    ImageList_AddIcon(list,LoadIcon(instance,"IDC_ICON_DELETE"));
    ImageList_AddIcon(list,LoadIcon(instance,"IDC_ICON_VEDIT"));
    ImageList_AddIcon(list,LoadIcon(instance,"IDC_ICON_VBROWSER"));
    ImageList_AddIcon(list,LoadIcon(instance,"IDC_ICON_VMONEY"));
    
    HWND listctrl=CreateWindowEx(0,
    WC_LISTVIEW,NULL,
    WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS|WS_BORDER,
    20,40,400,150,
    hwnd,(HMENU)IDC_LISTVIEW_01,
    (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),NULL);       
    ListView_InitialSettings(listctrl);
    HWND head=ListView_GetHeader(listctrl);
    SetWindowLongPtr(head,GWL_STYLE,(~WS_VISIBLE)&((UINT)GetWindowLongPtr(head,GWL_STYLE)));
    LVHead_InitialSettings(head);
    ListView_GetSettings(listctrl)->imglist=list;
    LOGFONT lf={0};
    GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
    lf.lfHeight=18;
    lf.lfWeight=20;
    strcpy(lf.lfFaceName,"微软雅黑");
    HFONT font=CreateFontIndirect(&lf);
    
    char title[][256]={\
        "操作(&O)",
        "Row No.",
        "发生总金额（賬）",
        "Col 3",
        "金额（賬）",
        "Col 7"
    };
    int title_width[]={\
        200,150,250,150,250,250,150,150,200
    };
    int columns=sizeof(title)/sizeof(char[256]);
    int col_i=0;
    while(col_i<columns) {
        LV_COLUMN lvc={0};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        lvc.fmt=LVCFMT_LEFT;
        lvc.pszText=title[col_i];
        lvc.cchTextMax=255;
        lvc.cx=title_width[col_i];
        
        ListView_InsertColumn(listctrl,col_i,&lvc);
        
        col_i++;
    }

    char listindex[][256]={\
            "编号%05d",
            "房间号-RoomNo.:%d",
            "%d",
            "video:%d",
            "%.2lf",
            "information %d"
    };
    int i_row=0;
    int i_col=0;
    int rows=8;
 
    while(i_row<rows) {
        i_col=0;
        while(i_col<columns) {
            LV_ITEM lv={0};
            char text[256]="";
            
            if(strstr(title[i_col],"金额")) sprintf(text,listindex[i_col],i_row*100000000.0/1.22*i_col*(i_row%3?-1:1));
            else sprintf(text,listindex[i_col],i_row);
            lv.mask=LVIF_TEXT;
            lv.iItem=i_row;
            lv.iSubItem=i_col;
            lv.pszText=text;
            lv.cchTextMax=256;
            
            if(i_col==0) ListView_InsertItem(listctrl,&lv);
            ListView_SetItem(listctrl,&lv);
            i_col++;
        }
        
        i_row++;
    }
    
    return 0;
}
