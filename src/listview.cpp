#include "../include/listview.h"

int ListView_InitialSettings(HWND hwnd)
{
    pListViewStyle ls=(pListViewStyle)calloc(sizeof(RListViewStyle),1);
    if(!ls) return -1;
    
    ls->proc=LVCProc;
    ls->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ls->proc);  
    
    /*
    HFONT font=CreateFont(12,0,0,0,
                    FW_SEMIBOLD,//FW_MEDIUM,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "楷体");
    */
    LOGFONT lf={0};
    GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
    lf.lfHeight=18;
    lf.lfWeight=25;
    strcpy(lf.lfFaceName,"黑体");
    HFONT font=CreateFontIndirect(&lf);
    SendMessage(hwnd,WM_SETFONT,(WPARAM)font,0);                
    ls->color_text=RGB(250,250,250);
    ls->rgb_sel=RGB(18,25,40);
    ls->rgb_odd=RGB(15,15,15);//RGB(57,59,61);
    ls->rgb_even=RGB(25,25,25);//RGB(50,51,51);
    ls->rgb_focus=RGB(7,7,7);
    ls->content_height=28;
    
    ls->col_index=-1;
    ls->ptCur={-1,-1};
    
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)ls);
    if((LVS_OWNERDRAWFIXED&((UINT)GetWindowLongPtr(hwnd,GWL_STYLE)))!=LVS_OWNERDRAWFIXED) {
        SetWindowLongPtr(hwnd,GWL_STYLE,LVS_OWNERDRAWFIXED|((UINT)GetWindowLongPtr(hwnd,GWL_STYLE)));
    }
    
#ifdef FUCKOFFSWITCH
    ls->fuckoff_switch=TRUE;
    strcpy(ls->fuckoff_sentence,""/*"上述内容不便展示，祝你好运！"*/);
#endif
    
    return 0;
}

inline pListViewStyle ListView_GetSettings(HWND hwnd)
{
    return (pListViewStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int ListView_ClearSettings(HWND hwnd)
{
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
    if(ls->font) DeleteObject(ls->font);    
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ls->pre_proc);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,NULL);
    
    free(ls);
    return 0;
}    

//描述：listview 内容部分的窗口过程
LRESULT CALLBACK LVCProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return 0;
            
    switch(message){
    case WM_NCCALCSIZE: {
        return LVC_NCCalcSize(hwnd,wParam,lParam);
    } break;
    case WM_NCACTIVATE:
    case WM_NCPAINT: {
        LVC_NCDrawScrollBar(hwnd,SB_HORZ);
        LVC_NCDrawScrollBar(hwnd,SB_VERT);
        return TRUE;
    } break;
    case WM_ERASEBKGND: {
        return TRUE;
    } break;
    case WM_STYLECHANGING:
    case WM_STYLECHANGED:
    case LVM_INSERTITEM:
    case LVM_DELETEITEM:
    case LVM_DELETEALLITEMS:
    case WM_NOTIFY: {
        return (HRESULT)LVC_ResetScrollPos(hwnd,message,wParam,lParam);
    } break; 
    case WM_MOUSEWHEEL: {
        int wheel_delta=GET_WHEEL_DELTA_WPARAM(wParam);
        int scroll_pixls=((double)wheel_delta/10)*5*-1;
        UINT style=GetWindowLongPtr(hwnd,GWL_STYLE);
        BOOL HasVScroll=((style&WS_VSCROLL)==WS_VSCROLL);//垂直
        BOOL HasHScroll=((style&WS_HSCROLL)==WS_HSCROLL);//水平
        
        SendMessage(hwnd,LVM_SCROLL,(WPARAM)((!HasVScroll&&HasHScroll)?wheel_delta:0),(LPARAM)(HasVScroll?scroll_pixls:0));
        return TRUE;
    } break;
    case WM_SETFOCUS:
        InvalidateRect(hwnd,NULL,TRUE);
    break;        
    case WM_PAINT: {
        LVC_Paint(hwnd,wParam,lParam);
        
        //modify 2021-12-23
        LVC_NCDrawScrollBar(hwnd,SB_HORZ);
        LVC_NCDrawScrollBar(hwnd,SB_VERT);
        return TRUE;
    } break;
    case WM_NCLBUTTONUP:
        ReleaseCapture();
    break;
    case WM_SIZE: {
        LVC_Size(hwnd,wParam,lParam);
        return TRUE;
    } break;
    case LVM_SCROLL: {
        //原函数会自动绘制非客户区，直接调用将导致闪烁。
        //自行绘制，依然会导致闪烁
        LVC_Scroll(hwnd,wParam,lParam);
        return TRUE;
    } break;
    case LVM_FINDITEM: {
        return LVC_FindItem(hwnd,wParam,lParam);
    } break;
    case LVM_ENSUREVISIBLE: {
        return LVC_EnsureVisible(hwnd,wParam,lParam);
    } break;
    case LVM_SETITEMTEXT: {
        RECT rc;
        //ListView_GetItemRect(hwnd,wParam,&rc,LVIR_BOUNDS);
        InvalidateRect(hwnd,NULL,TRUE);
    } break;
    case LVM_GETITEMRECT: {
        return LVC_GetItemRect(hwnd,wParam,lParam);
    } break;
    case WM_NCHITTEST: {
        return LVC_HitTest(hwnd,wParam,lParam);
    }
    case WM_NCLBUTTONDOWN: {
        return LVC_ScrollBefore(hwnd,wParam,lParam);
    } break;
    case WM_SETCURSOR: {
        return LVC_SetCursor(hwnd,wParam,lParam);
    } break;
    case WM_LBUTTONDOWN: {
        return LVC_LButtonDown(hwnd,wParam,lParam);
    } break;
    case WM_MOUSEMOVE: {
        return LVC_MouseMove(hwnd,wParam,lParam);
    } break;
    case WM_NCMOUSEMOVE: {
        LVC_NCDrawScrollBar(hwnd,SB_HORZ);
        LVC_NCDrawScrollBar(hwnd,SB_VERT);
        return 0;
    } break;
    case WM_LBUTTONUP: {
        LVC_LButtonUp(hwnd,wParam,lParam);
    } break;/*
    case WMYU_DRAWLISTVIEWITEM: {
        LVC_DrawItem(hwnd,wParam,lParam);
        return TRUE;
    } break;*/
    case WMYU_MEASURELISTVIEWITEM: {
        LPMEASUREITEMSTRUCT lpmis=(LPMEASUREITEMSTRUCT)lParam;
        lpmis->itemHeight=ls->content_height;
    } break;
    case WM_NCDESTROY: {
        WNDPROC pre_proc=ls->pre_proc;
        ListView_ClearSettings(hwnd);
        if(pre_proc) return CallWindowProc(pre_proc,hwnd,message,wParam,lParam);
    } break;
    }
    
    return CallWindowProc(ls->pre_proc,hwnd,message,wParam,lParam);
}

int LVC_LButtonDown(HWND hwnd,WPARAM wParam,LPARAM lParam)
{    
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
    
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    SetFocus(hwnd);
    RECT rcTitle;
    LVC_GetZoneRect(hwnd,ZLVTITILE,&rcTitle,TRUE);
    if(PtInRect(&rcTitle,pt)) {
        HWND head=ListView_GetHeader(hwnd);
        int itm_count=ListView_GetItemCount(hwnd);
        int col_count=Header_GetItemCount(head);
        RECT rcItem;
        int x_offset=0;
        
        UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
        BOOL HasHScroll=((style&WS_HSCROLL)==WS_HSCROLL);
        
        if(HasHScroll) {
            SCROLLINFO si={0};
            si.cbSize=sizeof(si);
            si.fMask=SIF_ALL;
            GetScrollInfo(hwnd,SB_HORZ,&si);
            x_offset=si.nPos;
        }
        int idx=0;
        while(idx<col_count) {
            Header_GetItemRect(head,idx,&rcItem);
            OffsetRect(&rcItem,-x_offset,0);
            
            if(abs(rcItem.right-pt.x)<5) {
                //分割条
                SetCursor(LoadCursor(NULL,IDC_SIZEWE));
                ls->col_index=idx;
                break;
            }
            idx++;
        }
        if(idx>=col_count) ls->col_index=-1;  
    }
    else {
        LVC_SetState(hwnd,wParam,lParam);
    }
    return 0;
}

int LVC_LButtonUp(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    LVC_ScrollDone(hwnd,wParam,lParam);
    
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
        
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    RECT rcTitle;
    LVC_GetZoneRect(hwnd,ZLVTITILE,&rcTitle,TRUE);
    
    if(ls->col_index!=-1) {
        ListView_SetColumnWidth(hwnd,ls->col_index,ls->col_width);
        ls->col_index=-1;
        InvalidateRect(hwnd,&rcTitle,TRUE);
    }
    return 0;
}

