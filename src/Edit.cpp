#include "../include/Edit.h"

int Edit_InitialSettings(HWND hwnd,char* title,char* text)
{
    pEditStyle es=(pEditStyle)calloc(sizeof(REditStyle),1);
    if(!es) return -1;
    
    es->internal_style=0;
    es->valid=TRUE;
    
    es->font_cy=17;
    es->font=CreateFont(es->font_cy,0,0,0,
                    FW_MEDIUM,//FW_SEMIBOLD,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "Courier New");
    SendMessage(hwnd,WM_SETFONT,(WPARAM)es->font,NULL);
    es->font_title=CreateFont(es->font_cy,0,0,0,
                    FW_MEDIUM,//FW_SEMIBOLD,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "微软雅黑");
    es->color_bk=RGB(15,15,15);
    es->color_text=RGB(0,125,250);
    es->brush=CreateSolidBrush(es->color_bk);
    
    es->color_bk_error=RGB(250,0,0);
    es->brush_error=CreateSolidBrush(es->color_bk_error);
    
    es->margin_left=70;
    es->color_title=RGB(140,140,140);
    strcpy(es->title,title);
    strcat(es->title,"：");
    
    Edit_SetText(hwnd,text);
    
    es->margin_border=1;//多行模式有用.
    
    //数值型右对齐
    if(ES_NUMBER==((UINT)GetWindowLongPtr(hwnd,GWL_STYLE)&ES_NUMBER)||
       es->internal_style==1||es->internal_style==2) 
        SetWindowLongPtr(hwnd,GWL_STYLE,ES_RIGHT|GetWindowLongPtr(hwnd,GWL_STYLE));
    
    es->color_bk_readonly=RGB(35,35,35);
    es->color_text_readonly=RGB(100,100,100);
    es->brush_readonly=CreateSolidBrush(es->color_bk_readonly);
    
    es->color_border=RGB(70,70,70);
    es->brush_border=CreateSolidBrush(es->color_border);
    
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)es);
    es->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)EditOwnerProc);
    return 0;
}

pEditStyle Edit_GetSettings(HWND hwnd)
{
    return (pEditStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int Edit_ClearSettings(HWND hwnd)
{
    pEditStyle es=Edit_GetSettings(hwnd);
    if(!es) return -1;
    
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)es->pre_proc);
    DeleteObject(es->font);
    DeleteObject(es->font_title);
    DeleteObject(es->brush);
    DeleteObject(es->brush_readonly);
    DeleteObject(es->brush_border);
    DeleteObject(es->brush_error);
    
    free(es);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,0);
    return 0;
}

int Edit_SetInternalStyle(HWND hwnd,int style)
{
    pEditStyle es=Edit_GetSettings(hwnd);
    
    es->internal_style=style;
    if(style==1||style==2) {
        SetWindowLongPtr(hwnd,GWL_STYLE,ES_RIGHT|GetWindowLongPtr(hwnd,GWL_STYLE));
        Edit_UpdateValue(hwnd);
    }
    return 0;
}

int Edit_UpdateValue(HWND hwnd) 
{
    char value_text[256]="";
    char* pcheck=NULL;
    pEditStyle es=Edit_GetSettings(hwnd);
    
    if(!es) return 0;
    if(es->internal_style!=1&&es->internal_style!=2) {
        es->valid=TRUE;
        return 0;//非数值型不做检查
    }
    Edit_GetText(hwnd,value_text,sizeof(value_text));
    if(strlen(value_text)<=0) {
        es->valid=TRUE;
        return 0;
    }
    else if(strlen(value_text)==1&&(value_text[0]=='+'||value_text[0]=='-')) { 
        es->valid=TRUE;
        return 0;
    }
    else if(es->internal_style==2) {
        double value_double;
        
        value_double=strtod(value_text,&pcheck);
        if(*pcheck!=0) {
            es->valid=FALSE;
            Edit_SetSel(hwnd,pcheck-value_text,strlen(value_text));
    
            wchar_t title_buffer[256]={0};
            wchar_t text_buffer[256]={0};
            swprintf(title_buffer,L"%s",L"数值转化");            
            EDITBALLOONTIP btip={0};
            btip.cbStruct=sizeof(EDITBALLOONTIP);
            btip.pszTitle=title_buffer;
            swprintf(text_buffer,L"%s",L"输入字符无法处理为有效的浮点数值，请检查。");
            btip.pszText=text_buffer;
            btip.ttiIcon=TTI_INFO;
            Edit_ShowBalloonTip(hwnd,&btip);
        }
        else {
            es->valid=TRUE;
        }
    }
    else {
        long value_long;
        value_long=strtol(value_text,&pcheck,10);
        if(*pcheck!=0) {
            es->valid=FALSE;
            Edit_SetSel(hwnd,pcheck-value_text,strlen(value_text));            
            
            wchar_t title_buffer[256]={0};
            wchar_t text_buffer[256]={0};
            swprintf(title_buffer,L"%s",L"数值转化");            
            EDITBALLOONTIP btip={0};
            btip.cbStruct=sizeof(EDITBALLOONTIP);
            btip.pszTitle=title_buffer;
            swprintf(text_buffer,L"%s",L"输入字符无法处理为有效的整型数值，请检查。");
            btip.pszText=text_buffer;
            btip.ttiIcon=TTI_INFO;
            Edit_ShowBalloonTip(hwnd,&btip);
        }
        else {
            es->valid=TRUE;
        }
    }
    
    return (es->valid?0:-1); 
}

