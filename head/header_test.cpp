#include "../../include/head.h"

#pragma comment(lib,"../../lib/head.lib")

/***********************************************************************************/
/**                          以下部分为测试程序                                   **/
/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

HBRUSH static_brush;

int Text_Append(HWND hwnd,char* ptext);
LRESULT CALLBACK HeadScrollTestProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

POINT StartDrag;
POINT StartPos;
BOOL StartDragFlag=FALSE;
int DrawScrollTest(HWND hwnd);
int GetScrollRect(HWND hwnd,LPRECT prc);
int GetScrollPos(HWND hwnd);

#define ID_GRIDHEADER_01 0x0001
#define ID_BUTTON_01 0x0002
#define ID_BUTTON_02 0x0003
#define ID_TEXT_01 0x0004
#define ID_TEXT_02 0x0005

void InitialWindow(HWND hwnd);
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT ctrl_command(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

HWND gshadow;
int BlendShadow(HWND hwnd);

void regist_window(char* class_name,HINSTANCE instance,WNDPROC proc)
{
    WNDCLASSEX wndclass;

    wndclass.cbSize=sizeof(wndclass);
    wndclass.style = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;//红烧鱼，如你所愿
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

    RegisterClassEx(&wndclass);
}

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
    char szClassName[]="Header_Scroll_Test";
    
    regist_window(szClassName,hInstance,MainWndProc);
        
    HWND hwnd = CreateWindowEx(0,
                               szClassName,
                               szClassName,
                               WS_THICKFRAME|WS_CAPTION|WS_OVERLAPPEDWINDOW|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CLIPCHILDREN,
                               200,
                               50,
                               700,
                               600,
                               NULL,
                               NULL,
                               hInstance,
                               NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL,"create windows error !","error",MB_OK);
        return -1;
    }
    
    //创建阴影窗口
    HWND shadow=CreateWindowEx(WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT,
                               szClassName,
                               szClassName,
                               WS_POPUP|WS_VISIBLE,
                               0,
                               0,
                               0,
                               0,
                               NULL,
                               NULL,
                               hInstance,
                               NULL);
    gshadow=shadow;    
    
    ShowWindow(hwnd,nCmdShow);
    UpdateWindow(hwnd);
                                           
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

LRESULT ctrl_command(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    LRESULT result = 0;
    switch(LOWORD(wParam))
    {
        case ID_BUTTON_01://left
        {
            HWND ctrl_hwnd = (HWND)lParam;
            HWND header=GetDlgItem(hwnd,ID_GRIDHEADER_01);
            pHeadStyle hs=Head_GetSettings(header);
            
            int item_count=Header_GetItemCount(header);
            int total_offset=0;
            for(int index=0;index<item_count;index++) {
                HDITEM hdi={0};
                
                hdi.mask=HDI_WIDTH;
                Header_GetItem(header,index,&hdi);
                
                total_offset+=hdi.cxy;
            }
            
            RECT rc;
            GetClientRect(header,&rc);
            
            if((total_offset-(rc.right-rc.left)+hs->offset)<=0) break;
            
            hs->offset-=20;
            ScrollWindow(header,-20,0,NULL,NULL);
            
            char text[256]="";
            sprintf(text,"Offset:%d",hs->offset);
            SetWindowText(hwnd,text);
        }
        break;
        case ID_BUTTON_02://right
        {
            HWND ctrl_hwnd = (HWND)lParam;   
            HWND header=GetDlgItem(hwnd,ID_GRIDHEADER_01);
            pHeadStyle hs=Head_GetSettings(header);
            if(hs->offset>=0) break;
            hs->offset+=20;
            ScrollWindow(header,20,0,NULL,NULL);
            
            char text[256]="";
            sprintf(text,"Offset:%d",hs->offset);
            SetWindowText(hwnd,text);
        }
        break;        
    }
    return result;
}

int BlendShadow(HWND hwnd)
{
	RECT rc;
	GetWindowRect(hwnd,&rc);
	HDC hdc,memdc;
	hdc=GetDC(NULL);//GetWindowDC(hwnd);
	memdc=CreateCompatibleDC(hdc);
	HBITMAP bmp,pre_bmp;
	
	bmp=CreateCompatibleBitmap(hdc,rc.right-rc.left,rc.bottom-rc.top);
	pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
	HBRUSH brush_bk=CreateSolidBrush(RGB(10,120,120));
	RECT rcMem={0,0,rc.right-rc.left,rc.bottom-rc.top};
	FillRect(memdc,&rcMem,brush_bk);
	DeleteObject(brush_bk);
	
	POINT pt={rc.left,rc.top};
	POINT pt_mem={0,0};
	SIZE size={rc.right-rc.left,rc.bottom-rc.top};
	BLENDFUNCTION stBlend={AC_SRC_OVER,0,200,AC_SRC_ALPHA};
    UpdateLayeredWindow(hwnd,NULL,&pt,&size,memdc,&pt_mem,0, &stBlend, ULW_ALPHA);
    
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    ReleaseDC(hwnd,hdc);
    return 0;    
}