int LVC_SetCursor(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
    UINT hit_pos=LOWORD(lParam);
    UINT trigger_msg=HIWORD(lParam);
    
    if(!(hit_pos==HTVSCROLL||hit_pos==HTHSCROLL))
    return CallWindowProc(ls->pre_proc,hwnd,WM_SETCURSOR,wParam,lParam);
    
    if(trigger_msg!=WM_LBUTTONDOWN&&trigger_msg!=WM_NCLBUTTONDOWN) 
    return CallWindowProc(ls->pre_proc,hwnd,WM_SETCURSOR,wParam,lParam);
    
    POINT pt={0};
    GetCursorPos(&pt);
    ScreenToClient(hwnd,&pt);
    
    RECT rcHS,rcVS;
    LVC_GetZoneRect(hwnd,ZHSCROLL,&rcHS,TRUE);
    LVC_GetZoneRect(hwnd,ZVSCROLL,&rcVS,TRUE);
    
    if(PtInRect(&rcHS,pt)||PtInRect(&rcVS,pt)) {
        SetCapture(hwnd);
        LVC_NCDrawScrollBar(hwnd,SB_HORZ);
        LVC_NCDrawScrollBar(hwnd,SB_VERT);
    }
    return 0;
}

int LVC_MouseMove(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    if(GetCapture()==hwnd) {
        LVC_Scrolling(hwnd,wParam,lParam);
        return 1;
    }
 
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
            
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    RECT rcTitle;
    LVC_GetZoneRect(hwnd,ZLVTITILE,&rcTitle,TRUE);
    BOOL IsMousePressed=((wParam&MK_LBUTTON)==MK_LBUTTON);
    HWND head=ListView_GetHeader(hwnd);
    int itm_count=ListView_GetItemCount(hwnd);
    int col_count=Header_GetItemCount(head);
    RECT rcItem;
    
    ls->ptCur=pt;
    InvalidateRect(hwnd,NULL,TRUE);
    
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL HasHScroll=((style&WS_HSCROLL)==WS_HSCROLL);
    int x_offset=0;
    if(HasHScroll) {
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        x_offset=si.nPos;
    }
    
    //鼠标滑过
    if(ls->col_index==-1&&!IsMousePressed&&PtInRect(&rcTitle,pt)) {
        for(int idx=0;idx<col_count;idx++) {
            Header_GetItemRect(head,idx,&rcItem);
            OffsetRect(&rcItem,-x_offset,0);
            
            //分割条
            if(abs(rcItem.right-pt.x)<5) {
                SetCursor(LoadCursor(NULL,IDC_SIZEWE));
                break;
            }
        }
    }
    else //按下分割条拖拽宽度
    if(ls->col_index!=-1) {
        SetCursor(LoadCursor(NULL,IDC_SIZEWE));
        Header_GetItemRect(head,ls->col_index,&rcItem);
        OffsetRect(&rcItem,-x_offset,0);
        ls->col_width=pt.x-rcItem.left;
        
        char col_text[256]="";
        HDITEM hdi={0};
        hdi.mask=HDI_TEXT;
        hdi.cchTextMax=sizeof(col_text);
        hdi.pszText=col_text;
        Header_GetItem(head,ls->col_index,&hdi);
        if(strstr(col_text,"（賬）")) {
            ls->col_width=250;
        }
        
        InvalidateRect(hwnd,&rcTitle,TRUE);   
    }
    
    return 1;
}

int LVC_NCCalcSize(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    RECT rect_new;
    RECT rect_old;
    RECT client_rect_new;
    RECT client_rect_old;
    RECT rc_hs;
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL HasHScroll=((style&WS_HSCROLL)==WS_HSCROLL);
    BOOL HasVScroll=((style&WS_VSCROLL)==WS_VSCROLL);
    BOOL HasBorder=((style&WS_BORDER)==WS_BORDER);
    
    if(wParam == TRUE) {
        LPNCCALCSIZE_PARAMS calc_param = (LPNCCALCSIZE_PARAMS)lParam;
        
        CopyRect(&rect_new,&(calc_param->rgrc[0]));
        CopyRect(&rect_old,&(calc_param->rgrc[1]));
        CopyRect(&client_rect_old,&(calc_param->rgrc[2]));
        
        client_rect_new = {rect_new.left+(HasBorder?1:0),
                           rect_new.top+(HasBorder?1:0),
                           rect_new.right-((HasBorder?1:0))-(HasVScroll?SCROLLBAR_PIXLS:0),
                           rect_new.bottom-((HasBorder?1:0))-(HasHScroll?SCROLLBAR_PIXLS:0)};
        CopyRect(&(calc_param->rgrc[0]),&client_rect_new);
        CopyRect(&(calc_param->rgrc[1]),&rect_new);
        CopyRect(&(calc_param->rgrc[2]),&rect_old);
        
        return WVR_VALIDRECTS;
    }
    else {
        RECT* prect = (RECT*)lParam;
        prect->right-=(HasVScroll?SCROLLBAR_PIXLS:0)+(HasBorder?1:0); 
        prect->bottom-=(HasHScroll?SCROLLBAR_PIXLS:0)+(HasBorder?1:0);
        prect->left+=(HasBorder?1:0);
        prect->top+=(HasBorder?1:0);
        
        return 0;
    }
}

int LVC_ResetScrollPos(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
    
    SCROLLINFO siH={0},siV={0},siAfter={0};
    siH.cbSize=siV.cbSize=sizeof(siH);
    siH.fMask=siV.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_HORZ,&siH);
    GetScrollInfo(hwnd,SB_VERT,&siV);
    
    LRESULT ret=CallWindowProc(ls->pre_proc,hwnd,message,wParam,lParam);
    
    siAfter.cbSize=sizeof(siAfter);
    siAfter.fMask=SIF_POS;
    siAfter.nPos=siH.nPos;
    SetScrollInfo(hwnd,SB_HORZ,&siAfter,TRUE);
    siAfter.nPos=siV.nPos;
    SetScrollInfo(hwnd,SB_VERT,&siAfter,TRUE);
    
    InvalidateRect(hwnd,NULL,TRUE);
    LVC_NCDrawScrollBar(hwnd,SB_HORZ);
    LVC_NCDrawScrollBar(hwnd,SB_VERT);
    
    return (int)ret;
}

int LVC_Size(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    UINT width=LOWORD(lParam);
    UINT height=HIWORD(lParam);
    
    //head位置
    RECT rcTitle={0};
    GetWindowRect(ListView_GetHeader(hwnd),&rcTitle);
    int title_height=rcTitle.bottom-rcTitle.top;
    HWND head=ListView_GetHeader(hwnd);
    //MoveWindow(head,0,0,rcTitle.right-rcTitle.left,title_height,TRUE);
    
    //获取外接参数
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
    
    //设置page
    UINT style=GetWindowLongPtr(hwnd,GWL_STYLE);   
    BOOL HasVScroll=((style&WS_VSCROLL)==WS_VSCROLL);
    BOOL HasHScroll=((style&WS_HSCROLL)==WS_HSCROLL);  
    SCROLLINFO si={0};
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    if(HasVScroll) {//垂直滚动条
        GetScrollInfo(hwnd,SB_VERT,&si);
        si.nPage=(height-title_height)/ls->content_height;
        si.fMask=SIF_PAGE|SIF_POS;
        si.nPos=si.nPos;
        SetScrollInfo(hwnd,SB_VERT,&si,FALSE);
    }
    if(HasHScroll) {
        GetScrollInfo(hwnd,SB_HORZ,&si);
        si.nPage=width;
        si.fMask=SIF_PAGE;
        SetScrollInfo(hwnd,SB_HORZ,&si,TRUE);
    }
    
    return 0;
}

int LVC_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    int idx_item=0;
    int page=0;
    int x_offset=0,y_offset=0;
    char item_text[256]="";//子项文本
    
    //获取外接参数
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
    
    //获取head参数    
    HWND head=ListView_GetHeader(hwnd);
    SetWindowLongPtr(head,GWL_STYLE,(~WS_VISIBLE)&((UINT)GetWindowLongPtr(head,GWL_STYLE)));
    
    int itm_count=ListView_GetItemCount(hwnd);
#ifdef FUCKOFFSWITCH
    if(ls->fuckoff_switch) itm_count=0;