int Edit_NCPaint(HWND hwnd)
{
    pEditStyle es=Edit_GetSettings(hwnd);
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    
    if(!es) return -1;
    //NC背景用client的DC进行填充
    HDC hdc=GetWindowDC(hwnd);
    RECT wnd_rc={0};
    //获取字体高度信息
    
    GetWindowRect(hwnd,&wnd_rc);
    OffsetRect(&wnd_rc,-wnd_rc.left,-wnd_rc.top);
    if(ES_MULTILINE!=(style&ES_MULTILINE)) {
        RECT title_rc={0},text_rc={0};
        int cy=0;
        TEXTMETRIC tm={0};
    
        GetTextMetrics(hdc,&tm);
        cy=tm.tmHeight+tm.tmExternalLeading;
        
        if(GetWindowLongPtr(hwnd,GWL_STYLE)&ES_READONLY)
            FillRect(hdc,&wnd_rc,es->brush_readonly);
        else FillRect(hdc,&wnd_rc,es->brush);
        
        CopyRect(&title_rc,&wnd_rc);
        title_rc.right=title_rc.left+es->margin_left;
        SetTextColor(hdc,es->color_title);
        SetBkMode(hdc,TRANSPARENT);
        SelectObject(hdc,es->font_title);
        //获取父窗口背景色
        HWND parent=GetParent(hwnd);
        if(parent) {
            //获取父窗口的背景色。通过WM_ERASEBKGND获取         
            //HDC parentdc=GetDC(parent);
            HBRUSH brush_title=CreateSolidBrush(RGB(0,0,0));//(HBRUSH)SendMessage(parent,WM_ERASEBKGND,(WPARAM)parentdc,0);
            FillRect(hdc,&title_rc,brush_title);
            DeleteObject(brush_title);
            //ReleaseDC(parent,parentdc);
        }
        DrawText(hdc,es->title,-1,&title_rc,DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
    
        CopyRect(&text_rc,&wnd_rc);
        text_rc.left=title_rc.right;
        FrameRect(hdc,&text_rc,es->brush_border);
    }
    else {
        //绘制滚动条
        RECT rc_mem;
        
        HDC memdc=CreateCompatibleDC(hdc);
        CopyRect(&rc_mem,&wnd_rc);
        HBITMAP bmp=CreateCompatibleBitmap(hdc,rc_mem.right-rc_mem.left,rc_mem.bottom-rc_mem.top);
        HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
        COLORREF color_trans=RGB(0,0,1);
        HBRUSH brush_trans=CreateSolidBrush(color_trans);
        
        //fresh background
        FillRect(memdc,&rc_mem,brush_trans);
        
        //绘制边框
        if((WS_BORDER==(WS_BORDER&style))&&es->margin_border>0) {
            for(int index=0;index<es->margin_border;index++) {
                FrameRect(memdc,&rc_mem,es->brush_border);
                InflateRect(&rc_mem,-1,-1);
            }
        }
        
        //贴图
        TransparentBlt(hdc,wnd_rc.left,wnd_rc.top,(wnd_rc.right-wnd_rc.left),(wnd_rc.bottom-wnd_rc.top),
                       memdc,0,0,(wnd_rc.right-wnd_rc.left),(wnd_rc.bottom-wnd_rc.top),color_trans);
        
        DeleteObject(brush_trans);
        DeleteObject(SelectObject(memdc,pre_bmp));
        DeleteDC(memdc);
    }
    
    ReleaseDC(hwnd,hdc);
    return 0;
}

int Edit_NCCalcSize(HWND hwnd,LPNCCALCSIZE_PARAMS calc)
{
    RECT rect_new;
    RECT rect_old;
    RECT client_rect_new;
    RECT client_rect_old;
    pEditStyle es=Edit_GetSettings(hwnd);   
    //调整非客户区的位置和大小
    //处理前
    //0:新 1:老 2:老客户区
    CopyRect(&rect_new,&(calc->rgrc[0]));
    CopyRect(&rect_old,&(calc->rgrc[1]));
    CopyRect(&client_rect_old,&(calc->rgrc[2]));
    
        
    HDC hdc=GetWindowDC(hwnd);
    RECT wnd_rc={0};
    //获取字体高度信息
    int cy=0,top_margin=0;;
    TEXTMETRIC tm={0};
    GetTextMetrics(hdc,&tm);
    cy=tm.tmHeight+tm.tmExternalLeading;
    top_margin=((rect_new.bottom-rect_new.top)-es->font_cy)>>1;
    if(top_margin<=0) top_margin=0;
    //更新font_cy.
    es->font_cy=cy;
        
    //处理后
    //0:新客户区 1:新 2:老
    client_rect_new = {rect_new.left+es->margin_left+2,
                       rect_new.top+top_margin,
                       rect_new.right-2,
                       rect_new.bottom-top_margin};
    CopyRect(&(calc->rgrc[0]),&client_rect_new);
    CopyRect(&(calc->rgrc[1]),&rect_new);
    CopyRect(&(calc->rgrc[2]),&rect_old);
    
    return WVR_VALIDRECTS;
}

int Edit_NCCalSize_MultiLines(HWND hwnd,LPNCCALCSIZE_PARAMS calc)
{
    RECT rect_new;
    RECT rect_old;
    RECT client_rect_new;
    RECT client_rect_old;
    pEditStyle es=Edit_GetSettings(hwnd);
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL IsBorder=((style&WS_BORDER)==WS_BORDER);   
    
    if(!es) return -1;
    //调整非客户区的位置和大小
    //处理前
    //0:新 1:老 2:老客户区
    CopyRect(&rect_new,&(calc->rgrc[0]));
    CopyRect(&rect_old,&(calc->rgrc[1]));
    CopyRect(&client_rect_old,&(calc->rgrc[2]));
    
    //处理后
    //0:新客户区 1:新 2:老
    client_rect_new = {rect_new.left+(IsBorder?es->margin_border:0),
                       rect_new.top+(IsBorder?es->margin_border:0),
                       rect_new.right-(IsBorder?es->margin_border:0)-(Edit_IsShowVScroll(hwnd)?SCROLLBAR_PIXLS:0),
                       rect_new.bottom-(IsBorder?es->margin_border:0)};
    CopyRect(&(calc->rgrc[0]),&client_rect_new);
    CopyRect(&(calc->rgrc[1]),&rect_new);
    CopyRect(&(calc->rgrc[2]),&rect_old);
    
    return WVR_VALIDRECTS;
}

LRESULT CALLBACK EditOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    pEditStyle es=Edit_GetSettings(hwnd);
    switch(msg) {
    case WM_NCCALCSIZE: {
        UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
        if(wParam==TRUE) {
            LPNCCALCSIZE_PARAMS calc_param = (LPNCCALCSIZE_PARAMS)lParam;
            
            //多行模式的调整则不同
            if((style&ES_MULTILINE)==ES_MULTILINE) return Edit_NCCalSize_MultiLines(hwnd,calc_param);
            else return Edit_NCCalcSize(hwnd,calc_param);
        }
    } break;
    case WM_NCPAINT:
    case WM_NCACTIVATE: {
        Edit_NCPaint(hwnd);
        Edit_DrawVScrollBar(hwnd);
        
        return 0;
    } break;
    case WM_MOUSEWHEEL: {
        Edit_DrawVScrollBar(hwnd);
    } break;
    case WM_PAINT: {
        Edit_DrawVScrollBar(hwnd);
    } break;
    case WM_NCHITTEST: {
        POINT hit={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
        RECT rc_vs={0};
        
        if((ES_MULTILINE&style)!=ES_MULTILINE) break;
        if(Edit_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,FALSE)!=0) break;
        
        if(PtInRect(&rc_vs,hit)) return HTVSCROLL;
    } break;
    case WM_NCLBUTTONDOWN: {
        RECT rc_vsthumb;
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
        
        if((ES_MULTILINE&style)!=ES_MULTILINE) break;
        if(0!=Edit_GetZoneRect(hwnd,ZVSTHUMB,&rc_vsthumb,FALSE)) break;
        
        if(PtInRect(&rc_vsthumb,pt)) {
            SetCapture(hwnd);
            
            es->scroll_state=THUMB_VCLICK;
                    
            SCROLLINFO si={0};
            si.cbSize=sizeof(si);
            si.fMask=SIF_ALL;
            GetScrollInfo(hwnd,SB_VERT,&si);
            
            es->dragpos.y=si.nPos;
            es->dragpt.y=pt.y;
            
            return 0;
        }
    } break;
    case WM_NCLBUTTONUP:
    case WM_LBUTTONUP: {
        UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
        
        if((ES_MULTILINE&style)!=ES_MULTILINE) break;
        ReleaseCapture();
        es->scroll_state=SCROLL_NONE;
    } break;
    case WM_MOUSEMOVE: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
        RECT rc_vs,rc_vsthumb;
        
        if((ES_MULTILINE&style)!=ES_MULTILINE) break;
        
        ClientToScreen(hwnd,&pt);
        if(0!=Edit_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,FALSE)) break;
        if(0!=Edit_GetZoneRect(hwnd,ZVSTHUMB,&rc_vsthumb,FALSE)) break;
        
        if(GetCapture()==hwnd) {
            if(es->scroll_state==THUMB_VCLICK) {
                //垂直滚动
                SCROLLINFO si={0};
                si.cbSize=sizeof(si);
                si.fMask=SIF_ALL;
                GetScrollInfo(hwnd,SB_VERT,&si);
                int cur_pos=es->dragpos.y+(pt.y-es->dragpt.y)*1.0/((rc_vs.bottom-rc_vs.top)-(rc_vsthumb.bottom-rc_vsthumb.top))*(si.nMax-si.nMin+1-si.nPage);
                if(cur_pos>si.nMax-si.nMin+1-si.nPage) cur_pos=si.nMax-si.nMin+1-si.nPage;
                else if(cur_pos<si.nMin) cur_pos=si.nMin;
                SendMessage(hwnd,WM_VSCROLL,MAKEWPARAM(SB_THUMBPOSITION,cur_pos),0);
            }
        }
    } break;
    case WM_NCDESTROY: {
        Edit_ClearSettings(hwnd);
    }
    }
    return CallWindowProc(es->pre_proc,hwnd,msg,wParam,lParam);
}