LRESULT CALLBACK MainWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    switch(message)
    {
    case WM_CREATE:
        {
            GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
            static_brush=CreateSolidBrush(RGB(45,45,45));
            if(hwnd!=gshadow) InitialWindow(hwnd);
        }
        break;
    case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
    case WM_COMMAND:
        {
            ctrl_command(hwnd,message,wParam,lParam);
        }
    break;
    case WM_SIZE:
        {
            int height=(int)HIWORD(lParam);
            int width=(int)LOWORD(lParam);
            HWND head=GetDlgItem(hwnd,ID_GRIDHEADER_01);
            HWND lbl=GetDlgItem(hwnd,ID_TEXT_01);
            HWND lbscroll=GetDlgItem(hwnd,ID_TEXT_02);
            RECT head_rect;
            int head_height=0;
            
            GetClientRect(head,&head_rect);
            head_height=28;//head_rect.bottom-head_rect.top;
            MoveWindow(head,1,0,width-2*1,head_height,TRUE);
            MoveWindow(lbl,1,head_height+10,width-1*2,450,TRUE);
            MoveWindow(lbscroll,1,head_height+20+450,width-1*2,14,TRUE);
            
            if(hwnd!=gshadow) {
                BlendShadow(gshadow);
            }
        }
    break;
	case WM_CTLCOLORSTATIC: { //Edit 在只读模式ES_READONLY下发送该消息
	    HDC hdc=(HDC)wParam;
	    HWND edit=(HWND)lParam;
	    
	    SetTextColor(hdc,RGB(0,128,255));
	    SetBkMode(hdc,TRANSPARENT);
	    SetBkColor(hdc,RGB(30,30,30));
	    return (HRESULT)static_brush;
	} break;
	case WM_WINDOWPOSCHANGED: {
	    LPWINDOWPOS winpos=(LPWINDOWPOS)lParam;
	    if(hwnd!=gshadow) {
            if((winpos->flags&(SWP_NOMOVE|SWP_NOSIZE))!=(SWP_NOSIZE|SWP_NOMOVE)) {
                RECT rc;
                GetWindowRect(hwnd,&rc);
                SetWindowPos(gshadow,NULL,rc.left-25, rc.top-25,
                                     rc.right-rc.left+50,rc.bottom-rc.top+50,SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
                BlendShadow(gshadow);
            }
	    }
	} break;
    case WM_MOUSEACTIVATE: {
        if(hwnd==gshadow) {
            return MA_NOACTIVATE;
        }
        //
    } break;
    }
    return DefWindowProc(hwnd,message,wParam,lParam);
}