#endif
    int col_count=Header_GetItemCount(head);
    int col_start=0,col_stop=col_count-1;
    RECT rcHead={0},rcClient;
    pLVHeadStyle hs=LVHead_GetSettings(head);
    if(!hs) return 0;
        
    int head_height=hs->title_height;
    GetClientRect(hwnd,&rcClient);
    SetRect(&rcHead,0,0,rcClient.right-rcClient.left,head_height);
    
    //获取x,y轴向偏移
    SCROLLINFO si={0};
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    UINT style=GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL HasVScroll=((style&WS_VSCROLL)==WS_VSCROLL);
    BOOL HasHScroll=((style&WS_HSCROLL)==WS_HSCROLL);
    if(HasVScroll) {
        GetScrollInfo(hwnd,SB_VERT,&si);//垂直滚动条
        //获取绘制item的起始位及页面范围
        idx_item=si.nPos;
        page=si.nPage;
    }
    else { 
        page=ListView_GetItemCount(hwnd);
    }
    
    if(HasHScroll) {
        GetScrollInfo(hwnd,SB_HORZ,&si);
        //获取绘制item的x轴向位移
        x_offset=si.nPos;
    }
    y_offset=head_height;
    
    //准备绘制环境
    PAINTSTRUCT ps;
    HDC hdc=BeginPaint(hwnd,&ps);
    int cx=ps.rcPaint.right-ps.rcPaint.left;
    int cy=ps.rcPaint.bottom-ps.rcPaint.top;
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    HFONT pre_font=(HFONT)SelectObject(memdc,(HFONT)SendMessage(hwnd,WM_GETFONT,0,0));
    SetBkMode(memdc,TRANSPARENT);
    HPEN pen=CreatePen(PS_SOLID,1,RGB(40,46,44));
    SelectObject(memdc,pen);
    
    //内存绘制区域
    RECT rcMem;
    CopyRect(&rcMem,&ps.rcPaint);
    
    //对齐缓冲DC与界面DC的坐标
    POINT pt_org;
    OffsetViewportOrgEx(memdc,-rcMem.left,-rcMem.top,&pt_org);
    
    RECT rcItem;//子项区域
    COLORREF rgb_curr;
    BOOL col_trigger=FALSE;//column绘制停止标记
    int rcTop=0,rcBottom=0;
    
    //填充背景
    HBRUSH bkbrush=CreateSolidBrush(RGB(20,20,20));
    FillRect(memdc,&rcMem,bkbrush);
    DeleteObject(bkbrush);
    
    if(itm_count<=0) {
        OffsetRect(&rcClient,-x_offset,0);
        POINT ptCenter={(rcClient.right+rcClient.left)/2,(rcClient.bottom+rcClient.top+head_height)/2};
        SetRect(&rcClient,ptCenter.x-150,ptCenter.y-20,ptCenter.x+150,ptCenter.y+20);
        HBRUSH brushNote=CreateSolidBrush(RGB(120,0,11));
        HBRUSH pre_brush=(HBRUSH)SelectObject(memdc,brushNote);
        Rectangle(memdc,rcClient.left,rcClient.top,rcClient.right,rcClient.bottom);
        DeleteObject(SelectObject(memdc,pre_brush));
        int size=32;//icon size
        DrawIconEx(memdc,rcClient.left+(40-size)/2,rcClient.top+(40-size)/2,(HICON)LoadIcon(NULL,IDI_WARNING),size,size,0,0,DI_NORMAL);
        SetTextColor(memdc,RGB(0,125,255));
#ifndef FUCKOFFSWITCH
        DrawText(memdc,"<No Data Record.>",-1,&rcClient,DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS);
#else
        if(ls->fuckoff_switch)
            DrawText(memdc,ls->fuckoff_sentence,-1,&rcClient,DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS); 
        else DrawText(memdc,"<No FUCKING DATA. Fuck Zdd>",-1,&rcClient,DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS);
#endif
        goto TITLE;
    }
    
    SetTextColor(memdc,RGB(255,255,255));    
    //更新无效区域
    for(int index=0;index<=page&&(index+idx_item<itm_count);index++) {
        //计算rcItem的y向坐标
        rcTop=index*ls->content_height+y_offset;
        rcBottom=rcTop+ls->content_height;
        //获取Item ID
        int itemID=index+idx_item;
        
        //对区域内的Item进行绘制操作。
        //优化操作，仅绘制需要绘制的部分
        if(rcTop>=ps.rcPaint.top&&rcTop<=ps.rcPaint.bottom) {
            //选中标记
            UINT state=ListView_GetItemState(hwnd,itemID,LVIS_SELECTED|LVIS_FOCUSED);
            
            //设置行背景色                
            /*if(itemID%2==0)*/ rgb_curr=ls->rgb_even;
            //else rgb_curr=ls->rgb_odd;
            if(state&LVIS_SELECTED==LVIS_SELECTED) rgb_curr=ls->rgb_sel;
            if(state&LVIS_FOCUSED==LVIS_FOCUSED) rgb_curr=ls->rgb_focus;
            HBRUSH brush=CreateSolidBrush(rgb_curr);
            HBRUSH pre_brush=(HBRUSH)SelectObject(memdc,brush);
            
            //绘制item及subitem.
            for(int idx_col=col_start;idx_col<=col_stop;idx_col++) {  
                ListView_GetSubItemRect(hwnd,itemID,idx_col, LVIR_LABEL,&rcItem);
                //x,y轴向位移
                OffsetRect(&rcItem,-x_offset,-idx_item*ls->content_height);
                
                //初始化需要绘制的column,排除无关绘制项目
                if(ps.rcPaint.left>rcItem.right) continue;
                else if(!col_trigger){
                    col_start=idx_col;
                    col_trigger=TRUE;
                }
                if(ps.rcPaint.right<rcItem.left) {
                    col_stop=idx_col-1;
                    break;
                }
                
                //分离Cell的绘制
                //子项绘制
                LVC_PaintItem(hwnd,memdc,&rcItem,itemID,idx_col);
            }
            DeleteObject(brush);
        }

    }
    
    TITLE:
    //画title
    HDITEM item={0};
    char col_title[256]="";
    if(ps.rcPaint.top<=(rcHead.top+rcHead.bottom)/2) {
        COLORREF rgb_1=RGB(0,0,0),rgb_2=RGB(30,30,30);
        GradientRect2(memdc,rcHead,rgb_1,rgb_2,1);            
        
        COLORREF color_1=RGB(10,10,10),color_2=RGB(40,40,40);
        HPEN sp_line1=CreatePen(PS_SOLID,1,color_1);
        HPEN sp_line2=CreatePen(PS_SOLID,1,color_2);
        HPEN penCol=CreatePen(PS_SOLID,3,RGB(0,125,255));
        for(int idx_col=col_start;idx_col<=col_stop;idx_col++) {
            Header_GetItemRect(head,idx_col,&rcItem);
            OffsetRect(&rcItem,-x_offset,0); 
            
            if(PtInRect(&rcItem,ls->ptCur)) {
                GradientRect2(memdc,rcItem,RGB(10,10,10),RGB(40,40,40),1);
            }
            
            item.mask=HDI_TEXT|HDI_STATE|HDI_WIDTH;
            item.pszText=col_title;
            item.cchTextMax=sizeof(col_title);
            Header_GetItem(head,idx_col,&item);
            
            if(strstr(col_title,"（賬）")) {
                *strstr(col_title,"（賬）")='\0';
                POINT pt[3]={0};
                RECT rcTotal={rcItem.left,rcItem.top,rcItem.right,(rcItem.top+rcItem.bottom)>>1};
                DrawText(memdc,col_title,-1,&rcTotal,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
                SelectObject(memdc,sp_line2);
                MoveToEx(memdc,rcTotal.left,rcTotal.bottom,&(pt[2]));
                LineTo(memdc,rcTotal.right,rcTotal.bottom);
                
                char scale[][4]={"佰","拾","兆","仟","佰","拾","亿","仟","佰","拾","萬","仟","佰","拾","圆","角","分"};
                char spscale[]="兆亿萬圆";
                int scale_count=sizeof(scale)/sizeof(char[4]);
                int cell_cx=13;
                int idx=0;
                COLORREF preColor;
                //HPEN line=CreatePen(PS_SOLID,1,RGB(15,115,10));
                //HPEN prePen=(HPEN)SelectObject(memdc,line);
                while(idx<scale_count) {
                    pt[0].x=rcItem.right-(idx+1)*cell_cx-2;
                    if(pt[0].x<rcItem.left) break;
                    pt[0].y=rcTotal.bottom;
                    pt[1].x=pt[0].x;
                    pt[1].y=rcItem.bottom;
                    RECT rcScale={pt[0].x,pt[0].y,pt[0].x+cell_cx,pt[1].y};
                        
                    MoveToEx(memdc,pt[0].x,pt[0].y,&(pt[2]));
                    LineTo(memdc,pt[1].x,pt[1].y);
                    if(strstr(spscale,scale[scale_count-1-idx])) preColor=SetTextColor(memdc,RGB(0,255/sizeof(spscale)*(strstr(spscale,scale[scale_count-1-idx])-spscale+2),255/sizeof(spscale)*2*((strstr(spscale,scale[scale_count-1-idx])-spscale+2))));
                    DrawText(memdc,scale[scale_count-1-idx],-1,&rcScale,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
                    if(strstr(spscale,scale[scale_count-1-idx])) SetTextColor(memdc,preColor);
                    
                    idx++;
                }
                
                if(idx==scale_count) {
                    //SelectObject(hdc,lineBold);
                    MoveToEx(memdc,pt[0].x-2,pt[0].y,&(pt[2]));
                    LineTo(memdc,pt[1].x-2,pt[1].y); 
                    RECT rcImg;
                    SetRect(&rcImg,rcItem.left,pt[0].y,pt[0].x,pt[1].y);
                    int ico_cx=16;
                    int ptx=((rcImg.right-rcImg.left-ico_cx)>>1)+rcImg.left,pty=((rcImg.bottom-rcImg.top-ico_cx)>>1)+rcImg.top;
                    HICON icon=ImageList_GetIcon(ls->imglist,3,ILD_NORMAL);
                    DrawIconEx(memdc,ptx,pty,icon,ico_cx,ico_cx,0,0,DI_NORMAL);
                    DestroyIcon(icon);         
                }
                //DeleteObject(SelectObject(memdc,prePen));
            }
            else {
                DrawText(memdc,col_title,-1,&rcItem,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
            }
            //绘制立体分割线
            POINT pt[5]={{rcItem.right-2,rcItem.top},
                         {rcItem.right-2,rcItem.bottom},
                         {rcItem.right-1,rcItem.top},
                         {rcItem.right-1,rcItem.bottom}
                         };
            
            SelectObject(memdc,sp_line1);
            MoveToEx(memdc,pt[0].x,pt[0].y,&(pt[4]));
            LineTo(memdc,pt[1].x,pt[1].y);
            
            SelectObject(memdc,sp_line2);
            MoveToEx(memdc,pt[2].x,pt[2].y,&(pt[4]));
            LineTo(memdc,pt[3].x,pt[3].y);
            
            if(idx_col==ls->col_index&&ls->col_index!=-1) {
                if(ls->col_width<5) ls->col_width=5;
                POINT ptCol[3]={{rcItem.left+ls->col_width,rcItem.top},
                                {rcItem.left+ls->col_width,rcItem.bottom}};
                SelectObject(memdc,penCol);
                MoveToEx(memdc,ptCol[0].x,ptCol[0].y,&ptCol[2]);
                LineTo(memdc,ptCol[1].x,ptCol[1].y);
                
                char test_log[256]="";
                sprintf(test_log,"width:%d",ls->col_width);
                DrawText(memdc,test_log,-1,&rcItem,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
            }
        }
        DeleteObject(sp_line1);
        DeleteObject(sp_line2);
        DeleteObject(penCol);
    }
    //贴图
    BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,cx,cy,
           memdc,rcMem.left,rcMem.top,SRCCOPY);
    
    //取消对齐
    SetViewportOrgEx(memdc,pt_org.x,pt_org.y,NULL);
    
    //资源归位及释放
    SelectObject(memdc,pre_font);
    DeleteObject(pen);
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    
    //结束绘制
    EndPaint(hwnd,&ps);
    
    return 0;    
}

inline int LVC_PaintItem(HWND hwnd,HDC hdc,LPRECT prc,UINT itemid,int subitem)
{
    RECT rcItem;
    char item_text[256]="";
    
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
        
    CopyRect(&rcItem,prc);
    Rectangle(hdc,rcItem.left-1,rcItem.top-1,rcItem.right,rcItem.bottom);
    memset(item_text,0x00,sizeof(item_text));
    ListView_GetItemText(hwnd,itemid,subitem,item_text,sizeof(item_text));
    InflateRect(&rcItem,-5,-2);
    
    HWND head=ListView_GetHeader(hwnd);
    char title_text[256]="";
    HDITEM hdi={0};
    hdi.mask=HDI_TEXT;
    hdi.cchTextMax=sizeof(title_text);
    hdi.pszText=title_text;
    Header_GetItem(head,subitem,&hdi);
    
    if(PtInRect(prc,ls->ptCur)) {
        HBRUSH active_brush=CreateSolidBrush(RGB(15,15,15));
        HBRUSH pre_brush=(HBRUSH)SelectObject(hdc,active_brush);
        Rectangle(hdc,prc->left,prc->top,prc->right,prc->bottom);
        DeleteObject(SelectObject(hdc,pre_brush));
    }
    
    if(strstr(title_text,"操作")){ 
        RECT rcBtn,rcEdit,rcDetail,rcDelete;
        int cx=20,cy=20;
        SetRect(&rcBtn,rcItem.left,rcItem.top+((rcItem.bottom-rcItem.top-cy)>>1),rcItem.left+cx,(rcItem.top+((rcItem.bottom-rcItem.top-cy)>>1))+cy);
        SetRect(&rcEdit,rcBtn.right+5,rcBtn.top,rcBtn.right+5+cx,rcBtn.top+cy);
        SetRect(&rcDelete,rcEdit.right+5,rcEdit.top,rcEdit.right+5+cx,rcEdit.top+cy);
        
        if(PtInRect(&rcBtn,ls->ptCur)) {
            RoundRect(hdc,rcBtn.left,rcBtn.top,rcBtn.right+1,rcBtn.bottom+1,3,3);
            GradientRect2(hdc,rcBtn,RGB(25,25,25),RGB(50,50,50),1);
        }
        HICON iDet=ImageList_GetIcon(ls->imglist,2,ILD_NORMAL);
        DrawIconEx(hdc,rcBtn.left,rcBtn.top,iDet,cx,cy,0,0,DI_NORMAL);
        DestroyIcon(iDet);
        //ImageList_Draw(ls->imglist,2,hdc,rcBtn.left,rcBtn.top,ILD_NORMAL);
        
        if(PtInRect(&rcEdit,ls->ptCur)) {
            RoundRect(hdc,rcEdit.left,rcEdit.top,rcEdit.right+1,rcEdit.bottom+1,3,3);
            GradientRect2(hdc,rcEdit,RGB(25,25,25),RGB(50,50,50),1);
        }
        HICON iEdit=ImageList_GetIcon(ls->imglist,1,ILD_NORMAL);
        DrawIconEx(hdc,rcEdit.left,rcEdit.top,iEdit,cx,cy,0,0,DI_NORMAL);
        DestroyIcon(iEdit);
        //ImageList_Draw(ls->imglist,1,hdc,rcEdit.left,rcEdit.top,ILD_NORMAL);
        
        if(PtInRect(&rcDelete,ls->ptCur)) {
            RoundRect(hdc,rcDelete.left,rcDelete.top,rcDelete.right+1,rcDelete.bottom+1,3,3);
            GradientRect2(hdc,rcDelete,RGB(255,0,0),RGB(50,5,5),1);
        }
        HICON iDel=ImageList_GetIcon(ls->imglist,0,ILD_NORMAL);
        DrawIconEx(hdc,rcDelete.left,rcDelete.top,iDel,cx,cy,0,0,DI_NORMAL);
        DestroyIcon(iDel);
        //ImageList_Draw(ls->imglist,0,hdc,rcDelete.left,rcDelete.top,ILD_NORMAL);
        
        rcItem.left=rcDelete.right+15;
        sprintf(item_text,"Row %05d",itemid);
        DrawText(hdc,item_text,-1,&rcItem,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
    }
    else if(strstr(title_text,"（賬）")){
        InflateRect(&rcItem,3,2);//矩形复位
        
        //16位数值，14-2
        char scale[][4]={"百","十","兆","千","百","十","亿","千","百","十","万","千","百","十","元","角","分"};
        int scale_count=sizeof(scale)/sizeof(char[4]);
        int cell_cx=13;
        POINT pt[3]={0};
        COLORREF preColor;
        BOOL NULLFLAG=FALSE;
        double value=0.0;
        char value_text[256]="";
        strcpy(value_text,item_text);
        if(value_text[0]=='\0') {
            strcpy(value_text,"0.00");
            NULLFLAG=TRUE;
        }
        if(1!=sscanf(value_text,"%lf",&value)) {
            int ico_cx=16;
            POINT ptIcon={rcItem.left+(rcItem.bottom-rcItem.top-ico_cx)/2,rcItem.top+(rcItem.bottom-rcItem.top-ico_cx)/2};
            RECT rcIcon={ptIcon.x,ptIcon.y,ptIcon.x+ico_cx,ptIcon.y+ico_cx};
            RECT rcText={rcIcon.right+5,rcItem.top,rcItem.right,rcItem.bottom};
            DrawIconEx(hdc,ptIcon.x,ptIcon.y,(HICON)LoadIcon(NULL,IDI_ERROR),ico_cx,ico_cx,0,0,DI_NORMAL);
            char text_msg[256]="";
            sprintf(text_msg,"Data conversion Failed. SRC：%s",value_text);
            DrawText(hdc,text_msg,-1,&rcText,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
            return 0;
        }
        
        if(value<0.0) {
            preColor=SetTextColor(hdc,RGB(255,0,0));
        }
        else {
            preColor=SetTextColor(hdc,RGB(0,100,200));
        }
        
        //定位小数点
        char* idx_point=strstr(value_text,".");
        if(!idx_point) {
            strcat(value_text,".00");
            idx_point=strstr(value_text,".");
        }
        //展示仅保留至分
        idx_point[0]=idx_point[1];
        idx_point[1]=idx_point[2];
        idx_point[2]='\0';
        
        int idx=0;
        RECT rcPre={0};
        RECT rcNum;
        pt[0].x=rcItem.right-1;
        pt[0].y=rcItem.top-1;
        pt[1].x=pt[0].x;
        pt[1].y=rcItem.bottom;
        MoveToEx(hdc,pt[0].x,pt[0].y,&(pt[2]));
        LineTo(hdc,pt[1].x,pt[1].y);
        while(idx<scale_count) {
            pt[0].x=rcItem.right-(idx+1)*cell_cx;
            if(pt[0].x<rcItem.left) {
                break;
            }
            pt[0].y=rcItem.top-1;
            pt[1].x=pt[0].x;
            pt[1].y=rcItem.bottom;
            CopyRect(&rcPre,&rcNum);
            SetRect(&rcNum,pt[0].x,pt[0].y,pt[0].x+cell_cx,pt[1].y);
            
            if(((int)strlen(value_text))-1-idx>=0) {
                char strNum[3]="";            
                strNum[0]=value_text[strlen(value_text)-1-idx];
                if(strNum[0]!='-') {
                    DrawText(hdc,strNum,-1,&rcNum,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
                }
            }
            MoveToEx(hdc,pt[0].x,pt[0].y,&(pt[2]));
            LineTo(hdc,pt[1].x,pt[1].y);
            
            idx++;
        }
        if(idx==scale_count) {
            MoveToEx(hdc,pt[0].x-2,pt[0].y,&(pt[2]));
            LineTo(hdc,pt[1].x-2,pt[1].y);
            
            if(value<0.0) {
                RECT rcNeg={rcItem.left,rcItem.top,pt[0].x,rcItem.bottom};
                DrawText(hdc,"负",-1,&rcNeg,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
            }
            if(NULLFLAG) {
                RECT rcNull={rcItem.left,rcItem.top,pt[0].x,rcItem.bottom};
                COLORREF preC=SetTextColor(hdc,RGB(255,255,0));
                DrawText(hdc,"空",-1,&rcNull,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
                SetTextColor(hdc,preC);
            }
             
        }
        if(strlen(item_text)>idx) {
            DrawIconEx(hdc,rcNum.left,rcNum.top+(rcNum.bottom-rcNum.top-cell_cx)/2,(HICON)LoadIcon(NULL,IDI_INFORMATION),cell_cx,cell_cx,0,0,DI_NORMAL);
        }
        SetTextColor(hdc,preColor); 
    }
    else { 
        DrawText(hdc,item_text,-1,&rcItem,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
    }
    return 0;
}

int LVC_DrawItem(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    int row_index;
    int column_count;
    HWND list_header;
    HDC memdc;
    HBITMAP bmp,pre_bmp;
    COLORREF rgb_selected=RGB(18,25,40);
    COLORREF rgb_odd_line=RGB(57,59,61);
    COLORREF rgb_even_line=RGB(50,51,51);
    COLORREF rgb_curr;
    HBRUSH brush,pre_brush;
    HPEN pen,pre_pen;
    HFONT pre_font;
    RECT item_rect;
    char text[1024] = "";
    BOOL check_style=FALSE;
    BOOL check_state=FALSE;
    
    LPDRAWITEMSTRUCT pdraw=(LPDRAWITEMSTRUCT)lParam;
    HDC hdc=pdraw->hDC;
    RECT rect;
    CopyRect(&rect,&pdraw->rcItem);
    
    row_index=pdraw->itemID;//行号
    list_header=ListView_GetHeader(hwnd);
    column_count=Header_GetItemCount(list_header);
    check_style=(((UINT)GetWindowLongPtr(hwnd,GWL_EXSTYLE)&LVS_EX_CHECKBOXES)==LVS_EX_CHECKBOXES);
    if(check_style) {
        check_state=ListView_GetCheckState(hwnd,row_index);
    }
    
    if(pdraw->itemAction&ODA_DRAWENTIRE==ODA_DRAWENTIRE) {
        memdc=CreateCompatibleDC(hdc);
        bmp = CreateCompatibleBitmap(hdc,(rect.right-rect.left),(rect.bottom-rect.top));
        pre_bmp = (HBITMAP)SelectObject(memdc,bmp);
        
        if(pdraw->itemState&ODS_CHECKED==ODS_CHECKED) rgb_curr=rgb_selected;
        else if(row_index%2==0) rgb_curr=rgb_even_line;
        else rgb_curr=rgb_odd_line;
        brush=CreateSolidBrush(rgb_curr);
        pre_brush=(HBRUSH)SelectObject(memdc,brush);
        pen=CreatePen(PS_SOLID,1,RGB(100,100,100));
        pre_pen=(HPEN)SelectObject(memdc,pen);
        Rectangle(memdc,0,-1,rect.right-rect.left,rect.bottom-rect.top);
        pre_font=(HFONT)SelectObject(memdc,(HFONT)SendMessage(hwnd,WM_GETFONT,0,0));//setfont(memdc,"微软雅黑");
        SetBkMode(memdc,TRANSPARENT);
        SetTextColor(memdc,RGB(255,255,255));
        for(int index=0;index<column_count;index++) {
            memset(text,0x00,sizeof(text));
            ListView_GetSubItemRect(hwnd,pdraw->itemID,index, LVIR_LABEL, &item_rect);
            ListView_GetItemText(hwnd,pdraw->itemID,index,text,sizeof(text));
            OffsetRect(&item_rect,-rect.left,-rect.top);
            
            if((index==0)&&check_style) {
                int R=item_rect.bottom-item_rect.top;
                RECT ckicn_rect={item_rect.left-15,item_rect.top,item_rect.left+R-15,item_rect.bottom};

                InflateRect(&ckicn_rect,-5,-5);
                R=ckicn_rect.right-ckicn_rect.left;
               
                HBRUSH icon_brush=CreateSolidBrush(RGB(GetRValue(rgb_curr)-10,GetGValue(rgb_curr)-10,GetBValue(rgb_curr)-10));
                HBRUSH icon_brush2=CreateSolidBrush(RGB(GetRValue(rgb_curr)+10,GetGValue(rgb_curr)+10,GetBValue(rgb_curr)+10));
                HBRUSH icon_brush3=CreateSolidBrush(RGB(100,100,100));
                FrameRect(memdc,&ckicn_rect,icon_brush2);
                InflateRect(&ckicn_rect,-1,-1);
                FrameRect(memdc,&ckicn_rect,icon_brush);
                InflateRect(&ckicn_rect,-1,-1);
                pre_brush=(HBRUSH)SelectObject(memdc,icon_brush3);                    
                Rectangle(memdc,ckicn_rect.left,ckicn_rect.top,ckicn_rect.right,ckicn_rect.bottom);
                SelectObject(memdc,pre_brush);
                
                DrawText(memdc,(check_state?"√":" "),-1,&ckicn_rect,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
                DeleteObject(icon_brush);
                DeleteObject(icon_brush2);
                DeleteObject(icon_brush3);
                         
                item_rect.left=ckicn_rect.right+10;
            }            
            DrawText(memdc,text,-1,&item_rect,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
        }
        
        BitBlt(hdc,rect.left,rect.top,(rect.right-rect.left),(rect.bottom-rect.top),
               memdc,0,0,SRCCOPY);
               
        DeleteObject(SelectObject(memdc,pre_bmp));
        SelectObject(memdc,pre_font);
        DeleteObject(SelectObject(memdc,pre_brush));
        DeleteObject(SelectObject(memdc,pre_pen));
        DeleteDC(memdc);
    }
    
    return 0;
}

int LVC_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL isallign_top_left)
{
    int result=-1;
    RECT rc,rcClient;
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL HasVScroll=((style&WS_VSCROLL)==WS_VSCROLL);
    BOOL HasHScroll=((style&WS_HSCROLL)==WS_HSCROLL);
    BOOL HasBorder=((style&WS_BORDER)==WS_BORDER);
    
    GetWindowRect(hwnd,&rc);
    GetClientRect(hwnd,&rcClient);
    
    switch(zone) {
    case ZVSCROLL: { //垂直滚动
        if(!HasVScroll) break;
            
        prc->left=rc.right-SCROLLBAR_PIXLS-(HasBorder?1:0);
        prc->right=prc->left+SCROLLBAR_PIXLS;
        prc->top=rc.top+(HasBorder?1:0);
        prc->bottom=prc->top+(rcClient.bottom-rcClient.top);
        result=0;
    } break;
    case ZVSTHUMB: {
        RECT rc_vs;
        if(LVC_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,FALSE)!=0) break;
        
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_VERT,&si);
        
        int thumb_pixls=si.nPage*1.0/(si.nMax-si.nMin+1)*(rc_vs.bottom-rc_vs.top);
        if(thumb_pixls<THUMB_MIN_PIXLS) thumb_pixls=THUMB_MIN_PIXLS;
        
        int thumb_pos=si.nPos*1.0/(si.nMax-si.nMin+1-si.nPage)*((rc_vs.bottom-rc_vs.top)-thumb_pixls);
        prc->top=rc_vs.top+thumb_pos;
        prc->bottom=prc->top+thumb_pixls;
        prc->left=rc_vs.left;
        prc->right=rc_vs.right;
        
        result=0;
    } break;
    case ZHSCROLL: { //水平滚动
        if(!HasHScroll) break;
            
        prc->left=rc.left+(HasBorder?1:0);
        prc->right=prc->left+(rcClient.right-rcClient.left);
        prc->top=rc.bottom-SCROLLBAR_PIXLS-(HasBorder?1:0);
        prc->bottom=prc->top+SCROLLBAR_PIXLS;
        result=0;
    } break;
    case ZHSTHUMB: {
        RECT rc_hs;
        if(LVC_GetZoneRect(hwnd,ZHSCROLL,&rc_hs,FALSE)!=0) break;
        
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        
        int thumb_pixls=si.nPage*1.0/(si.nMax-si.nMin+1)*(rc_hs.right-rc_hs.left);
        if(thumb_pixls<THUMB_MIN_PIXLS) thumb_pixls=THUMB_MIN_PIXLS;
        
        int thumb_pos=si.nPos*1.0/(si.nMax-si.nMin+1-si.nPage)*((rc_hs.right-rc_hs.left)-thumb_pixls);
        prc->top=rc_hs.top;
        prc->bottom=rc_hs.bottom;
        prc->left=rc_hs.left+thumb_pos;
        prc->right=prc->left+thumb_pixls;
        
        result=0;
    } break;
    case ZLVTITILE: {
        RECT rcTitle;
        HWND head=ListView_GetHeader(hwnd);
        GetWindowRect(head,&rcTitle);
        OffsetRect(&rcTitle,-rcTitle.left,-rcTitle.top);
        prc->top=rc.top;
        prc->bottom=prc->top+rcTitle.bottom;
        prc->left=rc.left;
        prc->right=prc->left+rcTitle.right;
        
        result=0;
    } break;
    }
    
    if((result==0)&&isallign_top_left) {
        OffsetRect(prc,-rc.left,-rc.top);
    }
    
    return result;
}

int LVC_Scroll(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    HWND head=ListView_GetHeader(hwnd);
    if(!head) return 0;
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return 0;
        
    int hscroll_pixls=(int)wParam;
    int vscroll_pixls=(int)lParam;
    RECT rc,rcScroll,rcHead,rcInvalid;
    GetClientRect(hwnd,&rc);
    CopyRect(&rcScroll,&rc);
    
    GetWindowRect(head,&rcHead);
    rcScroll.top=rcHead.bottom-rcHead.top;
    CopyRect(&rcInvalid,&rcScroll);
    
    SCROLLINFO si={0};
    si.cbSize=sizeof(SCROLLINFO);
    si.fMask=SIF_ALL;
    if(hscroll_pixls!=0) {
        GetScrollInfo(hwnd,SB_HORZ,&si);
        int pre_pos=si.nPos;
        si.fMask=SIF_POS;
        si.nPos=pre_pos+hscroll_pixls;
        if(si.nPos+si.nPage>si.nMax) si.nPos=si.nMax-si.nPage+1;
        else if(si.nPos<=si.nMin) si.nPos=si.nMin;
        SetScrollInfo(hwnd,SB_HORZ,&si,TRUE);
        int chscroll_pixls=si.nPos-pre_pos;
        //向右侧滚动
        rcScroll.top=0;
        if(chscroll_pixls>0) rcInvalid.left=rcInvalid.right-chscroll_pixls;
        else rcInvalid.right=rcInvalid.left-chscroll_pixls;
        ScrollWindow(hwnd,-chscroll_pixls,0,NULL,&rcScroll);
        InvalidateRect(hwnd,&rcInvalid,TRUE);
        LVC_NCDrawScrollBar(hwnd,SB_HORZ);
    }
    if(vscroll_pixls!=0) {
        GetScrollInfo(hwnd,SB_VERT,&si);
        int pre_pos=si.nPos;
        int page=si.nPage;
        si.fMask=SIF_POS;
        int scroll_pos=vscroll_pixls/ls->content_height;
        //修正滚动量
        si.nPos=pre_pos+scroll_pos;
        if(si.nPos+si.nPage>si.nMax) si.nPos=si.nMax-si.nPage+1;
        else if(si.nPos<=si.nMin) si.nPos=si.nMin;
        int cvscroll_pixls=(si.nPos-pre_pos)*ls->content_height;
        SetScrollInfo(hwnd,SB_VERT,&si,TRUE);
        if(cvscroll_pixls>0) //向下方滚动
            rcInvalid.bottom=rcInvalid.top+cvscroll_pixls;
        else
            rcInvalid.top=rcInvalid.top+(page+scroll_pos)*ls->content_height;
        if(rcInvalid.top<rcScroll.top) rcInvalid.top=rcScroll.top;
        if(rcInvalid.bottom>rcScroll.bottom) rcInvalid.bottom=rcScroll.bottom;
        ScrollWindow(hwnd,0,-cvscroll_pixls,NULL,&rcScroll);
        InvalidateRect(hwnd,&rcInvalid,TRUE);
        LVC_NCDrawScrollBar(hwnd,SB_VERT);
    }
    return 0;
}

int LVC_FindItem(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    int start_index=wParam;
    LVFINDINFO* lfi=(LVFINDINFO*)lParam;
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
    
    if((lfi->flags&LVFI_SUBSTRING)==LVFI_SUBSTRING) {
        int count=ListView_GetItemCount(hwnd);
        HWND head=ListView_GetHeader(hwnd);
        int cols=Header_GetItemCount(head);
        if(start_index==-1) start_index=0;
        char itemText[256]="";
        for(int row=start_index;row<count;row++) {
            for(int col=0;col<cols;col++) {
                ListView_GetItemText(hwnd,row,col,itemText,sizeof(itemText));
                if(strstr(itemText,lfi->psz)) {
                    LVC_EnsureVisible(hwnd,(WPARAM)row,(LPARAM)TRUE);
                    return row;
                }
            }
        }
        return -1;
    }
    else {
        return CallWindowProc(ls->pre_proc,hwnd,LVM_FINDITEM,wParam,lParam);
    }
    
}

BOOL LVC_EnsureVisible(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    HWND head=ListView_GetHeader(hwnd);
    if(!head) return 0;
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return 0;
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    
    int itemindex=(int)wParam;
    if(itemindex<=-1) return FALSE;
    
    int offsetx=0,offsety=0;
    BOOL HasVScroll=((style&WS_VSCROLL)==WS_VSCROLL);//垂直滚动
    BOOL HasHScroll=((style&WS_HSCROLL)==WS_HSCROLL);//水平滚动
    if(HasVScroll) {
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_VERT,&si);
        
        if(itemindex>=si.nPos&&itemindex<si.nPos+si.nPage) offsety=0;
        else {
            int scrolly=ls->content_height*(itemindex-si.nPos);
            SendMessage(hwnd,LVM_SCROLL,0,scrolly);
        }
    }
    return TRUE;    
}

BOOL LVC_GetItemRect(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    HWND head=ListView_GetHeader(hwnd);
    if(!head) return 0;
    pLVHeadStyle hs=(pLVHeadStyle)LVHead_GetSettings(head);    
        
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return 0;
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    
    LPRECT prc=(LPRECT)HIWORD(lParam);
    UINT mask=LOWORD(lParam);
    int itemindex=(int)wParam;
    if(itemindex<=-1) return FALSE;
    
    if(mask!=LVIR_BOUNDS) return CallWindowProc(ls->pre_proc,hwnd,LVM_GETITEMRECT,wParam,lParam);
    
    BOOL HasVScroll=((style&WS_VSCROLL)==WS_VSCROLL);
    BOOL HasHScroll=((style&WS_HSCROLL)==WS_HSCROLL);
    int offsetx=0,offsety=0;
    if(HasVScroll) {
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_VERT,&si);
        
        if(itemindex>=si.nPos&&itemindex<si.nPos+si.nPage) {
            offsety=itemindex-si.nPos;      
        }
        else return FALSE;
    }
    if(HasHScroll) {
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        
        offsetx=si.nPos;
    }
    
    prc->top=ls->content_height*offsety+hs->title_height;
    prc->bottom=prc->top+ls->content_height;
    
    int cols=Header_GetItemCount(head);
    HDITEM hdi={0};
    int col_width=0;
    for(int idx=0;idx<cols;idx++) {
        hdi.mask=HDI_WIDTH;
        Header_GetItem(head,idx,&hdi);
        col_width+=hdi.cxy;
    }
    prc->left=-offsetx;
    prc->right=prc->left+col_width;
    
    return TRUE;
}

int LVC_NCDrawScrollBar(HWND hwnd,UINT scrolltype)
{
    RECT rc;
    RECT rcThumb;
    HDC hdc=GetWindowDC(hwnd);
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
            
    if(scrolltype==SB_VERT) {
        if(LVC_GetZoneRect(hwnd,ZVSCROLL,&rc,TRUE)==0&&
           LVC_GetZoneRect(hwnd,ZVSTHUMB,&rcThumb,TRUE)==0) {
            int cx=rc.right-rc.left,cy=rc.bottom-rc.top;
           
            if((style&WS_HSCROLL)) cy+=SCROLLBAR_PIXLS;
           
            HDC memdc=CreateCompatibleDC(hdc);
            HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy); 
            HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
           
            RECT rcMem;
            CopyRect(&rcMem,&rc);
            OffsetRect(&rcMem,-rcMem.left,-rcMem.top);
            
            //GradientRect(memdc,rcMem,RGB(15,15,15),RGB(20,20,20),0);
            HBRUSH brush=CreateSolidBrush(RGB(15,15,15));
            FillRect(memdc,&rcMem,brush);
            DeleteObject(brush);
           
            OffsetRect(&rcThumb,-rc.left-1,-rc.top);
            InflateRect(&rcThumb,-1,0);
            Graphics graphic(memdc);
            GraphicsPath path;
            LinearGradientBrush pbrush(Rect(rcThumb.left,rcThumb.top,rcThumb.right-rcThumb.left,rcThumb.bottom-rcThumb.top),
                                       Color(255,50,50,50),
                                       Color(255,20,20,20),
                                       LinearGradientModeHorizontal);
            
            graphic.SetSmoothingMode(SmoothingModeHighQuality);
            path.AddArc(rcThumb.left,rcThumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,180,180);
            path.AddArc(rcThumb.left,rcThumb.bottom-SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,0,180);
            graphic.FillPath(&pbrush,&path);
            
            BitBlt(hdc,rc.left,rc.top,cx,cy,memdc,0,0,SRCCOPY);
            
            DeleteObject(SelectObject(memdc,pre_bmp));
            DeleteDC(memdc);
        }
    }
    else if(scrolltype==SB_HORZ) { //水平滚动
        if(LVC_GetZoneRect(hwnd,ZHSCROLL,&rc,TRUE)==0&&
           LVC_GetZoneRect(hwnd,ZHSTHUMB,&rcThumb,TRUE)==0) {
            int cx=rc.right-rc.left,cy=rc.bottom-rc.top;
           
            HDC memdc=CreateCompatibleDC(hdc);
            HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy); 
            HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
           
            RECT rcMem;
            CopyRect(&rcMem,&rc);
            OffsetRect(&rcMem,-rcMem.left,-rcMem.top);
            HBRUSH brush=CreateSolidBrush(RGB(15,15,15));
            FillRect(memdc,&rcMem,brush);
            DeleteObject(brush);
           
            OffsetRect(&rcThumb,-rc.left,-rc.top-1);
            InflateRect(&rcThumb,0,-1);
            Graphics graphic(memdc);
            GraphicsPath path;
            LinearGradientBrush pbrush(Rect(rcThumb.left,rcThumb.top,rcThumb.right-rcThumb.left,rcThumb.bottom-rcThumb.top),
                                       Color(255,50,50,50),
                                       Color(255,20,20,20),
                                       LinearGradientModeVertical);
            
            graphic.SetSmoothingMode(SmoothingModeHighQuality);
            path.AddArc(rcThumb.left,rcThumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,90,180);
            path.AddArc(rcThumb.right-SCROLLBAR_PIXLS,rcThumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,-90,180);
            graphic.FillPath(&pbrush,&path);
            
            BitBlt(hdc,rc.left,rc.top,cx,cy,memdc,0,0,SRCCOPY);
            
            DeleteObject(SelectObject(memdc,pre_bmp));
            DeleteDC(memdc);
        }
    }
    
    if((style&WS_BORDER)==WS_BORDER) {
        //画边框
        RECT rcGrid;
        GetWindowRect(hwnd,&rcGrid);
        OffsetRect(&rcGrid,-rcGrid.left,-rcGrid.top);

        POINT pt[5]={{rcGrid.left,rcGrid.top},\
        {rcGrid.right-1,rcGrid.top},\
        {rcGrid.right-1,rcGrid.bottom-1},\
        {rcGrid.top,rcGrid.bottom-1},\
        {0,0}\
        };
        HPEN pen=CreatePen(PS_SOLID,1,RGB(45,45,45));
        SelectObject(hdc,pen);
        for(int index=0;index<4;index++) {
            MoveToEx(hdc,pt[index].x,pt[index].y,&(pt[4]));
            LineTo(hdc,pt[(index+1)%4].x,pt[(index+1)%4].y);
        }
        DeleteObject(pen);
    }
    ReleaseDC(hwnd,hdc);
    return 0;
}

int LVC_HitTest(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
    
    RECT rc_hs,rc_vs;
    if(LVC_GetZoneRect(hwnd,ZHSCROLL,&rc_hs,FALSE)==0) {
        if(PtInRect(&rc_hs,pt)) return HTHSCROLL;
    }
    
    if(LVC_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,FALSE)==0) {
        if(PtInRect(&rc_vs,pt)) return HTVSCROLL;
    }
    
    return CallWindowProc(ls->pre_proc,hwnd,WM_NCHITTEST,wParam,lParam);
}

int LVC_ScrollBefore(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    UINT hit=wParam;
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
    
    if(hit==HTHSCROLL) {
        RECT rc_hs;
        if(LVC_GetZoneRect(hwnd,ZHSTHUMB,&rc_hs,FALSE)!=0) return -1;
        if(!PtInRect(&rc_hs,pt)) return -1;
        
        ScreenToClient(hwnd,&pt);
        ls->drag_pt.x=pt.x;
        ls->drag_pt.y=-1;
        
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        
        ls->drag_pos.x=si.nPos;
        ls->drag_pos.y=-1;    
        
        SetCapture(hwnd);
        return 0;
    }
    else if(hit==HTVSCROLL) {
        RECT rc_vs;
        if(LVC_GetZoneRect(hwnd,ZVSTHUMB,&rc_vs,FALSE)!=0) return -1;
        if(!PtInRect(&rc_vs,pt)) return -1;
        
        ScreenToClient(hwnd,&pt);
        ls->drag_pt.y=pt.y;
        ls->drag_pt.x=-1;
        
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS;
        GetScrollInfo(hwnd,SB_VERT,&si);
        
        ls->drag_pos.y=si.nPos;
        ls->drag_pos.x=-1;    
                    
        SetCapture(hwnd);
        return 0;
    }
    return CallWindowProc(ls->pre_proc,hwnd,WM_NCLBUTTONDOWN,wParam,lParam);
}

int LVC_Scrolling(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
        
    if(GetCapture()!=hwnd) return -1;
    if(ls->drag_pos.x!=-1) { // HORZ 水平滚动
        RECT rc_hs,rc_thumb;
        
        if(LVC_GetZoneRect(hwnd,ZHSCROLL,&rc_hs,TRUE)!=0) return -1;
        if(LVC_GetZoneRect(hwnd,ZHSTHUMB,&rc_thumb,TRUE)!=0) return -1;
        
        SCROLLINFO si;
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        int pre_pos=si.nPos;
        si.nPos=(int)ls->drag_pos.x+(int)(si.nMax-si.nMin+1-si.nPage)*1.0*(int)(pt.x-ls->drag_pt.x)/((rc_hs.right-rc_hs.left)-(rc_thumb.right-rc_thumb.left));   
        if(si.nPos==ls->drag_pos.x) return -1;
        
        if((int)si.nPos>(int)(si.nMax-si.nPage+1)) si.nPos=si.nMax-si.nPage+1;
        else if(si.nPos<si.nMin) si.nPos=si.nMin;
        
        ListView_Scroll(hwnd,si.nPos-pre_pos,0);
        LVC_NCDrawScrollBar(hwnd,SB_HORZ);
    }
    else if(ls->drag_pos.y!=-1) { //垂直滚动
        RECT rc_vs,rc_thumb;
        
        if(LVC_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,TRUE)!=0) return -1;
        if(LVC_GetZoneRect(hwnd,ZVSTHUMB,&rc_thumb,TRUE)!=0) return -1;
        
        SCROLLINFO si;
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_VERT,&si);
        int pre_pos=si.nPos;
        
        si.nPos=(int)ls->drag_pos.y+((int)(si.nMax-si.nMin+1-si.nPage))*1.0*((int)(pt.y-ls->drag_pt.y))/((rc_vs.bottom-rc_vs.top)-(rc_thumb.bottom-rc_thumb.top));   
        if(si.nPos==ls->drag_pos.y) return -1;
        
        if((int)si.nPos>(int)(si.nMax-si.nPage+1)) si.nPos=si.nMax-si.nPage+1;
        else if(si.nPos<si.nMin) si.nPos=si.nMin;
        
        ListView_Scroll(hwnd,0,(si.nPos-pre_pos)*ls->content_height);
        LVC_NCDrawScrollBar(hwnd,SB_VERT);
    }
    return 0;
}

