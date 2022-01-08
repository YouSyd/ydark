#include "../include/tabpage.h"

BOOL CALLBACK UpDownCheckProc(HWND hwnd,LPARAM lParam)
{
    pSearchParam sp=(pSearchParam)lParam;
    char class_name[256]="";
    GetClassName(hwnd,class_name,sizeof(class_name));
    HWND parent=GetParent(hwnd);
    
    if(strstr(class_name,sp->class_name)&&(parent==sp->parent)) {
        sp->hwnd=hwnd;
        return FALSE;
    }
    return TRUE;
}

LRESULT CALLBACK TabCtrlOwnerProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    pTabStyle ts=Tab_GetSettings(hwnd);
    if(!ts) return 0;
    
    switch(message){
    case WM_NCCALCSIZE: {
        RECT rect_new;
        RECT rect_old;
        RECT client_rect_new;
        RECT client_rect_old;
        //RECT rc_hs;
        UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
        BOOL HasBorder=((style&WS_BORDER)==WS_BORDER);
        
        if(wParam == TRUE) {
            LPNCCALCSIZE_PARAMS calc_param = (LPNCCALCSIZE_PARAMS)lParam;
            
            CopyRect(&rect_new,&(calc_param->rgrc[0]));
            CopyRect(&rect_old,&(calc_param->rgrc[1]));
            CopyRect(&client_rect_old,&(calc_param->rgrc[2]));
            
            client_rect_new = {rect_new.left+(HasBorder?ts->margin_border:0),
                               rect_new.top+(HasBorder?ts->margin_border:0),
                               rect_new.right-((HasBorder?ts->margin_border:0)),
                               rect_new.bottom-((HasBorder?ts->margin_border:0))};
            CopyRect(&(calc_param->rgrc[0]),&client_rect_new);
            CopyRect(&(calc_param->rgrc[1]),&rect_new);
            CopyRect(&(calc_param->rgrc[2]),&rect_old);
            
            return WVR_VALIDRECTS;
        }
        else {
            RECT* prect = (RECT*)lParam;
            prect->right-=(HasBorder?ts->margin_border:0); 
            prect->bottom-=(HasBorder?ts->margin_border:0);
            prect->left+=(HasBorder?ts->margin_border:0);
            prect->top+=(HasBorder?ts->margin_border:0);
            return 0;
        }
    } break;
    case WM_NCACTIVATE:
    case WM_NCPAINT: {
        UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
        BOOL IsBorder=((style&WS_BORDER)==WS_BORDER);
        if(!IsBorder) break;
            
        HDC hdc=GetWindowDC(hwnd);
        RECT rc;
        GetWindowRect(hwnd,&rc);
        OffsetRect(&rc,-rc.left,-rc.top);
        
        HPEN pen=CreatePen(PS_SOLID,1,ts->color_border);
        SelectObject(hdc,pen);
        
        for(int i=0;i<ts->margin_border;i++) {
            POINT pt[5]={{rc.left,rc.top},\
            {rc.right,rc.top},\
            {rc.right,rc.bottom},\
            {rc.top,rc.bottom},\
            {0,0}\
            };
            for(int index=0;index<4;index++) {
                MoveToEx(hdc,pt[index].x,pt[index].y,&(pt[4]));
                LineTo(hdc,pt[(index+1)%4].x,pt[(index+1)%4].y);
            }
            InflateRect(&rc,-1,-1);
        }
        DeleteObject(pen);
        ReleaseDC(hwnd,hdc);
        
        return 0;
    } break;
    case TCM_DELETEITEM: {
        int index=(int)wParam;
        TCITEM ti={0};
        ti.mask=TCIF_PARAM;
        TabCtrl_GetItem(hwnd,index,&ti);
        pTabItem pti=(pTabItem)ti.lParam;
        if(pti) {
            if(0!=SendMessage(pti->hwnd,WM_CLOSE,0,0)) {
                MessageBox(hwnd,"关闭动作被终止","提示",MB_OK|MB_ICONINFORMATION);
                return 0;
            }
            free(pti);
        }
        
        TabCtrl_RemoveImage(hwnd,index);
        
        LRESULT ret=CallWindowProc(ts->pre_proc,hwnd,message,wParam,lParam);
        int tab_count=TabCtrl_GetItemCount(hwnd);
        if(ts->index_Sel>=tab_count) {
            if(tab_count>0) ts->index_Sel=tab_count-1;
            else ts->index_Sel=-1;
            TabCtrl_SetCurSel(hwnd,ts->index_Sel);
        }
        else {
            TabCtrl_SetCurSel(hwnd,index);
        }
        return ret;
    } break;
    case WM_NCHITTEST: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        RECT rc;
        GetWindowRect(hwnd,&rc);
        //GetClientRect(hwnd,&rc);
        RECT rcItem;
        CopyRect(&rcItem,&rc);
        switch(ts->tab_pos) {
        case TAB_TOP:
            rcItem.bottom=rcItem.top+ts->item_cy;
        break;
        case TAB_LEFT:
            rcItem.right=rcItem.left+ts->item_cx;
        break;
        }
        if(PtInRect(&rcItem,pt)) return HTCLIENT;
        else return HTNOWHERE;
    } break;
    case TCM_HITTEST: {
        LPTCHITTESTINFO hit=(LPTCHITTESTINFO)lParam;
        
        RECT rcItem;
        int tab_count=TabCtrl_GetItemCount(hwnd);
        for(int index=0;index<tab_count;index++) {
            TabCtrl_GetItemRect(hwnd,index,&rcItem);
            RECT rcIcon,rcLabel,rcClose;
            CopyRect(&rcIcon,&rcItem);
            CopyRect(&rcClose,&rcItem);
            CopyRect(&rcLabel,&rcItem);
            
            rcIcon.right=rcIcon.left+(rcClose.bottom-rcClose.top);
            if(ts->tab_pos==TAB_TOP) rcClose.left=rcClose.right-(rcClose.bottom-rcClose.top);
            else rcClose.left=rcClose.right;
            rcLabel.left=rcIcon.right;
            rcLabel.right=rcClose.left;
            if(PtInRect(&rcItem,hit->pt)) {
                if(PtInRect(&rcIcon,hit->pt)) hit->flags=TCHT_ONITEMICON;
                else if(PtInRect(&rcLabel,hit->pt)) hit->flags=TCHT_ONITEMLABEL;
#define TCHT_ONITEMCLOSE (TCHT_ONITEMLABEL+1)
                else if(PtInRect(&rcClose,hit->pt)) hit->flags=TCHT_ONITEMCLOSE;
                
                return index;
            }
        }
        
        hit->flags=TCHT_NOWHERE;
        return -1;
    } break;
    case TCM_SETCURSEL: {
        int index_Sel=(int)wParam;
        
        if(index_Sel<0) {
            ts->index_Sel=-1;
            break;
        }
        
        RECT rcItem;
        TCITEM ti={0};
        pTabItem pti=NULL;
        UINT style;
        int cur_Sel=ts->index_Sel;
       	if(cur_Sel!=-1) {
            ti.mask=TCIF_PARAM;
            TabCtrl_GetItem(hwnd,ts->index_Sel,&ti);
    	    pti=(pTabItem)ti.lParam;
    	    style=(UINT)GetWindowLongPtr(pti->hwnd,GWL_STYLE);
    	    if((style&WS_VISIBLE)==WS_VISIBLE) {
    	        SetWindowLongPtr(pti->hwnd,GWL_STYLE,(~WS_VISIBLE)&style);
    	    }
    	    SetWindowPos(pti->hwnd,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW);
        }
        
        //维护新的选中项
       	ts->index_Sel=index_Sel;
       	
       	ti.mask=TCIF_PARAM;
        TabCtrl_GetItem(hwnd,ts->index_Sel,&ti);
	    pti=(pTabItem)ti.lParam;
	    style=(UINT)GetWindowLongPtr(pti->hwnd,GWL_STYLE);
	    if((style&WS_VISIBLE)!=WS_VISIBLE) SetWindowLongPtr(pti->hwnd,GWL_STYLE,WS_VISIBLE|style);
	        
	    RECT rc;
	    //GetWindowRect(hwnd,&rc);
	    GetClientRect(hwnd,&rc);
	    OffsetRect(&rc,-rc.left,-rc.top);
	    switch(ts->tab_pos) {
	    case TAB_LEFT: {
	        rc.left+=ts->item_cx+1;
	    } break;
	    case TAB_TOP: {
	        rc.top+=ts->item_cy;
	    } break;
	    }
	    InvalidateRect(hwnd,&rc,TRUE);
       	TabCtrl_AdjustRect(pti->hwnd,TRUE,&rc);
       	SetWindowPos(pti->hwnd,NULL,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,SWP_SHOWWINDOW);
       	//MoveWindow(pti->hwnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,TRUE);
       	//UpdateWindow(pti->hwnd);
       	
       	if(cur_Sel!=-1) {
       	    if(/*index_Sel!=cur_Sel&&*/index_Sel!=-1) {
       	        TabCtrl_GetItemRect(hwnd,index_Sel,&rcItem);
       	        InvalidateRect(hwnd,&rcItem,TRUE);
       	    }
       	    TabCtrl_GetItemRect(hwnd,cur_Sel,&rcItem);
       		InvalidateRect(hwnd,&rcItem,TRUE);
       	}
       	return 0;
    } break;
    case TCM_GETCURSEL: {
        return ts->index_Sel;
    } break;
    case TCM_GETITEMRECT: {
        int index=(int)wParam;
        LPRECT prc=(LPRECT)lParam;
        
        RECT rc;
        GetClientRect(hwnd,&rc);
        switch(ts->tab_pos) {
        case TAB_LEFT: {
            int item_height=ts->item_cy;
            int item_width=ts->item_cx;
            
            //未考虑滚动
            prc->left=rc.left;
            prc->right=prc->left+item_width;
            prc->top=index*item_height;
            prc->bottom=prc->top+item_height;
        } break;
        case TAB_TOP: {
            //保持原状
            LRESULT ret=CallWindowProc(ts->pre_proc,hwnd,message,wParam,lParam);
            prc->bottom=prc->top+ts->item_cy;
            return ret;
        } break;
        }
        
        return TRUE;
    } break;
    case WM_PAINT: {
        Tab_Paint(hwnd,wParam,lParam);
        return 0;
    } break;
    case WM_ERASEBKGND: {
        return 1;
    } break;
    case WM_LBUTTONDOWN: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        TCHITTESTINFO hit={0};
        memcpy(&hit.pt,&pt,sizeof(POINT));
        int index_Sel=TabCtrl_GetCurSel(hwnd);
        int index_Hit=TabCtrl_HitTest(hwnd,&hit);
        if(index_Hit==-1) break;
    
        RECT rcPre,rcItem;
        if(index_Sel!=index_Hit&&index_Sel!=-1) {
            TabCtrl_GetItemRect(hwnd,index_Sel,&rcPre);
            InvalidateRect(hwnd,&rcPre,FALSE);
        }
        TabCtrl_GetItemRect(hwnd,index_Hit,&rcItem);
        TabCtrl_SetCurSel(hwnd,index_Hit);
        InvalidateRect(hwnd,&rcItem,TRUE);
        
        if(hit.flags==TCHT_ONITEMCLOSE) {
            //关闭
            SendMessage(hwnd,TCM_DELETEITEM,(WPARAM)index_Hit,0);
        }
        
        return 0;
    } break;
    case WM_MOUSEMOVE: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        
        TCHITTESTINFO hit={0};
        memcpy(&hit.pt,&pt,sizeof(POINT));
        int index_hover=TabCtrl_HitTest(hwnd,&hit);
        
        RECT rcItem;
        int pre_hover=ts->index_hover;
        ts->index_hover=index_hover;
        if(pre_hover!=-1&&index_hover!=pre_hover) {
            TabCtrl_GetItemRect(hwnd,pre_hover,&rcItem);
            InvalidateRect(hwnd,&rcItem,TRUE);
        }
        if(index_hover!=-1&&index_hover!=pre_hover) {
            TabCtrl_GetItemRect(hwnd,index_hover,&rcItem);
            InvalidateRect(hwnd,&rcItem,TRUE);
        }
        
        if(index_hover>=0) {
            RECT rcItem;
            TabCtrl_GetItemRect(hwnd,index_hover,&rcItem);
            InvalidateRect(hwnd,&rcItem,TRUE);
        }
        
        return 0;
    } break;
    case WM_SIZE: {
        HWND hwnd_msg=GetDlgItem(GetParent(hwnd),0x0002);
		int height=HIWORD(lParam);
		int width=LOWORD(lParam);
		
        HWND updown=Tab_GetUpDown(hwnd);
        if(updown!=NULL) {
            UINT Ret=(UINT)SendMessage(updown,UDM_GETPOS,0,0);
            int result=HIWORD(Ret);
            int updown_pos=LOWORD(Ret);
            
            if(WS_VISIBLE==(WS_VISIBLE&(UINT)GetWindowLongPtr(updown,GWL_STYLE))) {
                //MoveWindow(updown,width-40,0,40,ts->item_cy,TRUE);
            }
        }
        
        int index=TabCtrl_GetCurSel(hwnd);
        if(index>=0) {
            TCITEM ti={0};
            ti.mask=TCIF_PARAM;
            TabCtrl_GetItem(hwnd,index,&ti);
            pTabItem pti=(pTabItem)ti.lParam;
            
            RECT rc;
    	    //GetWindowRect(hwnd,&rc);
    	    //OffsetRect(&rc,-rc.left,-rc.top);
    	    GetClientRect(hwnd,&rc);
    	    switch(ts->tab_pos) {
    	    case TAB_LEFT: {
    	        rc.left+=ts->item_cx+1;
    	    } break;
    	    case TAB_TOP: {
    	        rc.top+=ts->item_cy;
    	    } break;
    	    }
    	    TabCtrl_AdjustRect(pti->hwnd,TRUE,&rc);
            SetWindowPos(pti->hwnd,NULL,rc.left,rc.top,(rc.right-rc.left),(rc.bottom-rc.top),SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
        }
    } break;
    case WM_NCDESTROY: {
        if(ts) {
            int count=TabCtrl_GetItemCount(hwnd);
            for(int index=0;index<count;index++) {
                TabCtrl_DeleteItem(hwnd,index);
            }
            
            DeleteObject(ts->font);
            DeleteObject(ts->brush_border);
            DeleteObject(ts->brush);
            
            WNDPROC tmp_proc=NULL;
            if(ts->pre_proc) {
                tmp_proc=ts->pre_proc;
                SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ts->pre_proc);
            }
            free(ts);
            SetWindowLongPtr(hwnd,GWLP_USERDATA,NULL);
            return CallWindowProc(tmp_proc,hwnd,message,wParam,lParam); 
        }
    } break;
    }
    return CallWindowProc(ts->pre_proc,hwnd,message,wParam,lParam);   
}