void InitialWindow(HWND hwnd)
{
    HFONT font=CreateFont(20,0,0,0,
                    FW_MEDIUM,//FW_SEMIBOLD,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "Courier New");
                    
    HWND header=CreateWindow(WC_HEADER,
                              NULL,
                              WS_CHILD|WS_VISIBLE|HDS_BUTTONS/*|HDS_HORZ*/,
                              50,50,1000,38,
                              hwnd,
                              (HMENU)ID_GRIDHEADER_01,
                              (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),
                              NULL);
    Head_InitialSettings(header);
    SendMessage(header,WM_SETFONT,(WPARAM)font,0);
    {
        HDITEM hdi={0};
        char treegrid_title[][256]={\
        "Column 1",
        "Column 2",
        "Column 3",
        "Column 4",
        "Column 5",
        "Column 6",
        "Column 7",
        "Column 8",
        "Column 9",
        "Column 10"/*,
        "Column 11",
        "Column 12",
        "Column 13",
        "Column 14",
        "Column 15",
        "Column 16",
        "Column 17",
        "Column 18",
        "Column 19",
        "Column 20"*/};     
        for(int index=0;index<sizeof(treegrid_title)/sizeof(char[256]);index++) {
            hdi.mask=HDI_TEXT|HDI_FORMAT|HDI_WIDTH;
            hdi.cxy=150;
            hdi.pszText=treegrid_title[index];
            hdi.cchTextMax=sizeof(hdi.pszText);
            hdi.fmt=HDF_CENTER|HDF_STRING;
            
            SendMessage(header,HDM_INSERTITEM,(WPARAM)index,(LPARAM)&hdi);
        }
    }
    
    HWND bn_left=CreateWindow(WC_BUTTON,
                              "<<Left",
                              WS_CHILD|WS_VISIBLE,
                              50,250,150,25,
                              hwnd,
                              (HMENU)ID_BUTTON_01,
                              (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),
                              NULL);
    SendMessage(bn_left,WM_SETFONT,(WPARAM)font,0);
    HWND bn_right=CreateWindow(WC_BUTTON,
                              "Right>>",
                              WS_CHILD|WS_VISIBLE,
                              250,250,150,25,
                              hwnd,
                              (HMENU)ID_BUTTON_02,
                              (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),
                              NULL);
    SendMessage(bn_right,WM_SETFONT,(WPARAM)font,0);
    

    HWND lbl=CreateWindow(WC_STATIC,
                          "",
                          WS_CHILD|WS_VISIBLE,
                          0,80,500,150,
                          hwnd,
                          (HMENU)ID_TEXT_01,
                          (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),
                          NULL);
    SendMessage(lbl,WM_SETFONT,(WPARAM)font,0);
    
    char text[1024]="";
    sprintf(text,"HHT_ABOVE:0x%08X",HHT_ABOVE);
    Text_Append(lbl,text);
    sprintf(text,"HHT_BELOW:0x%08X",HHT_BELOW);
    Text_Append(lbl,text);
    sprintf(text,"HHT_NOWHERE:0x%08X",HHT_NOWHERE);
    Text_Append(lbl,text);
    sprintf(text,"HHT_ONDIVIDER:0x%08X",HHT_ONDIVIDER);
    Text_Append(lbl,text);
    sprintf(text,"HHT_ONDIVOPEN:0x%08X",HHT_ONDIVOPEN);
    Text_Append(lbl,text);
    sprintf(text,"HHT_ONHEADER:0x%08X",HHT_ONHEADER);
    Text_Append(lbl,text);
    
    HWND lbl2=CreateWindow(WC_STATIC,
                          "",
                          WS_CHILD|WS_VISIBLE|SS_NOTIFY,
                          0,80,100,10,
                          hwnd,
                          (HMENU)ID_TEXT_02,
                          (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),
                          NULL);
    SendMessage(lbl2,WM_SETFONT,(WPARAM)font,0);
    SetWindowLongPtr(lbl2,GWLP_USERDATA,GetWindowLongPtr(lbl2,GWLP_WNDPROC));
    SetWindowLongPtr(lbl2,GWLP_WNDPROC,(LONG_PTR)HeadScrollTestProc);
}

int Text_Append(HWND hwnd,char* ptext)
{
    char text[1024]="";
    
    Static_GetText(hwnd,text,sizeof(text));
    strcat(text,ptext);
    strcat(text,"\r\n");
    Static_SetText(hwnd,text);
    
    return 0;
}

LRESULT CALLBACK HeadScrollTestProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    WNDPROC pre_scroll_proc=(WNDPROC)GetWindowLongPtr(hwnd,GWLP_USERDATA);
    
    switch(message) {
    case WM_PAINT: {
        DrawScrollTest(hwnd);
        return 0;
    } break;
    case WM_LBUTTONDOWN: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        RECT rc_thumb;
        if(GetScrollRect(hwnd,&rc_thumb)!=0) break;
        
        HWND head=GetDlgItem(GetParent(hwnd),ID_GRIDHEADER_01);
        pHeadStyle hs=Head_GetSettings(head);    
            
        if(PtInRect(&rc_thumb,pt)) {
            StartDrag.x=pt.x;
            StartPos.x=-(hs->offset);//GetScrollPos(hwnd);
            StartDragFlag=TRUE;
            SetCapture(hwnd);
        }
    } break;
    case WM_MOUSEMOVE: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        if(StartDragFlag&&GetCapture()==hwnd) {
            HWND head=GetDlgItem(GetParent(hwnd),ID_GRIDHEADER_01);
            pHeadStyle hs=Head_GetSettings(head);  
            RECT rc,rc_thumb;
            
            GetClientRect(hwnd,&rc);
            GetScrollRect(head,&rc_thumb);
            //测算当前的Pos.
            SCROLLINFO si;
            si.nMin=0;
            si.nMax=Head_GetTotalSize(head);
            si.nPage=rc.right-rc.left;
            si.nPos=StartPos.x+(pt.x-StartDrag.x)*1.0/((rc.right-rc.left)-(rc_thumb.right-rc_thumb.left))*(si.nMax-si.nMin+1-si.nPage);
            
            int scroll_pixls=si.nPos-abs(hs->offset);
            hs->offset=-si.nPos;
            if(hs->offset>0) {
                scroll_pixls-=hs->offset;
                hs->offset=0;
            }
            else if(-hs->offset>si.nMax-si.nMin+1-si.nPage) {
                scroll_pixls=-(hs->offset-(si.nMax-si.nMin+1-si.nPage));
                hs->offset=-(si.nMax-si.nMin+1-si.nPage);
            }
            
            InvalidateRect(head,NULL,TRUE);
            
            char text[256]="";
            sprintf(text,"Offset:%d",hs->offset);
            SetWindowText(GetParent(hwnd),text);
        }
    } break;
    case WM_LBUTTONUP: {
        StartDragFlag=FALSE;
        ReleaseCapture();
    } break;
    }
    return CallWindowProc(pre_scroll_proc,hwnd,message,wParam,lParam);    
}