int LVC_ScrollDone(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
    
    if(GetCapture()==hwnd) {
        ReleaseCapture();
        if(ls->drag_pos.y!=-1) LVC_NCDrawScrollBar(hwnd,SB_VERT);
        if(ls->drag_pos.x!=-1) LVC_NCDrawScrollBar(hwnd,SB_HORZ);
        
        ls->drag_pt.x=ls->drag_pt.y=ls->drag_pos.x=ls->drag_pos.y=-1;            
    }
    
    return 0;
}

int LVC_SetState(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    POINT offset={0,0};
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL HasVScroll=(style&WS_VSCROLL==WS_VSCROLL);
    BOOL HasHScroll=(style&WS_HSCROLL==WS_HSCROLL);
    SCROLLINFO si={0};
    HWND head=ListView_GetHeader(hwnd);
    RECT rcHead,rcHeadItem,rcItem;
    int cols=Header_GetItemCount(head);
    
    pListViewStyle ls=ListView_GetSettings(hwnd);
    if(!ls) return -1;
        
    GetWindowRect(head,&rcHead);
    if(pt.y<(rcHead.bottom-rcHead.top)) return 0;
        
    //水平位移
    if(HasHScroll) {
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        offset.x=si.nPos;
    }
    //垂直滚动量
    if(HasVScroll) {
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_VERT,&si);
        offset.y=si.nPos;
    }
    
    UINT itemID=(pt.y-(rcHead.bottom-rcHead.top))/ls->content_height+offset.y;
    int subIndex=0;
    pt.y=(rcHead.bottom-rcHead.top)/2;

    for(int index=0;index<cols;index++) {
        Header_GetItemRect(head,index,&rcHeadItem);
        OffsetRect(&rcHeadItem,-offset.x,0);
        
        if(PtInRect(&rcHeadItem,pt)) {
            subIndex=index;
            ListView_SetItemState(hwnd,itemID,LVIS_FOCUSED,LVIS_FOCUSED);
            //需要刷新选区
            InvalidateRect(hwnd,NULL,TRUE);
            SetFocus(hwnd);
            break;
        }
    }
    return 0;
}


