#include "../include/listbox.h"

int ListBox_InitialSettings(HWND hwnd)
{
    pLBStyle ls=(pLBStyle)calloc(sizeof(RLBStyle),1);
    if(!ls) return -1;
    
    ls->font_cy=20;
    ls->font=CreateFont(ls->font_cy,0,0,0,
                    FW_MEDIUM,//FW_SEMIBOLD,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "Courier New");
    SendMessage(hwnd,WM_SETFONT,(WPARAM)ls->font,NULL);
    
    ls->color_bk=RGB(15,15,15);
    ls->color_text=RGB(0,125,250);
    ls->brush=CreateSolidBrush(ls->color_bk);
    
    ls->margin_border=1;
    
    ls->color_border=RGB(70,70,70);
    ls->brush_border=CreateSolidBrush(ls->color_border);
    
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)ls);
    ls->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ListBoxOwnerProc);
    return 0;
}

pLBStyle ListBox_GetSettings(HWND hwnd)
{
    return (pLBStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int ListBox_ClearSettings(HWND hwnd)
{
    pLBStyle ls=ListBox_GetSettings(hwnd);
    if(!ls) return -1;
    
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ls->pre_proc);
    DeleteObject(ls->font);
    DeleteObject(ls->brush);
    DeleteObject(ls->brush_border);
    
    free(ls);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,0);
    return 0;
}

int ListBox_NCPaint(HWND hwnd)
{
    pLBStyle ls=ListBox_GetSettings(hwnd);
    
    if(!ls) return -1;
    
    HDC hdc=GetWindowDC(hwnd);
    UINT style=GetWindowLongPtr(hwnd,GWL_STYLE);
    RECT rc={0},rc_mem;
    GetWindowRect(hwnd,&rc);
    CopyRect(&rc_mem,&rc);
    OffsetRect(&rc_mem,-rc_mem.left,-rc_mem.top);
    
    //创建内存DC
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,rc_mem.right,rc_mem.bottom);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    
    //刷底漆
    COLORREF color_trans=RGB(0,0,1);
    HBRUSH brush=CreateSolidBrush(color_trans);
    FillRect(memdc,&rc_mem,brush);
    
    //绘制边框
    if((style&WS_BORDER)==WS_BORDER) {
        RECT rc_tmp;
        CopyRect(&rc_tmp,&rc_mem);
        for(int index=0;index<ls->margin_border;index++) {
            FrameRect(memdc,&rc_tmp,ls->brush_border);
            InflateRect(&rc_tmp,-1,-1);
        }
    }
    
    //贴图
    TransparentBlt(hdc,0,0,(rc.right-rc.left),(rc.bottom-rc.top),
                   memdc,0,0,rc_mem.right,rc_mem.bottom,color_trans);
    
    DeleteObject(brush);
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    ReleaseDC(hwnd,hdc);
    return 0;
}

int ListBox_NCCalcSize(HWND hwnd,LPNCCALCSIZE_PARAMS calc)
{
    RECT rect_new;
    RECT rect_old;
    RECT client_rect_new;
    RECT client_rect_old;
    pLBStyle ls=ListBox_GetSettings(hwnd);
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL IsVScroll=((style&WS_VSCROLL)==WS_VSCROLL);
    BOOL IsBorder=((style&WS_BORDER)==WS_BORDER);
    //调整非客户区的位置和大小
    //处理前
    //0:新 1:老 2:老客户区
    CopyRect(&rect_new,&(calc->rgrc[0]));
    CopyRect(&rect_old,&(calc->rgrc[1]));
    CopyRect(&client_rect_old,&(calc->rgrc[2]));
    
    //处理后
    //0:新客户区 1:新 2:老
    client_rect_new = {rect_new.left+(IsBorder?ls->margin_border:0),
                       rect_new.top+(IsBorder?ls->margin_border:0),
                       rect_new.right-(IsBorder?ls->margin_border:0)-(IsVScroll?SCROLLBAR_PIXLS:0),
                       rect_new.bottom-(IsBorder?ls->margin_border:0)};
    CopyRect(&(calc->rgrc[0]),&client_rect_new);
    CopyRect(&(calc->rgrc[1]),&rect_new);
    CopyRect(&(calc->rgrc[2]),&rect_old);
    
    return WVR_VALIDRECTS;
}