int Edit_MultiLineAppend(HWND hwnd,char* appendstr)
{
    int multi_length=Edit_GetTextLength(hwnd);
    Edit_SetSel(hwnd,multi_length,multi_length);
    return 0;
}

BOOL Edit_IsShowVScroll(HWND hwnd)
{
    SCROLLINFO si={0};
    //垂直滚动条
    BOOL vscroll_flag=(WS_VSCROLL==((WS_VSCROLL)&(UINT)GetWindowLongPtr(hwnd,GWL_STYLE)));
    
    if(!vscroll_flag) return FALSE;
    
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_VERT,&si);
    if(si.nPage>=si.nMax-si.nMin+1) {//无需绘制垂直滚动条
        return FALSE;
    }
    else return TRUE;
}

BOOL Edit_IsShowHScroll(HWND hwnd)
{
    SCROLLINFO si={0};
    //垂直滚动条
    BOOL hscroll_flag=(WS_HSCROLL)&GetWindowLongPtr(hwnd,GWL_STYLE);
    if(!hscroll_flag) return FALSE;
    
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_HORZ,&si);
    if(si.nPage>=si.nMax-si.nMin+1) {//无需绘制垂直滚动条
        return FALSE;
    }
    else return TRUE;
}

int Edit_DrawVScrollBar(HWND hwnd)
{
    RECT rc_vs={0},rc_thumb={0},rc_mem={0};
    HDC hdc,memdc;
    HBITMAP bmp,pre_bmp;
    HBRUSH brush,pre_brush;
    HPEN pen,pre_pen;
    pEditStyle es=Edit_GetSettings(hwnd);
    BOOL is_vscrollbar=Edit_IsShowVScroll(hwnd);
    
    if(!is_vscrollbar) return 0;
        
    if(0!=Edit_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,TRUE)) return -1;
    if(0!=Edit_GetZoneRect(hwnd,ZVSTHUMB,&rc_thumb,TRUE)) return -1;
    
    hdc=GetWindowDC(hwnd);
    memdc=CreateCompatibleDC(hdc);
    CopyRect(&rc_mem,&rc_vs);
    OffsetRect(&rc_mem,-rc_mem.left,-rc_mem.top);
    bmp=CreateCompatibleBitmap(hdc,rc_mem.right,rc_mem.bottom);
    pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    brush=CreateSolidBrush(es->color_bk);
    pre_brush=(HBRUSH)SelectObject(memdc,brush);
    pen=CreatePen(PS_SOLID,1,es->color_bk);
    pre_pen=(HPEN)SelectObject(memdc,pen);
    
    //绘制滚动区
    Rectangle(memdc,rc_mem.left,rc_mem.top,rc_mem.right,rc_mem.bottom);
    
    //绘制thumb块
    OffsetRect(&rc_thumb,-rc_vs.left,-rc_vs.top);    
    if(is_vscrollbar) {
        Graphics graphic(memdc);
        graphic.SetSmoothingMode(SmoothingModeHighQuality);
        GraphicsPath path;
        
        LinearGradientBrush pbrush(Rect(rc_thumb.left,rc_thumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS),
                                  Color(255,15,15,15),Color(255,40,40,40),LinearGradientModeHorizontal);
        path.AddArc(rc_thumb.left,rc_thumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,
                    -180,180);
        path.AddArc(rc_thumb.left,rc_thumb.bottom-SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,
                    0,180);
        graphic.FillPath(&pbrush,&path);
    }
    
    //贴图
    BitBlt(hdc,rc_vs.left,rc_vs.top,rc_vs.right-rc_vs.left,rc_vs.bottom-rc_vs.top,
           memdc,0,0,SRCCOPY);
    
    DeleteObject(SelectObject(memdc,pre_pen));
    DeleteObject(SelectObject(memdc,pre_brush));
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    ReleaseDC(hwnd,hdc);
    
    return 0;
}