int LVHead_InitialSettings(HWND hwnd)
{
    pLVHeadStyle lh=(pLVHeadStyle)calloc(sizeof(RLVHeadStyle),1);
    if(!lh) return -1;
    
    lh->proc=LVHProc;
    lh->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)LVHProc);
    lh->color_text=RGB(250,250,250);
    HFONT font=CreateFont(18,0,0,0,
                    FW_SEMIBOLD,//FW_MEDIUM,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "微软雅黑");
    lh->font=font;
    lh->title_height=50;
    SendMessage(hwnd,WM_SETFONT,(WPARAM)lh->font,0);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)lh);
    
    return 0;
}

pLVHeadStyle LVHead_GetSettings(HWND hwnd)
{
    return (pLVHeadStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int LVHead_ClearSettings(HWND hwnd)
{
    pLVHeadStyle lh=LVHead_GetSettings(hwnd);
    if(!lh) return -1;
    
    if(lh->font) DeleteObject(lh->font);
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)lh->pre_proc);
    free(lh);
    
    return 0;
}

//描述：listview的子控件 header 的窗口过程
LRESULT CALLBACK LVHProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    pLVHeadStyle head=LVHead_GetSettings(hwnd);
    if(!head) return 0;
        
    switch(message) {
   	case HDM_LAYOUT: {
        LRESULT result = CallWindowProc(head->pre_proc,hwnd, message, wParam, lParam);
        
        LPHDLAYOUT lphdLayout=(LPHDLAYOUT)lParam;
        lphdLayout->pwpos->cy=head->title_height;
        lphdLayout->prc->top=lphdLayout->pwpos->cy;
        
        return result;
    } break;
    case WM_MOUSEMOVE: return 1;
    case WM_PAINT: {
        LVHPaint(hwnd,wParam,lParam);
        return 0;
    } break;
    case WM_ERASEBKGND: {
        return 1;
    } break;
    case WM_NCDESTROY: {
        WNDPROC pre_proc=head->pre_proc;
        LVHead_ClearSettings(hwnd);
        if(pre_proc) return CallWindowProc(pre_proc,hwnd,message,wParam,lParam);
    } break; 
    }
    
    return CallWindowProc(head->pre_proc,hwnd,message,wParam,lParam);    
}