int ListBox_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL IsAllign_TopLeft)
{
    UINT style=GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL IsVScroll=((style&WS_VSCROLL)==WS_VSCROLL);
    pLBStyle ls=ListBox_GetSettings(hwnd);
    RECT rc,rc_client;
    
    GetWindowRect(hwnd,&rc);
    GetClientRect(hwnd,&rc_client);
    
    if(!ls) return -1;
    if(!IsVScroll) return -1;
    
    switch(zone) {
    case ZVSCROLL: {
        prc->top=rc.top+ls->margin_border;
        prc->bottom=rc.bottom-ls->margin_border;
        prc->left=rc.left+(rc_client.right-rc_client.left)+ls->margin_border;
        prc->right=rc.right-ls->margin_border;    
    } break;
    case ZVSTHUMB: {  
        RECT rc_vs;  
        int thumb_pixls,thumb_pos;
        SCROLLINFO si={0};
        
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_VERT,&si);
        
        if(ListBox_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,FALSE)!=0) return -1;
        //设置拇指区
        thumb_pixls=(rc_vs.bottom-rc_vs.top)*1.0*si.nPage/(si.nMax-si.nMin+1);
        if(thumb_pixls<THUMB_MIN_PIXLS) thumb_pixls=THUMB_MIN_PIXLS;
        thumb_pos=(rc_vs.bottom-rc_vs.top-thumb_pixls)*1.0*si.nPos/(si.nMax-si.nMin-si.nPage+1);
        prc->top=rc_vs.top+thumb_pos;
        prc->bottom=prc->top+thumb_pixls;
        prc->left=rc.left+(rc_client.right-rc_client.left)+ls->margin_border;
        prc->right=rc.right-ls->margin_border;   
    } break;
    }
    if(IsAllign_TopLeft) OffsetRect(prc,-rc.left,-rc.top);
    return 0;
}

int ListBox_DrawScrollBar(HWND hwnd)
{
    pLBStyle ls=ListBox_GetSettings(hwnd);
    if(!ls) return -1;
    
    RECT rc_mem,rc_vs,rc_thumb;
    
    if(ListBox_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,TRUE)!=0) return -1;
    CopyRect(&rc_mem,&rc_vs);
    OffsetRect(&rc_mem,-rc_vs.left,-rc_vs.top);
    //资源初始化
    HDC hdc=GetWindowDC(hwnd);
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,rc_mem.right-rc_mem.left,rc_mem.bottom-rc_mem.top);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    
    //刷底漆
    FillRect(memdc,&rc_mem,ls->brush);
    
    if(ListBox_GetZoneRect(hwnd,ZVSTHUMB,&rc_thumb,TRUE)!=0) return -1;
    OffsetRect(&rc_thumb,-rc_vs.left,-rc_vs.top);

    //刷拇指
    Graphics graphic(memdc);
    graphic.SetSmoothingMode(SmoothingModeHighQuality);
    GraphicsPath path;
    LinearGradientBrush pbrush(Rect(rc_thumb.left,rc_thumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS),
                              Color(255,40,40,40),Color(255,20,20,20),LinearGradientModeHorizontal);
    path.AddArc(rc_thumb.left,rc_thumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,180,180);
    path.AddArc(rc_thumb.left,rc_thumb.bottom-SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,0,180);
    graphic.FillPath(&pbrush,&path);
    
    //贴图
    BitBlt(hdc,rc_vs.left,rc_vs.top,rc_vs.right-rc_vs.left,rc_vs.bottom-rc_vs.top,
           memdc,0,0,SRCCOPY);        
    
    //资源释放
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    ReleaseDC(hwnd,hdc);
    return 0;
}