int Tab_InitialSettings(HWND hwnd)
{
    pTabStyle ts=(pTabStyle)calloc(sizeof(RTabStyle),1);
    if(!ts) return -1;
    
    ts->font_cy=15;
    ts->font=CreateFont(ts->font_cy,0,0,0,
                        FW_MEDIUM,//FW_SEMIBOLD,
                        FALSE,FALSE,FALSE,
                        DEFAULT_CHARSET,
                        OUT_OUTLINE_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, 
                        VARIABLE_PITCH,
                        "Courier New");
    SendMessage(hwnd,WM_SETFONT,(WPARAM)ts->font,NULL);
    
    ts->item_cx=240;
    ts->item_cy=25;
    
    ts->color_bk=RGB(15,15,15);
    ts->color_text=RGB(250,250,250);
    ts->brush=CreateSolidBrush(ts->color_bk);
    
    ts->margin_border=1;
    
    ts->color_border=RGB(53,53,53);
    ts->brush_border=CreateSolidBrush(ts->color_border);
    
    ts->index_Sel=-1;
    
    ts->proc=TabCtrlOwnerProc;
    ts->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ts->proc);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)ts);
    return 0;
}

pTabStyle Tab_GetSettings(HWND hwnd)
{
    return (pTabStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int Tab_ClearSettings(HWND hwnd)
{
    pTabStyle ts=Tab_GetSettings(hwnd);
    if(!ts) return -1;
    
    DeleteObject(ts->font);
    DeleteObject(ts->brush);
    DeleteObject(ts->brush_border);
    
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ts->pre_proc);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,0);
    
    free(ts);
    return 0;
}

