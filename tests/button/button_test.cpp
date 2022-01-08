#include <windows.h>
#include <gdiplus.h>
#include<commctrl.h>

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib,"comctl32.lib")

#define WINDOW_CLASS_NAME "BUTTON_TEST"
#define ID_BTN01 0x0001
#define ID_BTN02 0x0002
#define ID_BTN03 0x0003

using namespace Gdiplus;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
HINSTANCE instance;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int Button_Test(HWND hwnd);

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
    HWND hwnd;
    MSG msg;
    WNDCLASSEX winclass;
    InitCommonControls();
    instance=hInstance;
    
    winclass.cbSize=sizeof(WNDCLASSEX);
    winclass.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
    winclass.lpfnWndProc=WindowProc;
    winclass.cbClsExtra=0;
    winclass.cbWndExtra=0;
    winclass.hInstance=hInstance;
    winclass.hIcon=LoadIcon(NULL, IDI_APPLICATION);
    winclass.hCursor=LoadCursor(NULL,IDC_ARROW);//LoadCursor(instance,"IDC_CURSOR");
    winclass.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
    winclass.lpszMenuName=NULL;
    winclass.lpszClassName=WINDOW_CLASS_NAME;
    winclass.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
    if(!RegisterClassEx(&winclass)) return 0;
        
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    if (!(hwnd=CreateWindowEx(NULL,WINDOW_CLASS_NAME,"button test",WS_OVERLAPPEDWINDOW|WS_VISIBLE,\
        240,262,800,500,NULL,NULL,hInstance,NULL)))
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