LRESULT CALLBACK ListBoxOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    pLBStyle ls=ListBox_GetSettings(hwnd);
    switch(msg) {
    case WM_NCCALCSIZE: {
        if(wParam==TRUE) {
            LPNCCALCSIZE_PARAMS calc_param = (LPNCCALCSIZE_PARAMS)lParam;
            return ListBox_NCCalcSize(hwnd,calc_param);
        }
    } break;
    case WM_PAINT: {
        ListBox_DrawScrollBar(hwnd);
    } break;
    case WM_NCPAINT:
    case WM_NCACTIVATE: {
        ListBox_NCPaint(hwnd);
        ListBox_DrawScrollBar(hwnd);
        
        return 0;
    } break;
    case WM_MOUSEWHEEL: {
        ListBox_DrawScrollBar(hwnd);
    } break;
    case WM_KEYDOWN: {
    } break;
    case WM_VSCROLL: {//垂直
    } break;
    case WM_HSCROLL: {
    } break;
    case WM_NCHITTEST: {
        LRESULT result=CallWindowProc(ls->pre_proc,hwnd,msg,wParam,lParam);
        POINT hit={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        RECT rc={0};
        
        if(ListBox_GetZoneRect(hwnd,ZVSCROLL,&rc,FALSE)==0) {
            if(PtInRect(&rc,hit)) return HTVSCROLL;
        }
        
        return result;
    } break;
    case WM_NCLBUTTONDOWN: {
        RECT rc={0};
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        
        if(ListBox_GetZoneRect(hwnd,ZVSTHUMB,&rc,FALSE)!=0) break;   
        if(PtInRect(&rc,pt)) {
            SCROLLINFO si={0};
            si.cbSize=sizeof(si);
            si.fMask=SIF_ALL;
            GetScrollInfo(hwnd,SB_VERT,&si);
            
            ls->dragpos.y=si.nPos;
            ls->dragpt.y=pt.y;    
            SetCapture(hwnd);
            ls->scroll_state=THUMB_VCLICK;
            return 0;
        }
    } break;
    case WM_NCLBUTTONUP:
    case WM_LBUTTONUP: {
        ls->scroll_state=SCROLL_NONE;
        ReleaseCapture();
    } break;
    case WM_MOUSEMOVE: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        RECT rc_vs,rc_thumb;
        SCROLLINFO si={0};
        
        if(GetCapture()!=hwnd) break;
        if(ls->scroll_state!=THUMB_VCLICK) break;
        
        if(ListBox_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,FALSE)!=0) break;
        if(ListBox_GetZoneRect(hwnd,ZVSTHUMB,&rc_thumb,FALSE)!=0) break;
        ClientToScreen(hwnd,&pt);
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_VERT,&si);
        
        //计算当前的nPos
        si.nPos=(int)(ls->dragpos.y)+((int)(si.nMax-si.nMin+1-si.nPage))*1.0*((int)(pt.y-ls->dragpt.y))/((rc_vs.bottom-rc_vs.top)-(rc_thumb.bottom-rc_thumb.top));
        if(si.nPos>si.nMax-si.nMin+1-si.nPage) si.nPos=si.nMax-si.nMin+1-si.nPage;
        else if(si.nPos<si.nMin) si.nPos=si.nMin;
        
        if(si.nPos==ls->dragpos.y) break;
        
        //更新并重绘
        //根据滚动的坐标，调整scrollinfo， 重绘scrollbar ，发送WM_VSCROLL，系统都做了
        SendMessage(hwnd,WM_VSCROLL,MAKEWPARAM(SB_THUMBPOSITION,si.nPos),NULL);
    } break;
    case WM_NCDESTROY: {
        ListBox_ClearSettings(hwnd);
    }
    }
    return CallWindowProc(ls->pre_proc,hwnd,msg,wParam,lParam);
}