int Edit_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL is_allignuptop)
{
    RECT rc;
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL IsVscroll=Edit_IsShowVScroll(hwnd);
    BOOL IsBorder=((style&WS_BORDER)==WS_BORDER);
    pEditStyle es=Edit_GetSettings(hwnd);
    
    if(!es) return -1;
    
    GetWindowRect(hwnd,&rc);
    
    switch(zone) {
    case ZVSCROLL: {
        if(!IsVscroll) return -1;
        
        prc->top=rc.top+(IsBorder?es->margin_border:0);
        prc->left=rc.right-(IsBorder?es->margin_border:0)-SCROLLBAR_PIXLS;
        prc->right=prc->left+SCROLLBAR_PIXLS;
        prc->bottom=rc.bottom-(IsBorder?es->margin_border:0);
    } break;
    case ZVSTHUMB: {
        RECT rc_vs;  
        int thumb_pixls,thumb_pos;
        SCROLLINFO si={0};
        
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_VERT,&si);
        
        if(Edit_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,FALSE)!=0) return -1;
        //设置拇指区
        thumb_pixls=(rc_vs.bottom-rc_vs.top)*1.0*si.nPage/(si.nMax-si.nMin+1);
        if(thumb_pixls<THUMB_MIN_PIXLS) thumb_pixls=THUMB_MIN_PIXLS;
        thumb_pos=(rc_vs.bottom-rc_vs.top-thumb_pixls)*1.0*si.nPos/(si.nMax-si.nMin-si.nPage+1);
        prc->top=rc_vs.top+thumb_pos;
        prc->bottom=prc->top+thumb_pixls;
        prc->left=rc_vs.left;
        prc->right=rc_vs.right;  
    } break;
    }
    
    if(is_allignuptop) OffsetRect(prc,-rc.left,-rc.top);
    
    return 0;
}