LRESULT CALLBACK WindowProc(HWND hwnd,UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg){
	case WM_CREATE: {
	    Button_Test(hwnd);
	} break;
	case WM_SIZE: {
		int height=HIWORD(lParam);
		int width=LOWORD(lParam);
		//设置Z序
		int height_tab=200;
		//SetWindowPos(GetDlgItem(hwnd,IDC_TabView_01),HWND_BOTTOM,5,5,width-5*2,height-5-height_tab,SWP_SHOWWINDOW);
		//SetWindowPos(GetDlgItem(hwnd,IDC_LISTBOX_01),HWND_BOTTOM,5,height-height_tab,width-5*2,height_tab-5,SWP_SHOWWINDOW);
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
	case WM_NOTIFY: {
	    LPNMHDR phdr=(LPNMHDR)lParam;
	    switch(phdr->code) {
	    //case TCN_SELCHANGE: {// 当前选中项目TabCtrl_GetCurSel
	    //    int index_Sel= TabCtrl_GetCurSel(phdr->hwndFrom);
	    //    //....
	    //    //RECT rcItem;
	    //    //TabCtrl_GetItemRect(phdr->hwndFrom,index_Sel,&rcItem);
	    //} break;
	    //case TCN_SELCHANGING:// 当前选中项目将要变化
	    //    BOOL Stop_Change=FALSE;
	    //    int index_Sel= TabCtrl_GetCurSel(phdr->hwndFrom);
	    //    //...
	    //    //RECT rcItem;
	    //    //TabCtrl_GetItemRect(phdr->hwndFrom,index_Sel,&rcItem);	        
	    //    return Stop_Change;//是否阻止改变
	    //break;	    
	    }
	} break;
	//case WM_CTLCOLORLISTBOX: {
	//    HDC hdc=(HDC)wParam;
	//    HWND lb=(HWND)lParam;
	//    
	//    HWND tb=GetDlgItem(hwnd,IDC_TabView_01);
	//    pTabStyle ts=Tab_GetSettings(tb);
	//    if(!ts) break;
	//    SetTextColor(hdc,ts->color_text);
	//    SetBkColor(hdc,ts->color_bk);
	//    return (HRESULT)(ts->brush);
	//} break;
	/*case WM_CTLCOLORBTN:{
	    HDC hdc=(HDC)wParam;
	    HWND btn=(HWND)lParam;
        
        //有BS_PUSHBUTTON的不起作用
        SetTextColor(hdc,RGB(0,125,250));
	    SetBkColor(hdc,RGB(200,15,15));
	    return (HRESULT)CreateSolidBrush(RGB(0,100,230));
	} break;*/
	case WM_DRAWITEM: {
	    LPDRAWITEMSTRUCT pds=(LPDRAWITEMSTRUCT)lParam;
	    HDC hdc=pds->hDC;
        RECT rc;
        CopyRect(&rc,&pds->rcItem);
        //rc.right-=5;
        //InflateRect(&rc,-1,-1);
        HDC memdc=CreateCompatibleDC(hdc);
        HBITMAP bmp=CreateCompatibleBitmap(hdc,rc.right-rc.left,rc.bottom-rc.top);
        HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
        
        Graphics graphic(memdc);
        graphic.SetSmoothingMode(SmoothingModeHighQuality);
        graphic.FillRectangle(&SolidBrush(Color::Black),0,0,rc.right-rc.left,rc.bottom-rc.top);
        //graphic.FillEllipse(&SolidBrush(Color(250,0,128,250)),0,0,rc.right-rc.left,rc.bottom-rc.top);
    	GraphicsPath path;
    	path.AddEllipse(10,10,rc.right-rc.left-20,rc.bottom-rc.top-20);
    	PathGradientBrush brush(&path);
    	brush.SetCenterColor((pds->itemState==ODS_HOTLIGHT)?Color(255,255,0,0):Color(255,100,100,100));
    	Color colors[]={Color(255,0,0,0)};
    	int count=1;
    	brush.SetSurroundColors(colors,&count);
    	brush.SetCenterPoint(Point((rc.right-rc.left)>>1,(rc.bottom-rc.top)>>1));
    	graphic.FillRectangle(&brush,10,10,rc.right-rc.left-20,rc.bottom-rc.top-20);
    	
    	char text[256]="";
    	GetWindowText(pds->hwndItem,text,sizeof(text));
    	RECT rcText;
    	CopyRect(&rcText,&rc);
    	OffsetRect(&rcText,-rcText.left,-rcText.top);
    	SetBkMode(memdc,TRANSPARENT);
    	SetTextColor(memdc,RGB(25,155,255));
    	
    	LOGFONT lf;
    	GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);    	
    	lf.lfHeight = -MulDiv(12,GetDeviceCaps(hdc, LOGPIXELSY), 72);
    	lf.lfWidth = 0;
    	lf.lfEscapement = 0;//字体旋转角度
    	if(strstr("▲▼",text)) lf.lfEscapement =900;
    	//else if(strstr(text,"▼")) lf.lfEscapement=1800;
    	lf.lfOrientation = 0;
    	lf.lfWeight = FW_BOLD;
    	lf.lfItalic = FALSE;
    	lf.lfUnderline = FALSE;
    	lf.lfStrikeOut = FALSE;
    	lf.lfCharSet = DEFAULT_CHARSET;
    	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    	lf.lfQuality = DEFAULT_QUALITY;
    	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;    	
    	strcpy(lf.lfFaceName,"微软雅黑");
    	HFONT hf=CreateFontIndirect(&lf);
    	HFONT pre_hf=(HFONT)SelectObject(memdc,hf);
    	RECT rcAdj;
    	CopyRect(&rcAdj,&rc);
    	OffsetRect(&rcAdj,-5,20);
    	DrawText(memdc,text,-1,(strstr("▲▼",text)?&rcAdj:&rc),DT_VCENTER|DT_CENTER|DT_SINGLELINE);
    	
    	BitBlt(hdc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,
    	       memdc,0,0,SRCCOPY);
    	DeleteObject(SelectObject(memdc,pre_hf));
    	DeleteObject(SelectObject(memdc,pre_bmp));
	    DeleteDC(memdc);
	
	} break;
	default: break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int Button_Test(HWND hwnd)
{
    //button
    HWND btn=CreateWindowEx(NULL,WC_BUTTON,"▲",(WS_TABSTOP|WS_CHILD|WS_VISIBLE|BS_OWNERDRAW|WS_CLIPSIBLINGS),
    					   10,10,80,80,hwnd,(HMENU)ID_BTN01,
    					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
    HWND btn2=CreateWindowEx(NULL,WC_BUTTON,"| |",(WS_TABSTOP|WS_CHILD|WS_VISIBLE|BS_OWNERDRAW|WS_CLIPSIBLINGS),
    					   90,10,80,80,hwnd,(HMENU)ID_BTN02,
    					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
    HWND btn3=CreateWindowEx(NULL,WC_BUTTON,"▼",(WS_TABSTOP|WS_CHILD|WS_VISIBLE|BS_OWNERDRAW|WS_CLIPSIBLINGS),
    					   170,10,80,80,hwnd,(HMENU)ID_BTN03,
    					   (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),NULL);
    
    //set region.
    RECT rc;
    GetClientRect(btn,&rc);

    HRGN rgn=CreateEllipticRgn((rc.right-rc.left)>>1,(rc.bottom-rc.top)>>1,(rc.right-rc.left)>>1+1,(rc.bottom-rc.top)>>1+1);
    
    HDC hdc,memdc;
    HBITMAP bmp,pre_bmp;
    
    COLORREF back_color=RGB(0,0,0);
    hdc=GetDC(NULL);
    memdc=CreateCompatibleDC(hdc);
    bmp=CreateCompatibleBitmap(hdc,rc.right-rc.left,rc.bottom-rc.top);
    pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    
    Graphics graphic(memdc);
    graphic.SetSmoothingMode(SmoothingModeHighQuality);
    graphic.FillRectangle(&SolidBrush(Color(255,0,0,0)),0,0,rc.right-rc.left,rc.bottom-rc.top);
    //graphic.FillEllipse(&SolidBrush(Color(255,100,100,100)),0,0,rc.right-rc.left,rc.bottom-rc.top);
    GraphicsPath path;
    path.AddEllipse(0,0,rc.right-rc.left,rc.bottom-rc.top);
    PathGradientBrush brush(&path);
    brush.SetCenterColor(Color(255,250,10,20));
    Color colors[]={Color(255,0,20,40)};
    int count=1;
    brush.SetSurroundColors(colors,&count);
    brush.SetCenterPoint(Point((rc.right-rc.left)>>1,(rc.bottom-rc.top)>>1));
    graphic.FillRectangle(&brush,0,0,rc.right-rc.left,rc.bottom-rc.top);    	
    	
    BITMAP bm={0};
    BYTE red,green,blue;
    GetObject(bmp,sizeof(bm),&bm);
    int bmpixes_bytes=bm.bmBitsPixel>>3;
	int bmsize=bm.bmWidthBytes*bm.bmHeight;
	BYTE* pbmpixels=(BYTE*)malloc(sizeof(BYTE)*bmsize);
	GetBitmapBits(bmp,bmsize,pbmpixels);
    for(int i_height=0,y_offset=0;i_height<bm.bmHeight;i_height++,y_offset+=bm.bmWidthBytes) {
	    int i_start=-1,i_stop=-1;
	    for(int i_width=0,x_offset=0;i_width<bm.bmWidth;i_width++,x_offset+=bmpixes_bytes) {
            blue=pbmpixels[y_offset+x_offset];
            green=pbmpixels[y_offset+x_offset+1];
            red=pbmpixels[y_offset+x_offset+2];
            
            if(!((blue==GetBValue(back_color))&&(red==GetRValue(back_color))&&(green==GetGValue(back_color)))&&i_start==-1) {
                i_start=i_width;
                
                i_stop=bm.bmWidth-i_start+1;
                HRGN rgn_tmp=CreateRectRgn(i_start,i_height,i_stop,i_height+1);
                CombineRgn(rgn,rgn,rgn_tmp,RGN_OR);
                DeleteObject(rgn_tmp);
                break;
            }
            
            //这种算法不知道为啥，画出来右边有一块豁口
            //对称区域，这段代码不会执行
            if((blue==GetBValue(back_color))&&(red==GetRValue(back_color))&&(green==GetGValue(back_color))&&((i_start!=-1)||(i_width==bm.bmWidth-1))) {
                i_stop=i_width;
                HRGN rgn_tmp=CreateRectRgn(i_start,i_height,i_stop,i_height+1);
                CombineRgn(rgn,rgn,rgn_tmp,RGN_OR);
                DeleteObject(rgn_tmp);
                i_start=-1;
            }
	    }
	}
	free(pbmpixels);
	DeleteObject(SelectObject(memdc,pre_bmp));
	DeleteDC(memdc);
	ReleaseDC(NULL,hdc);
    
    if(rgn==NULL) MessageBox(hwnd,"原型Region创建失败。","Error",MB_OK|MB_ICONERROR);
    else {
        SetWindowRgn(btn,rgn,FALSE);
        SetWindowRgn(btn2,rgn,FALSE);
        SetWindowRgn(btn3,rgn,FALSE);
    }
    return 0;
}