int GetScrollPos(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd,&rc);
    
    HWND head=GetDlgItem(GetParent(hwnd),ID_GRIDHEADER_01);
    pHeadStyle hs=Head_GetSettings(head);
    
    SCROLLINFO si;
    si.nMin=0;
    si.nMax=Head_GetTotalSize(head);
    si.nPage=rc.right-rc.left;
    si.nPos=-(hs->offset);
    
    if(rc.right-rc.left>si.nMax) return -1;
    
    int thumb_pixls=(rc.right-rc.left)*1.0*si.nPage/(si.nMax-si.nMin+1);
    if(thumb_pixls<50) thumb_pixls=50;
    int thumb_pos=(rc.right-rc.left-thumb_pixls)*1.0*si.nPos/(si.nMax-si.nMin+1-si.nPage);
    
    return thumb_pos;
}

int GetScrollRect(HWND hwnd,LPRECT prc)
{
    RECT rc;
    GetClientRect(hwnd,&rc);
    SCROLLINFO si;
    HWND head=GetDlgItem(GetParent(hwnd),ID_GRIDHEADER_01);
    pHeadStyle hs=Head_GetSettings(head);
    
    si.nMin=0;
    si.nMax=Head_GetTotalSize(head);
    si.nPage=rc.right-rc.left;
    si.nPos=-(hs->offset);
    
    if(rc.right-rc.left>si.nMax) return -1;
    
    int thumb_pixls=(rc.right-rc.left)*1.0*si.nPage/(si.nMax-si.nMin+1);
    if(thumb_pixls<50) thumb_pixls=50;
    int thumb_pos=(rc.right-rc.left-thumb_pixls)*1.0*si.nPos/(si.nMax-si.nMin+1-si.nPage);
    
    RECT rc_thumb;
    rc_thumb.left=rc.left+thumb_pos;
    rc_thumb.right=rc_thumb.left+thumb_pixls;
    rc_thumb.top=rc.top;
    rc_thumb.bottom=rc.bottom;
        
    CopyRect(prc,&rc_thumb);
    
    return 0;
}

int DrawScrollTest(HWND hwnd)
{
    HWND head=GetDlgItem(GetParent(hwnd),ID_GRIDHEADER_01);
    int total_offset=Head_GetTotalSize(head);
        
    RECT rc_thumb;
    GetScrollRect(hwnd,&rc_thumb);
    
    RECT rc,rc_mem;
    GetClientRect(hwnd,&rc);
    
    HDC hdc=GetDC(hwnd);
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,rc.right-rc.left,rc.bottom-rc.top);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    HBRUSH brush_bk=CreateSolidBrush(RGB(50,50,50));
    
    CopyRect(&rc_mem,&rc);
    OffsetRect(&rc_mem,-rc_mem.left,-rc_mem.top);
    FillRect(memdc,&rc_mem,brush_bk);
    
    OffsetRect(&rc_thumb,-rc.left,-rc.top);
    if(rc.right-rc.left<total_offset) {
    Graphics graphic(memdc);
    GraphicsPath path;
    LinearGradientBrush pbrush(Rect(rc_thumb.left,rc_thumb.top,rc_thumb.right-rc_thumb.left,rc_thumb.bottom-rc_thumb.top),
                               Color(255,5,5,5),
                               Color(255,45,45,45),
                               LinearGradientModeVertical);
    
    graphic.SetSmoothingMode(SmoothingModeHighQuality);
    path.AddArc(rc_thumb.left,rc_thumb.top,(rc_thumb.bottom-rc_thumb.top),(rc_thumb.bottom-rc_thumb.top),90,180);
    path.AddArc(rc_thumb.right-(rc_thumb.bottom-rc_thumb.top),rc_thumb.top,(rc_thumb.bottom-rc_thumb.top),(rc_thumb.bottom-rc_thumb.top),-90,180);
    graphic.FillPath(&pbrush,&path);
    }
    
    BitBlt(hdc,rc.left,rc.top,rc.right-rc.top,rc.bottom-rc.top,
           memdc,0,0,SRCCOPY);
    
    DeleteObject(brush_bk);
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    ReleaseDC(hwnd,hdc);
    
    return 0;
}