//Fuck the head...
//ps.rcPaint 更新区域在滚动情形下很奇怪
//...
int LVHPaint(HWND hwnd,WPARAM wParam,LPARAM lParem) 
{
    pLVHeadStyle hs=LVHead_GetSettings(hwnd);
    if(!hs) return -1;
    
    HWND listview=GetParent(hwnd);
    if(!listview) return -1;
    int x_offset=0;
    UINT style=(UINT)GetWindowLongPtr(listview,GWL_STYLE);
    BOOL HasHScrollBar=((WS_HSCROLL&style)==WS_HSCROLL);
    if(HasHScrollBar) {
    SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(listview,SB_HORZ,&si);
        x_offset=si.nPos;
    }
    
    PAINTSTRUCT ps;
    BeginPaint(hwnd,&ps);
    HDC hdc=ps.hdc;
    int cx=ps.rcPaint.right-ps.rcPaint.left;
    int cy=ps.rcPaint.bottom-ps.rcPaint.top;
    /*
    HDC hdc=GetDC(hwnd);
    RECT rc;
    GetClientRect(hwnd,&rc);
    int cx=rc.right-rc.left;
    int cy=rc.bottom-rc.top;
    */
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    HFONT pre_font=(HFONT)SelectObject(memdc,(HFONT)SendMessage(hwnd,WM_GETFONT,0,0));
    
    RECT rc_bmp;
    SetRect(&rc_bmp,0,0,cx,cy);
    COLORREF rgb_1=RGB(15,15,15),rgb_2=RGB(30,30,30);
    GradientRect2(memdc,rc_bmp,rgb_1,rgb_2,1);
    
    SetTextColor(memdc,hs->color_text);
    SetBkMode(memdc,TRANSPARENT);
    
    //POINT pt_org;
    //OffsetViewportOrgEx(memdc,-(ps.rcPaint.left)-x_offset,-(ps.rcPaint.top),&pt_org);
    
    int total_offset=0;
    int col_count=Header_GetItemCount(hwnd);
    RECT rcItem;
    HDITEM item={0};
    char col_title[256]="";
    
    for(int index=0;index<col_count;index++) {
        Header_GetItemRect(hwnd,index,&rcItem);
        OffsetRect(&rcItem,-x_offset,0);
        
        if(rcItem.right<=ps.rcPaint.left) continue;        
        else if(rcItem.left<=ps.rcPaint.left&&rcItem.right>ps.rcPaint.left) {
            //需要部分绘制
            total_offset=rcItem.right-ps.rcPaint.left;
        }
        else if(rcItem.left>=ps.rcPaint.left&&total_offset<cx) {
            //需要整段绘制
            total_offset+=rcItem.right-rcItem.left;
        }
        else if(total_offset>=cx) break; 
        //还原
        //OffsetRect(&rcItem,x_offset,0);
        item.mask=HDI_TEXT|HDI_STATE|HDI_WIDTH;
        item.pszText=col_title;
        item.cchTextMax=sizeof(col_title);
        Header_GetItem(hwnd,index,&item);
        
        HBRUSH brush_border=CreateSolidBrush(RGB(0,128,255));
        FrameRect(memdc,&rcItem,brush_border);
        DeleteObject(brush_border);
        
        DrawText(memdc,col_title,-1,&rcItem,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
          
        //绘制立体分割线
        POINT pt[7]={{rcItem.right-2,rcItem.top},
                     {rcItem.right-2,rcItem.bottom},
                     {rcItem.right-1,rcItem.top},
                     {rcItem.right-1,rcItem.bottom},
                     //{hs->dragnow-hs->offset,rcItem.top},
                     //{hs->dragnow-hs->offset,rcItem.bottom}
                     };
        
        COLORREF color_1=RGB(0,0,0),color_2=RGB(30,30,30);
        HPEN sp_line1=CreatePen(PS_SOLID,1,color_1);
        HPEN sp_line2=CreatePen(PS_SOLID,1,color_2);
        HPEN pre_sp;
        
        pre_sp=(HPEN)SelectObject(memdc,sp_line1);
        MoveToEx(memdc,pt[0].x,pt[0].y,&(pt[6]));
        LineTo(memdc,pt[1].x,pt[1].y);
        
        SelectObject(memdc,sp_line2);
        MoveToEx(memdc,pt[2].x,pt[2].y,&(pt[6]));
        LineTo(memdc,pt[3].x,pt[3].y);
        
        /*
        if(GetCapture()==hwnd&&hs->dragging) {
            COLORREF color_drag=RGB(0,100,200);
            HPEN pen_drag=CreatePen(PS_SOLID,2,color_drag);
            SelectObject(memdc,pen_drag);
            MoveToEx(memdc,pt[4].x,pt[4].y,&(pt[6]));
            LineTo(memdc,pt[5].x,pt[5].y);
            DeleteObject(pen_drag);
        }  
        */
        DeleteObject(sp_line1);
        DeleteObject(sp_line2);
    }
    
    //SetViewportOrgEx(memdc,pt_org.x,pt_org.y,NULL);
    //BitBlt(hdc,0,0,cx,cy,memdc,0,0,SRCCOPY);
    BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,cx,cy,memdc,0,0,SRCCOPY);
       
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    //ReleaseDC(hwnd,hdc);
    EndPaint(hwnd,&ps);
    return 0;
}

void GradientRect2(HDC hdc,RECT rc_tmp,COLORREF c1,COLORREF c2,int verical_or_horizen)
{
    RECT rc;
    CopyRect(&rc,&rc_tmp);
    
    Point pt[4]={Point(rc.left,rc.top),Point(rc.right,rc.top),Point(rc.right,rc.bottom),Point(rc.left,rc.bottom)};
    Rect rect(rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
    Graphics graphic(hdc);
    GraphicsPath path;
    LinearGradientBrush brush(rect,Color(255,GetRValue(c1),GetGValue(c1),GetBValue(c1)),
                                   Color(255,GetRValue(c2),GetGValue(c2),GetBValue(c2)),
                              verical_or_horizen?LinearGradientModeVertical:LinearGradientModeHorizontal);
    
    graphic.SetSmoothingMode(SmoothingModeHighQuality);
    path.AddLines(pt,4);
    graphic.FillPath(&brush,&path);
}