int Tab_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam)
{    
    pTabStyle ts=Tab_GetSettings(hwnd);
    if(!ts) return -1;
    
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
    
    int itemStart=0;
    HWND updown=Tab_GetUpDown(hwnd);
    if(updown!=NULL) {
        UINT style=(UINT)GetWindowLongPtr(updown,GWL_STYLE);
        if(ts->tab_pos==TAB_LEFT) {
            if(WS_VISIBLE==(WS_VISIBLE&style)) SetWindowLongPtr(updown,GWL_STYLE,(style&(~WS_VISIBLE)));
        }
        else if(ts->tab_pos==TAB_TOP) {
            //UINT Ret=(UINT)SendMessage(updown,UDM_GETPOS,0,0);
            //int result=HIWORD(Ret);
            //itemStart=LOWORD(Ret);
        }
    }
    
    POINT pt_org;
    OffsetViewportOrgEx(memdc,-ps.rcPaint.left,-ps.rcPaint.top,&pt_org);
     
    SelectObject(memdc,(HFONT)SendMessage(hwnd,WM_GETFONT,0,0));
    SetBkMode(memdc,TRANSPARENT);
    SetTextColor(memdc,ts->color_text);
    HBRUSH brush=CreateSolidBrush(RGB(30,30,30));
    HBRUSH brush_Sel=CreateSolidBrush(RGB(10,10,10));
    HPEN pen=CreatePen(PS_SOLID,1,RGB(53,53,53));
    SelectObject(memdc,brush);
    SelectObject(memdc,pen);
    TCITEM ti={0};
    char title[256]="";
    int tab_count=TabCtrl_GetItemCount(hwnd);
    int index_Sel=TabCtrl_GetCurSel(hwnd);
    RECT rcTab={0};
    for(int index=itemStart;index<=tab_count;index++) {
        if(index==tab_count) {
            RECT rc;
            GetClientRect(hwnd,&rc);
            switch(ts->tab_pos) {
            case TAB_LEFT:
                rcTab.top=rcTab.bottom;
                rcTab.bottom=rc.bottom;
            break;
            case TAB_TOP:
                rcTab.top=rc.top;
                rcTab.bottom=rc.bottom;
                rcTab.left=rcTab.right;
                rcTab.right=rc.right;
            break;
            }
            Rectangle(memdc,rcTab.left,rcTab.top,rcTab.right,rcTab.bottom);
        }
        else {
            TabCtrl_GetItemRect(hwnd,index,&rcTab);
            
            ti.mask=TCIF_TEXT|TCIF_IMAGE;
            ti.pszText=title;
            ti.cchTextMax=sizeof(title);
            ti.iImage=index;
            TabCtrl_GetItem(hwnd,index,&ti);
            
            RECT rcIcon,rcLabel,rcClose,rcTmp;
            CopyRect(&rcIcon,&rcTab);
            CopyRect(&rcLabel,&rcTab);
            CopyRect(&rcTmp,&rcTab);
            CopyRect(&rcClose,&rcTab);
            rcIcon.left+=3;
            rcIcon.right=rcIcon.left+(rcIcon.bottom-rcIcon.top);
            rcLabel.left=rcIcon.right+1;
            
            Rectangle(memdc,rcTmp.left,rcTmp.top,rcTmp.right,rcTmp.bottom);
            if(index_Sel==index) {
                FillRect(memdc,&rcTmp,brush_Sel);
                SetTextColor(memdc,RGB(0,100,200));
                
                RECT rcActive;
                switch(ts->tab_pos) {
                case TAB_LEFT: 
                    rcActive={rcTmp.left,rcTmp.top+1,rcTmp.left+7,rcTmp.bottom-1};
                
                    rcIcon.left=3+rcActive.right;
                    rcIcon.right=rcIcon.left+(rcTmp.bottom-rcTmp.top);
                    
                    rcLabel.left=rcIcon.right+1;
                break;
                case TAB_TOP: {
                    rcActive={rcTmp.left,rcTmp.top,rcTmp.right,rcTmp.top+3};
                   
                    rcIcon.top=rcTmp.top+5;
                    rcIcon.right=rcIcon.left+(rcTmp.bottom-rcTmp.top);
                    
                    rcLabel.top=rcIcon.top;
                    rcLabel.left=rcIcon.right+1;
                } break;
                }
                HBRUSH brush_Active=CreateSolidBrush(RGB(150,0,0));
                FillRect(memdc,&rcActive,brush_Active);
                DeleteObject(brush_Active);
            }
            else if(index==ts->index_hover) {
                SetTextColor(memdc,RGB(0,100,200));
            }
            ImageList_DrawEx(TabCtrl_GetImageList(hwnd),index,memdc,rcIcon.left,rcIcon.top+((rcIcon.bottom-rcIcon.top-16)>>1),16,16,CLR_DEFAULT,CLR_DEFAULT,ILD_NORMAL);
            DrawText(memdc,ti.pszText,-1,&rcLabel,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
            
            //绘制关闭. 手绘
            if(ts->tab_pos==TAB_TOP) {
                rcClose.left=rcClose.right-(rcTmp.bottom-rcTmp.top);
                InflateRect(&rcClose,-((rcClose.right-rcClose.left-12)>>1),-((rcClose.right-rcClose.left-12)>>1));
                
                Graphics graphic(memdc);
                SolidBrush gra_brush((index==ts->index_hover)?Color(255,150,0,0):Color(255,53,53,53));
                graphic.SetSmoothingMode(SmoothingModeHighQuality);
                graphic.FillEllipse(&gra_brush,RectF(rcClose.left,rcClose.top,12,12));
                Pen pen(Color(255,150,150,150),2.5);
                InflateRect(&rcClose,-3,-3);
                graphic.DrawLine(&pen,rcClose.left,rcClose.top,rcClose.right,rcClose.bottom);
                graphic.DrawLine(&pen,rcClose.right,rcClose.top,rcClose.left,rcClose.bottom);
            }
            
            if((ts->tab_pos==TAB_TOP)&&(rcTab.left>ps.rcPaint.right)) break;
            if((ts->tab_pos==TAB_LEFT)&&(rcTab.top>ps.rcPaint.bottom)) break;
            
            SetTextColor(memdc,ts->color_text);
        }
    }
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

int Tab_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL IsAllign_TopLeft)
{
    UINT style=GetWindowLongPtr(hwnd,GWL_STYLE);
    pTabStyle tb=Tab_GetSettings(hwnd);
    RECT rc,rc_client;
    
    if(!tb) return -1;
        
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

LRESULT CALLBACK TabUpDownOwnerProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
HWND Tab_GetUpDown(HWND hwnd)
{
    RSearchParam sp={hwnd,UPDOWN_CLASS,NULL};
    EnumChildWindows(hwnd,UpDownCheckProc,(LPARAM)&sp);
    
    if(sp.hwnd) {
        if(TabUpDownOwnerProc!=(WNDPROC)GetWindowLongPtr(sp.hwnd,GWLP_WNDPROC)) {
            SetWindowLongPtr(sp.hwnd,GWLP_USERDATA,(LONG_PTR)SetWindowLongPtr(sp.hwnd,GWLP_WNDPROC,(LONG_PTR)TabUpDownOwnerProc));
        }
    }
    return sp.hwnd;
}

LRESULT CALLBACK TabUpDownOwnerProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    WNDPROC pre_proc=(WNDPROC)GetWindowLongPtr(hwnd,GWLP_USERDATA);
    switch(message) {
    case WM_PAINT:{
        PAINTSTRUCT ps={0};
        BeginPaint(hwnd,&ps);
        HDC hdc=ps.hdc;
        
        RECT rc,rcUp,rcDown;
        GetClientRect(hwnd,&rc);
        CopyRect(&rcUp,&rc);
        CopyRect(&rcDown,&rc);
        rcUp.right=((rc.right+rc.left)>>1);
        rcDown.left=rcUp.right;
        HPEN pen=CreatePen(PS_SOLID,1,RGB(53,53,53));
        HBRUSH brush=CreateSolidBrush(RGB(20,20,20));
        SelectObject(hdc,pen);
        SelectObject(hdc,brush);
        Rectangle(hdc,rcUp.left,rcUp.top,rcUp.right,rcUp.bottom);
        Rectangle(hdc,rcDown.left,rcDown.top,rcDown.right,rcDown.bottom);
        
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd,&pt);
        SetBkMode(hdc,TRANSPARENT);
        if(PtInRect(&rcUp,pt)) SetTextColor(hdc,RGB(200,0,0));
        else SetTextColor(hdc,RGB(53,53,53));
        DrawText(hdc,"<<",-1,&rcUp,DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS);
        
        if(PtInRect(&rcDown,pt)) SetTextColor(hdc,RGB(200,0,0));
        else SetTextColor(hdc,RGB(53,53,53));
        DrawText(hdc,">>",-1,&rcDown,DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS);
        
        DeleteObject(pen);
        DeleteObject(brush);
        EndPaint(hwnd,&ps);
        return 0;
    } break;
    case WM_MOUSEMOVE: {
        InvalidateRect(hwnd,NULL,FALSE);
    } break;
    }
    return CallWindowProc(pre_proc,hwnd,message,wParam,lParam);
}

int Tab_AppendPage(HWND tab,HWND page,char* title)
{
    TCITEM ti={0};
    
    int count=TabCtrl_GetItemCount(tab);
    int index=count;
    
    pTabItem pi=(pTabItem)calloc(sizeof(RTabItem),1);
    pi->hwnd=page;
    pi->index=index;
    
    ti.mask=TCIF_TEXT|TCIF_IMAGE|TCIF_PARAM;
    ti.pszText=title;
    ti.cchTextMax=256;
    ti.iImage=index;
    ti.lParam=(LPARAM)pi;
    TabCtrl_InsertItem(tab,index,&ti);
    TabCtrl_SetItem(tab,index,&ti);
    
    return 0;
}