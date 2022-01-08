#include "../include/container.h"
#include "../include/listbox.h"

LRESULT CALLBACK FrameProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    pFrameStyle fs=Frame_GetSettings(hwnd);
    
	switch (msg) {
	case WM_CREATE: {
	    InitCommonControls();
	} break;
	case WM_NCCALCSIZE: {
        return Frame_NCCalcSize(hwnd,wParam,lParam);
	} break;
	case WM_NCACTIVATE:
	case WM_NCPAINT: {
        BOOL active_flag=(BOOL)wParam;
        Frame_NCDraw(hwnd,&active_flag);
        return !active_flag;	    
	} break;/*
	case WM_PAINT: {
        PAINTSTRUCT ps={0};
        BeginPaint(hwnd,&ps);
        HDC hdc=ps.hdc;
        HBRUSH brush=CreateSolidBrush(fs->color_bk);
        FillRect(hdc,&ps.rcPaint,brush);
        DeleteObject(brush);
        EndPaint(hwnd,&ps);
        return 0;
	} break;*/
    case WM_NCMOUSEMOVE: {
        UINT HITPOS=wParam;
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        BOOL active_flag=(GetActiveWindow()==hwnd)||
                         (GetFocus()==hwnd)&&((WS_CHILD&GetWindowLongPtr(hwnd,GWL_STYLE))==WS_CHILD);
        Frame_NCDraw(hwnd,&active_flag);
        Frame_TrackNCMouseLeave(hwnd);
        return 0;
    } break;
    case WM_NCMOUSELEAVE: {
        BOOL active_flag=(GetActiveWindow()==hwnd)||
                         (GetFocus()==hwnd)&&((WS_CHILD&GetWindowLongPtr(hwnd,GWL_STYLE))==WS_CHILD);
        Frame_NCDraw(hwnd,&active_flag);
        
        Frame_StopNCMouseTrack(hwnd);
    } break;
    case WM_MOVE: { //有bug ,不同进程的frame交叠的时候程序崩溃
        POINT pt;
        RECT rc,rc_active;
        HWND hwnd_bk=NULL;
        //pFrameStyle fs=Frame_GetSettings(hwnd);
        pHintInfo phint=Frame_GetHintSettings(hwnd);
        //Frame_LogMsg(hmsg," 0x%08X WM_MOVE.",hwnd);
        if((GetWindowLongPtr(hwnd,GWL_STYLE)&WS_CAPTION)!=WS_CAPTION) {
            //Frame_LogMsg(hmsg," 0x%08X 该窗口没有caption",hwnd);
            break;
        }
        
        //hint 位置调整，与frame同步
        if(phint&&fs) Frame_AllignHintWindow(hwnd);
        
        //拖拽至Frame，调整hint的显隐。
        GetCursorPos(&pt);
        hwnd_bk=Frame_GetFrameFromCursor(hwnd);
        if(hwnd_bk==NULL) break;
        pFrameStyle fs_bk=Frame_GetSettings(hwnd_bk);  
        phint=Frame_GetHintSettings(hwnd_bk);
                    
        GetWindowRect(hwnd_bk,&rc);
        CopyRect(&rc_active,&rc);
        InflateRect(&rc_active,-20,-20);
        
        if(PtInRect(&rc_active,pt)&&GetAsyncKeyState(VK_LBUTTON)) {
            SetWindowLongPtr(fs_bk->hint,GWL_STYLE,WS_VISIBLE|GetWindowLongPtr(fs_bk->hint,GWL_STYLE));
        }
        else {//无法触发
            SetWindowLongPtr(fs_bk->hint,GWL_STYLE,(~WS_VISIBLE)&GetWindowLongPtr(fs_bk->hint,GWL_STYLE));
        }
        SetWindowPos(fs_bk->hint,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
        SendMessage(fs_bk->hint,WM_DOCKINGONTHEWAY,0,MAKELPARAM(pt.x,pt.y));
    } break;
    case WM_EXITSIZEMOVE: {
        HWND hwnd_bk=Frame_GetFrameFromCursor(hwnd);
        pFrameStyle fs_bk=NULL;
        pHintInfo phint=NULL;
        
        if(hwnd_bk==NULL) break;
        fs_bk=Frame_GetSettings(hwnd_bk);
        phint=Frame_GetHintSettings(hwnd_bk);
        SetWindowLongPtr(fs_bk->hint,GWL_STYLE,(~WS_VISIBLE)&GetWindowLongPtr(fs_bk->hint,GWL_STYLE));
        SetWindowPos(fs_bk->hint,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_HIDEWINDOW);
        
        //窗口停靠
        phint->hintdock(hwnd_bk,hwnd,fs_bk->hint);
    } break;
    case WM_INITMENUPOPUP: {
        Frame_InitialMenuPopup(hwnd,wParam,lParam);
    } break;    
    case WM_NCHITTEST: {
        return Frame_NCHitTest(hwnd,wParam,lParam);
    } break;
    case WM_SYSCOMMAND: {
        if(wParam==SC_KEYMENU) {
            BOOL active_flag=TRUE;
            Frame_NCDraw(hwnd,&active_flag);
            //可以捕捉并手工处理菜单消息
            //switch(lParam) { //加速键
            //case 0: {
            //    //menubar activate. F10 与 Alt触发
            //} break;
            //case 'f': {
            //    //快捷键F
            //} break;
            //}
            return 0;
        }
    } break;
    case WM_NCLBUTTONDOWN: {
        UINT HITPOS = wParam;
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        
        if(HITPOS==HTMENU) {
            Frame_PopMenu(hwnd,pt);
            return 0;
        }
        else if(HITPOS==HTMINBUTTON||HITPOS==HTMAXBUTTON||HITPOS==HTCLOSE) return 0;
        else if(HITPOS==HTHSCROLL) Frame_ViewHBeginDrag(hwnd,pt);
    } break;
    case WM_MOUSEMOVE: {
        POINT pt_cur={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        if(GetCapture()==hwnd) {
            Frame_ViewHDraging(hwnd,pt_cur);
        }
    } break;
    case WM_LBUTTONUP: {
        if(GetCapture()==hwnd) {
            Frame_ViewHStopDrag(hwnd);
        }
    }
    case WM_NCLBUTTONUP: {
        UINT HITPOS = wParam;
        if(HITPOS==HTMINBUTTON) ShowWindow(hwnd,SW_MINIMIZE);
        else if(HITPOS==HTMAXBUTTON) {
            if(IsZoomed(hwnd)) ShowWindow(hwnd,SW_RESTORE);
            else ShowWindow(hwnd,SW_MAXIMIZE);
        }
        else if(HITPOS==HTCLOSE) SendMessage(hwnd,WM_CLOSE,0,0);
    } break;
    case WM_SIZE: {		
        int height = HIWORD(lParam);
	    int width = LOWORD(lParam);
		RECT window_rect,rc_client;
        HRGN window_rgn;
        
		if(fs&&fs->hint) {
		    Frame_AllignHintWindow(hwnd);
		    pHintInfo phint=Hint_GetSettings(fs->hint);
		    if(phint&&phint->docksize) phint->docksize(hwnd);
		}
        
        //Rectangle the window   
        GetWindowRect(hwnd,&window_rect);
        window_rgn=CreateRectRgn(0,0,window_rect.right-window_rect.left,window_rect.bottom-window_rect.top);
        SetWindowRgn(hwnd,window_rgn,TRUE);
        DeleteObject(window_rgn);
        
        //更新高宽
        GetClientRect(hwnd,&rc_client);
        height=rc_client.bottom-rc_client.top;
        width=rc_client.right-rc_client.left;
        //设置SCROLLINFO
        if(Frame_GetViewSI(hwnd)) Frame_ViewHSize(hwnd,height);
    } break;
    case WM_NCDESTROY: {
        Frame_ClearSettings(hwnd);
    } break;
    case WM_MEASUREITEM: {
        LPMEASUREITEMSTRUCT lpmis;
        lpmis=(LPMEASUREITEMSTRUCT)lParam;
        
        if(lpmis->CtlType==ODT_MENU) {
            UINT menu_item_id=lpmis->itemID;
            lpmis->itemWidth=200;
            lpmis->itemHeight=((menu_item_id==0x0)?5:28);//menu item & menu spliter
        }
    } break;
	case WM_DRAWITEM: {
	    UINT ctrl_id=(UINT)wParam;
	    LPDRAWITEMSTRUCT pDraw=(LPDRAWITEMSTRUCT)lParam;
	    
	    if(pDraw->CtlType==ODT_MENU) {
	        Frame_DrawMenuItem(pDraw);
	    }
	} break;
	case WM_CTLCOLORLISTBOX: {
	    HDC hdc=(HDC)wParam;
	    HWND lb=(HWND)lParam;
	    pLBStyle ls=ListBox_GetSettings(lb);
	    if(!ls) break;
	    SetTextColor(hdc,ls->color_text);
	    SetBkColor(hdc,ls->color_bk);
	    return (HRESULT)(ls->brush);
	} break;
	}
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int Frame_ClearSettings(HWND hwnd)
{
    pFrameStyle fs=Frame_GetSettings(hwnd);
    if(!fs) return -1;
    
    if(fs->pvsi) Frame_ClearViewSI(hwnd);
    if(fs->hint) Hint_ClearSettings(fs->hint);
    
    DeleteObject(fs->font);
    DeleteObject(fs->brush_mibk);
    DeleteObject(fs->brush_cap_active);
    DeleteObject(fs->brush_cap_deactive);
    DeleteObject(fs->brush_border);
    DeleteObject(fs->brush_scroll);
    DeleteObject(fs->pen);
    DeleteObject(fs->brush_btnclose);
    DeleteObject(fs->brush_btn);
    
    free(fs);
    
    return 0;
}

pFrameStyle Frame_GetSettings(HWND hwnd)
{
    return (pFrameStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int Frame_InitialSettings(HWND hwnd)
{
    pFrameStyle fs=(pFrameStyle)calloc(sizeof(RFrameStyle),1);
    if(!fs) return -1;
    
    fs->font=CreateFont(17,0,0,0,
                        FW_MEDIUM,//FW_SEMIBOLD,
                        FALSE,FALSE,FALSE,
                        DEFAULT_CHARSET,
                        OUT_OUTLINE_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, 
                        VARIABLE_PITCH,
                        "Courier New");
    SendMessage(hwnd,WM_SETFONT,(WPARAM)fs->font,0);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)fs);
    
    fs->color_mibk=RGB(43,43,43);
    fs->brush_mibk=CreateSolidBrush(fs->color_mibk);
    
    fs->color_active=RGB(30,30,30);
    fs->brush_cap_active=CreateSolidBrush(fs->color_active);
    fs->color_deactive=RGB(40,40,40);
    fs->brush_cap_deactive=CreateSolidBrush(fs->color_deactive);
    
    fs->color_border=RGB(53,53,53);
    fs->brush_border=CreateSolidBrush(fs->color_border);
    
    fs->color_text=RGB(255,255,255);
    fs->color_bk=RGB(16,16,16);
    
    fs->pen=CreatePen(PS_SOLID,1,fs->color_border);
    
    fs->brush_btnclose=CreateSolidBrush(RGB(255,0,0));
    fs->brush_btn=CreateSolidBrush(RGB(200,200,200));
    
    fs->color_scroll=RGB(0,0,0);
    fs->brush_scroll=CreateSolidBrush(fs->color_scroll);
    
    fs->proc=FrameProc;
    fs->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)fs->proc); 
    
    return 0;
}

int Frame_PopMenu(HWND hwnd,POINT pt)
{
    int item_count;
    HMENU menu;
    HMENU sub_menu;
    RECT mi_rc,win_rc;
    POINT pos;

    GetWindowRect(hwnd,&win_rc);
    pos=pt;
    pt.x=pt.x-win_rc.left;
    pt.y=pt.y-win_rc.top;
    
    menu=GetMenu(hwnd);
    item_count=GetMenuItemCount(menu);
    for(int index=0;index<item_count;index++) {
        Frame_GetMenuItemRect(hwnd,index,&mi_rc);
        if(PtInRect(&mi_rc,pt)) {
            sub_menu=GetSubMenu(menu,index);
            TrackPopupMenu(sub_menu,TPM_LEFTALIGN|TPM_TOPALIGN,mi_rc.left+win_rc.left,mi_rc.bottom+win_rc.top,0,hwnd,NULL);                                
            return 0;
        }
    }
    return 0;
}

BOOL Frame_IsMenuBarRect(HWND hwnd,POINT pt)
{
    int item_count;
    RECT mb_rc;
    MENUBARINFO mb_if;
    
    Frame_GetMenuItemRect(hwnd,0,&mb_rc);
    mb_if.cbSize=sizeof(mb_if);
    GetMenuBarInfo(hwnd,OBJID_MENU,0,&mb_if);
    OffsetRect(&(mb_if.rcBar),-mb_if.rcBar.left,-mb_if.rcBar.top);
    mb_rc.left=mb_if.rcBar.left;
    mb_rc.right=mb_if.rcBar.right;
    
    return PtInRect(&mb_rc,pt);
}

int Frame_GetMenuItemRect(HWND hwnd,int index,LPRECT prc)
{
    HMENU menu;
    MENUBARINFO mb_if;
    POINT pt_offset;
    RECT rc_bar;
    RMetrics mt;
    
    Frame_GetNCZoneRect(hwnd,ZMENUBAR,&rc_bar,TRUE);
    menu=GetMenu(hwnd);
    mb_if.cbSize=sizeof(mb_if);
    GetMenuBarInfo(hwnd,OBJID_MENU,1,&mb_if);
    pt_offset={rc_bar.left-mb_if.rcBar.left,rc_bar.top-mb_if.rcBar.top};
    
    Frame_GetMetrics(hwnd,&mt);
    GetMenuBarInfo(hwnd,OBJID_MENU,index+1,&mb_if);
    CopyRect(prc,&(mb_if.rcBar));
    OffsetRect(prc,pt_offset.x+index*mt.ncbtn_margin+mt.border_pixls,pt_offset.y);
    
    return 0;
}

int Frame_ScrollBarDraw(HWND hwnd)
{
    RECT rc_hs,rc_thumb,rc_wndhs;
    RMetrics mt;
    pFrameStyle fs=Frame_GetSettings(hwnd);
    
    if(Frame_GetNCZoneRect(hwnd,ZHSCROLL,&rc_hs,TRUE)!=1) return 0;
    if(Frame_GetNCZoneRect(hwnd,ZHSTHUMB,&rc_thumb,TRUE)!=1) return 0;
    
    Frame_GetMetrics(hwnd,&mt);
    HDC hdc=GetWindowDC(hwnd);
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,rc_hs.right-rc_hs.left,rc_hs.bottom-rc_hs.top);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    RECT rcMem;
    CopyRect(&rcMem,&rc_hs);
    OffsetRect(&rcMem,-rcMem.left,-rcMem.top);
    OffsetRect(&rc_thumb,-rc_hs.left,-rc_hs.top);
    FillRect(memdc,&rcMem,fs->brush_scroll);
    
    Graphics graphic(memdc);
    GraphicsPath path;
    LinearGradientBrush pbrush(Rect(rc_thumb.left,rc_thumb.top,rc_thumb.right-rc_thumb.left,rc_thumb.bottom-rc_thumb.top),
                               Color(255,GetRValue(fs->color_border+((GetCapture()==hwnd)?20:0)),GetGValue(fs->color_border)+((GetCapture()==hwnd)?20:0),GetBValue(fs->color_border)+((GetCapture()==hwnd)?20:0)),
                               Color(255,GetRValue(fs->color_scroll),GetGValue(fs->color_scroll),GetBValue(fs->color_scroll)),
                               LinearGradientModeHorizontal);
    graphic.SetSmoothingMode(SmoothingModeHighQuality);
    path.AddArc(rc_thumb.left,rc_thumb.top,mt.scroll_pixls,mt.scroll_pixls,180,180);
    path.AddArc(rc_thumb.left,rc_thumb.bottom-mt.scroll_pixls,mt.scroll_pixls,mt.scroll_pixls,0,180);
    graphic.FillPath(&pbrush,&path);
    
    BitBlt(hdc,rc_hs.left,rc_hs.top,(rc_hs.right-rc_hs.left),(rc_hs.bottom-rc_hs.top),memdc,0,0,SRCCOPY);
    
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    ReleaseDC(hwnd,hdc);
    
    return 0;
}

int Frame_NCDraw(HWND hwnd,LPVOID param)
{
    BOOL active_flag=*(BOOL*)param;
    pFrameStyle fs=Frame_GetSettings(hwnd);
    RMetrics mt={0};
    HICON icon;
    HICON icon_cfg;
    HICON icon_msg;
    RECT rc={0},rc_mem={0},rc_cap,rc_ico,rc_title,rc_bar,rc_min,rc_max,rc_x,rc_cfg,rc_msg;
    POINT pt_cursor;
    char text[256]="";
    COLORREF color_trans=RGB(0,0,1);
    HBRUSH brush_trans=CreateSolidBrush(color_trans),pre_brush;
    
    if(!fs) return 1;
    
    GetWindowRect(hwnd,&rc);
    CopyRect(&rc_mem,&rc);
    OffsetRect(&rc_mem,-rc_mem.left,-rc_mem.top);
    
    //获取鼠标位置
    GetCursorPos(&pt_cursor);
    pt_cursor.x-=rc.left;
    pt_cursor.y-=rc.top;
    
    //刷新画板
    HDC hdc=GetWindowDC(hwnd);
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,rc_mem.right,rc_mem.bottom);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    SelectObject(memdc,fs->font);//(HFONT)SendMessage(hwnd,WM_GETFONT,0,0));//失败，不知道为什么
    FillRect(memdc,&rc_mem,brush_trans);
    DeleteObject(brush_trans);
    
    Frame_GetMetrics(hwnd,&mt);
    //填充标题
    if(Frame_GetNCZoneRect(hwnd,ZCAPTION,&rc_cap,TRUE)==1) {
        rc_cap.bottom++;
        //Frame_GradientRect(memdc,rc_cap,RGB(15,15,15),(active_flag?fs->color_active:fs->color_deactive),1);
        Frame_GradientRect(memdc,rc_cap,(active_flag?fs->color_active:fs->color_deactive),(active_flag?fs->color_active:fs->color_deactive),1);
    }
    //绘制图标
    if(Frame_GetNCZoneRect(hwnd,ZICON,&rc_ico,TRUE)==1) {
        icon=(HICON)SendMessage(hwnd,WM_GETICON,(WPARAM)ICON_SMALL,0);//ICON_BIG 大图标
        if(icon==NULL) icon=(HICON)GetClassLongPtrW(hwnd, GCLP_HICONSM);
        if(icon == NULL) {
            HWND hwnd_owner=GetWindow(hwnd,GW_OWNER);
            if(hwnd_owner) icon=(HICON)GetClassLongPtrW(hwnd_owner, GCLP_HICONSM);
            if(icon==NULL) icon=(HICON)LoadIcon(NULL,IDI_APPLICATION);
        }
        DrawIconEx(memdc,rc_ico.left,rc_ico.top,icon,mt.icon_pixls,mt.icon_pixls,0,0,DI_NORMAL);//是否需要释放?
    }
    //绘制标题文本
    if(Frame_GetNCZoneRect(hwnd,ZTEXTTITLE,&rc_title,TRUE)==1) {
        GetWindowText(hwnd,text,sizeof(text));
        SetBkMode(memdc,TRANSPARENT);
        SetTextColor(memdc,fs->color_text);
        DrawText(memdc,text,-1,&rc_title,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
    }
    //绘制MenuBar
    if(Frame_GetNCZoneRect(hwnd,ZMENUBAR,&rc_bar,TRUE)==1) {
        HMENU menu;
        MENUBARINFO mb_if;
        MENUITEMINFO mi_if;
        int count;
        char menu_text[256]="";
        RECT rc_mi;//菜单项区域
        POINT pt_offset;
        
        FillRect(memdc,&rc_bar,active_flag?fs->brush_cap_active:fs->brush_cap_deactive);
        //绘制菜单项.
        menu=GetMenu(hwnd);
        count=GetMenuItemCount(menu);
        
        mb_if.cbSize=sizeof(mb_if);
        for(int index=1;index<=count;index++) {
            GetMenuBarInfo(hwnd,OBJID_MENU,index,&mb_if);//菜单项的index从1开始
            
            mi_if.cbSize=sizeof(mi_if);
            mi_if.fMask=MIIM_STRING|MIIM_STATE;
            mi_if.dwTypeData=menu_text;
            mi_if.cch=sizeof(menu_text);
            GetMenuItemInfo(menu,index-1,TRUE,&mi_if);
            
            if(index==1) pt_offset={rc_bar.left-mb_if.rcBar.left,rc_bar.top-mb_if.rcBar.top};
            
            CopyRect(&rc_mi,&(mb_if.rcBar));
            OffsetRect(&rc_mi,pt_offset.x+index*mt.ncbtn_margin+mt.border_pixls,pt_offset.y);
            rc_mi.right+=8;
            if(PtInRect(&rc_mi,pt_cursor)) {
                Frame_GradientRect(memdc,rc_mi,RGB(15,15,15),RGB(55,55,55),1);
                FrameRect(memdc,&rc_mi,fs->brush_border);
            }
            DrawText(memdc,menu_text,-1,&rc_mi,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
        }
    }
    
    //绘制关闭
    if(Frame_GetNCZoneRect(hwnd,ZCLOSE,&rc_x,TRUE)==1) {
        if(PtInRect(&rc_x,pt_cursor)) FillRect(memdc,&rc_x,fs->brush_btnclose);//关闭
        DrawText(memdc,"×",-1,&rc_x,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
    }
    //绘制最大化
    if(Frame_GetNCZoneRect(hwnd,ZMAX,&rc_max,TRUE)==1) {
        if(PtInRect(&rc_max,pt_cursor)) FillRect(memdc,&rc_max,fs->brush_btn);
        if(IsZoomed(hwnd)) {
            //最大化 
            DrawText(memdc,"□",-1,&rc_max,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
            OffsetRect(&rc_max,2,-2);
            DrawText(memdc,"□",-1,&rc_max,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
        }
        else {
            //恢复
            DrawText(memdc,"□",-1,&rc_max,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
        }
    }
    
    //绘制最小化
    if(Frame_GetNCZoneRect(hwnd,ZMIN,&rc_min,TRUE)==1) {
        if(PtInRect(&rc_min,pt_cursor)) FillRect(memdc,&rc_min,fs->brush_btn);
        DrawText(memdc,"-",-1,&rc_min,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
    }
    
    //绘制config
    if(Frame_GetNCZoneRect(hwnd,ZCONFIG,&rc_cfg,TRUE)==1) {
        if(PtInRect(&rc_cfg,pt_cursor)) {
            icon_cfg=LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_CFG2");
        }
        else icon_cfg=LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_CFG");
        InflateRect(&rc_cfg,-2,-2);
        DrawIconEx(memdc,rc_cfg.left,rc_cfg.top,icon_cfg,(rc_cfg.right-rc_cfg.left),(rc_cfg.bottom-rc_cfg.top),0,0,DI_NORMAL);
    }
    //绘制msg
    if(Frame_GetNCZoneRect(hwnd,ZMSG,&rc_msg,TRUE)==1) {
        if(PtInRect(&rc_msg,pt_cursor)) {
            icon_msg=LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_MSG");
        }
        else icon_msg=LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),"IDC_ICON_MSG2");
        InflateRect(&rc_msg,-2,-3);
        DrawIconEx(memdc,rc_msg.left,rc_msg.top,icon_msg,(rc_msg.right-rc_msg.left),(rc_msg.bottom-rc_msg.top),0,0,DI_NORMAL);
    }
    //绘制边框
    if(WS_BORDER==(GetWindowLongPtr(hwnd,GWL_STYLE)&WS_BORDER)) {
        SelectObject(memdc,fs->pen);
        for(int index=0;index<mt.border_pixls;index++) {
             FrameRect(memdc,&rc_mem,fs->brush_border);
             InflateRect(&rc_mem,-1,-1);
        }
    }
    
    //绘制滚动条
    Frame_ScrollBarDraw(hwnd);
    
    OffsetRect(&rc,-rc.left,-rc.top);
    TransparentBlt(hdc,rc.left,rc.top,(rc.right-rc.left),(rc.bottom-rc.top),
                   memdc,0,0,(rc.right-rc.left),(rc.bottom-rc.top),color_trans);
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    ReleaseDC(hwnd,hdc);
                   
    return 0;
}

int Frame_CopyBtnRect(HWND hwnd,LPRECT pbtn_rc,int btn_count)
{
    RECT rc_frame;
    int rc_length;
    int btn_height=20,btn_width=25;
    int btn_margin=10;//最按钮距离右侧边框的pixs
    
    if(hwnd==NULL) return -1;
    if(pbtn_rc==NULL) return -1;
    
    GetWindowRect(hwnd,&rc_frame);
    OffsetRect(&rc_frame,-rc_frame.left,-rc_frame.top);
    rc_length=rc_frame.right-rc_frame.left;
    
    for(int index=btn_count;index>0;index--) {
        pbtn_rc[index-1]={rc_length-btn_margin-btn_width*index,btn_margin,rc_length-btn_margin-btn_width*(index-1),btn_margin+btn_height};
    }
    
    return 0;
}

void Frame_GradientRect(HDC hdc,RECT rc_tmp,COLORREF c1,COLORREF c2,int verical_or_horizen)
{
    RECT rc;
    CopyRect(&rc,&rc_tmp);
    //下面的代码很迷糊，但是有效，不知道GDI+的填充范围怎么搞的
    OffsetRect(&rc,((verical_or_horizen==0)?0:0),(verical_or_horizen==1)?-1:0);
    
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


int Frame_DrawMenuItem(LPDRAWITEMSTRUCT draw)
{
    UINT menu_item_id=draw->itemID;
    char text[256]="";
    RECT rect;
    HDC hdc;
    HMENU menu;
    HWND menu_hwnd;
    MENUITEMINFO mi_if={0}/*,itemtype={0}*/;
    COLORREF rgb_bk=RGB(43,43,43);//RGB(70,70,70);
    COLORREF rgb_disabled=RGB(150,150,150);
    COLORREF rgb_text_disabled=RGB(100,100,100);
    COLORREF rgb_check=RGB(65,65,65);
    COLORREF rgb_text_check=RGB(0,128,255);
    COLORREF rgb_text=RGB(255,255,255);
    HBRUSH brush,pre_brush;
    HPEN pen,pre_pen;
    
    hdc=draw->hDC;
    InflateRect(&(draw->rcItem),1,1);
    CopyRect(&rect,&(draw->rcItem));
    menu=(HMENU)draw->hwndItem;
    menu_hwnd=draw->hwndItem;
    
    mi_if.cbSize=sizeof(mi_if);
    mi_if.fMask=MIIM_STRING|MIIM_STATE|MIIM_ID;
    mi_if.dwTypeData=text;
    mi_if.cch=sizeof(text);
    GetMenuItemInfo(menu,menu_item_id,FALSE,&mi_if);
    
    if(draw->itemState&ODS_CHECKED==ODS_CHECKED) {
        if(mi_if.fState&MFS_DISABLED==MFS_DISABLED) {
            brush=CreateSolidBrush(rgb_bk);
            pre_brush=(HBRUSH)SelectObject(hdc,brush);
            SetTextColor(hdc,rgb_text_disabled);
            pen=CreatePen(PS_SOLID,1,rgb_text_disabled);
            pre_pen=(HPEN)SelectObject(hdc,pen);
        }
        else {
            brush=CreateSolidBrush(rgb_check);
            pre_brush=(HBRUSH)SelectObject(hdc,brush);
            SetTextColor(hdc,rgb_text_check);
            pen=CreatePen(PS_SOLID,1,rgb_check);
            pre_pen=(HPEN)SelectObject(hdc,pen);
        }
    }
    else {
        brush=CreateSolidBrush(rgb_bk);
        pre_brush=(HBRUSH)SelectObject(hdc,brush);
        if(mi_if.fState&MFS_DISABLED==MFS_DISABLED) {
            SetTextColor(hdc,rgb_text_disabled);
            pen=CreatePen(PS_SOLID,1,rgb_text_disabled);
            pre_pen=(HPEN)SelectObject(hdc,pen);
        }
        else {
            SetTextColor(hdc,rgb_text);
            pen=CreatePen(PS_SOLID,1,rgb_bk);
            pre_pen=(HPEN)SelectObject(hdc,pen);
        }
    }

    SetBkMode(hdc,TRANSPARENT);
    FillRect(hdc,&rect,brush);
    if(strlen(text)>0) {
        RECT check_rect;
        CopyRect(&check_rect,&rect);
        check_rect.right=rect.left+(rect.bottom-rect.top);

        if((UINT)(mi_if.fState&MFS_CHECKED)==(UINT)MFS_CHECKED) {
            DrawText(hdc,"√",-1,&check_rect,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
        }
        rect.left = rect.left+50;//(rect.bottom-rect.top);
        DrawText(hdc,text,-1,&rect,DT_SINGLELINE|DT_VCENTER|DT_EXPANDTABS);//DT_EXPANDTABS识别\t
    }
    else {
        //绘制菜单分隔条
        POINT pt[5]={{rect.left+5,(rect.top+rect.bottom)/2},
                     {rect.right-5,(rect.top+rect.bottom)/2},
                     {rect.left+5,(rect.top+rect.bottom)/2+1},
                     {rect.right-5,(rect.top+rect.bottom)/2+1}};
        
        COLORREF color_1=RGB(19,19,19),color_2=RGB(68,68,68);
        HPEN sp_line1=CreatePen(PS_SOLID,1,color_1);
        HPEN sp_line2=CreatePen(PS_SOLID,1,color_2);
        HPEN pre_sp;
        
        pre_sp=(HPEN)SelectObject(hdc,sp_line1);
        MoveToEx(hdc,pt[0].x,pt[0].y,&(pt[4]));
        LineTo(hdc,pt[1].x,pt[1].y);
        
        SelectObject(hdc,sp_line2);
        MoveToEx(hdc,pt[2].x,pt[2].y,&(pt[4]));
        LineTo(hdc,pt[3].x,pt[3].y);
        SelectObject(hdc,pre_sp);
        DeleteObject(sp_line1);
        DeleteObject(sp_line2);
    }
    
    DeleteObject(SelectObject(hdc,pre_brush));
    DeleteObject(SelectObject(hdc,pre_pen));
    return 0;
}

int Frame_GetMetrics(HWND hwnd,pMetrics mt)
{
    if((hwnd==NULL)||(!mt)) return -1;
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL IsChild=((style&WS_CHILD)==WS_CHILD);
    BOOL IsCaption=((style&WS_CAPTION)==WS_CAPTION);
    BOOL IsBorder=((style&WS_BORDER)==WS_BORDER);
    BOOL IsMinButton=((style&WS_MINIMIZEBOX)==WS_MINIMIZEBOX);
    BOOL IsMaxButton=((style&WS_MAXIMIZEBOX)==WS_MAXIMIZEBOX);
    BOOL IsMenu=(GetMenu(hwnd)!=NULL);
    
    if(!IsChild) {
        mt->icon_margin=mt->ncbtn_margin=(IsCaption?5:0);
        mt->icon_pixls=(IsCaption?24:0);
        mt->ncbtn_cx=mt->ncbtn_cy=(IsMinButton?20:0);
        mt->menubar_margin=(IsMenu?5:0);
        mt->menubar_cy=(IsMenu?24:0);
        mt->border_pixls=(IsBorder?1:0); 
        mt->scroll_pixls=13;
        mt->min_thumb_pixls=50;
    }
    else {
        mt->icon_margin=mt->ncbtn_margin=(IsCaption?3:0);
        mt->icon_pixls=(IsCaption?16:0);
        mt->ncbtn_cx=mt->ncbtn_cy=(IsMinButton?16:0);
        mt->menubar_margin=0;
        mt->menubar_cy=0;
        mt->border_pixls=(IsBorder?1:0);
        mt->scroll_pixls=13;
        mt->min_thumb_pixls=50;
    }
    
    return 0;
}

/*
allign_topleft:TRUE 相对于窗口左上角的坐标
               FALSE 屏幕坐标
 */
int Frame_GetNCZoneRect(HWND hwnd,EFNCZone type,LPRECT prc,BOOL allign_topleft)
{
    RECT rc={0};
    RMetrics mt={0};
    int assign_flag=-1;
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
    BOOL IsCaption=((style&WS_CAPTION)==WS_CAPTION);
    BOOL IsCloseButton=IsCaption;//((style&WS_CAPTION)==WS_CAPTION);
    BOOL IsMaxButton=((style&WS_MAXIMIZEBOX)==WS_MAXIMIZEBOX);
    BOOL IsMinButton=((style&WS_MINIMIZEBOX)==WS_MINIMIZEBOX);
    BOOL IsMenu=(GetMenu(hwnd)!=NULL);
    BOOL IsHScroll=((style&WS_HSCROLL)==WS_HSCROLL);
    
    GetWindowRect(hwnd,&rc);
    Frame_GetMetrics(hwnd,&mt);
    
    switch(type) {
    case ZCLOSE: {
        if(!IsCloseButton) break;
        prc->top=rc.top+mt.ncbtn_margin;
        prc->bottom=prc->top+mt.ncbtn_cy;
        prc->right=rc.right-mt.ncbtn_margin;
        prc->left=prc->right-mt.ncbtn_cx;
        assign_flag=1;
    } break;
    case ZMAX: {
        if(!IsMaxButton) break;
        prc->top=rc.top+mt.ncbtn_margin;
        prc->bottom=prc->top+mt.ncbtn_cy;
        prc->right=rc.right-mt.ncbtn_margin-(IsCloseButton?mt.ncbtn_cx:0);
        prc->left=prc->right-mt.ncbtn_cx;
        assign_flag=1;
    } break;
    case ZMIN: {
        if(!IsMinButton) break;
        prc->top=rc.top+mt.ncbtn_margin;
        prc->bottom=prc->top+mt.ncbtn_cy;
        prc->right=rc.right-mt.ncbtn_margin-(IsCloseButton?mt.ncbtn_cx:0)-(IsMaxButton?mt.ncbtn_cx:0);
        prc->left=prc->right-mt.ncbtn_cx;
        assign_flag=1;
    } break;
    case ZICON: {
        if(!IsCaption) break;
        prc->top=rc.top+mt.icon_margin;
        prc->bottom=prc->top+mt.icon_pixls;
        prc->left=rc.left+mt.icon_margin;
        prc->right=prc->left+mt.icon_pixls;
        assign_flag=1;
    } break;
    case ZMENUBAR: {
        if(!IsMenu) break;
        prc->top=rc.top+mt.icon_margin*2+mt.icon_pixls;
        prc->bottom=prc->top+mt.menubar_cy;
        prc->left=rc.left;
        prc->right=rc.right;
        assign_flag=1;
    } break;
    case ZCAPTION: {
        if(!IsCaption) break;
        prc->top=rc.top+mt.border_pixls;
        prc->bottom=rc.top+mt.icon_margin*2+mt.icon_pixls;
        prc->left=rc.left+mt.border_pixls;
        prc->right=rc.right-mt.border_pixls;
        assign_flag=1;
    } break;
    case ZTEXTTITLE: {
        if(!IsCaption) break;
        prc->left=rc.left+mt.icon_margin*2+mt.icon_pixls;
        prc->right=rc.right-mt.ncbtn_margin*2-(IsCloseButton?mt.ncbtn_cx:0)-(IsMaxButton?mt.ncbtn_cx:0)-(IsMinButton?mt.ncbtn_cx:0);
        prc->top=rc.top;
        prc->bottom=rc.top+mt.icon_margin*2+mt.icon_pixls;
        assign_flag=1;
    } break;
    case ZCONFIG: {
        if(!IsCaption) break;
        prc->top=rc.top+mt.icon_margin;
        prc->bottom=prc->top+mt.ncbtn_cy;
        prc->right=rc.right-mt.ncbtn_margin*2-(IsCloseButton?mt.ncbtn_cx:0)-(IsMaxButton?mt.ncbtn_cx:0)-(IsMinButton?mt.ncbtn_cx:0);
        prc->left=prc->right-mt.ncbtn_cx;
        assign_flag=1;
    } break;
    case ZMSG: {
        if(!IsCaption) break;
        prc->top=rc.top+mt.icon_margin;
        prc->bottom=prc->top+mt.ncbtn_cy;
        prc->right=rc.right-mt.ncbtn_margin*3-mt.ncbtn_cx*4;
        prc->left=prc->right-mt.ncbtn_cx;
        assign_flag=1;
    } break;
    case ZHSCROLL: {
        SCROLLINFO si={0};
        
        if(!IsHScroll) break; 
        
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        
        prc->top=rc.top+mt.icon_margin*2+mt.icon_pixls+mt.menubar_cy;
        prc->bottom=rc.bottom-mt.border_pixls;
        prc->left=rc.right-mt.border_pixls-mt.scroll_pixls;
        prc->right=rc.right-mt.border_pixls;
        
        if(prc->bottom-prc->top>=si.nMax-si.nMin) break;
            
        assign_flag=1;
    } break;
    case ZHSTHUMB: {
        RECT rc_hs;
        SCROLLINFO si={0};
        int thumb_pixls,thumb_pos;
        
        if(Frame_GetNCZoneRect(hwnd,ZHSCROLL,&rc_hs,FALSE)!=1) break;
            
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        
        //获取滚动条pos信息
        thumb_pixls=si.nPage*1.0/(si.nMax-si.nMin+1)*(rc_hs.bottom-rc_hs.top);
        thumb_pixls=(thumb_pixls<mt.min_thumb_pixls)?mt.min_thumb_pixls:thumb_pixls;
        thumb_pos=si.nPos*1.0/(si.nMax-si.nMin+1-si.nPage)*(rc_hs.bottom-rc_hs.top-thumb_pixls);
        
        prc->left=rc_hs.left;prc->top=rc_hs.top+thumb_pos;
        prc->right=rc_hs.right;prc->bottom=prc->top+thumb_pixls;
        
        assign_flag=1;
    } break;
    }
    
    if(assign_flag==1&&allign_topleft) {
        OffsetRect(prc,-rc.left,-rc.top);
    }
    return assign_flag;
}

int Frame_NCCalcSize(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    RECT rect_new;
    RECT rect_old;
    RECT client_rect_new;
    RECT client_rect_old;
    RECT rc_hs;
    RMetrics mt={0};
    BOOL HasMenu=(GetMenu(hwnd)!=NULL);//失效原因应该与WM_NCCALCSIZE的触发时机有关.在创建时触发
    BOOL HasHScroll=((GetWindowLongPtr(hwnd,GWL_STYLE)&WS_HSCROLL)==WS_HSCROLL);
    Frame_GetMetrics(hwnd,&mt);
    
    if(wParam == TRUE) {
        LPNCCALCSIZE_PARAMS calc_param = (LPNCCALCSIZE_PARAMS)lParam;
        
        CopyRect(&rect_new,&(calc_param->rgrc[0]));
        CopyRect(&rect_old,&(calc_param->rgrc[1]));
        CopyRect(&client_rect_old,&(calc_param->rgrc[2]));
        
        //如果窗口够显示
        if(HasHScroll) {
            SCROLLINFO si={0};
            si.cbSize=sizeof(si);
            si.fMask=SIF_ALL;
            GetScrollInfo(hwnd,SB_HORZ,&si);
            
            if(Frame_GetNCZoneRect(hwnd,ZHSCROLL,&rc_hs,TRUE)!=1) HasHScroll=FALSE;
            else {
                if(rc_hs.bottom-rc_hs.top>=si.nMax-si.nMin) HasHScroll=FALSE;
            }
        }    
        client_rect_new = {rect_new.left+mt.border_pixls,
                           rect_new.top+mt.icon_margin*2+mt.icon_pixls+(HasMenu?mt.menubar_cy:0),
                           rect_new.right-mt.border_pixls-(HasHScroll?mt.scroll_pixls:0),
                           (rect_new.bottom-mt.border_pixls)};
        CopyRect(&(calc_param->rgrc[0]),&client_rect_new);
        CopyRect(&(calc_param->rgrc[1]),&rect_new);
        CopyRect(&(calc_param->rgrc[2]),&rect_old);
        
        return WVR_VALIDRECTS;
    }
    else {
        RECT* prect = (RECT*)lParam;
        
        if(HasHScroll) {
            SCROLLINFO si={0};
            si.cbSize=sizeof(si);
            si.fMask=SIF_ALL;
            GetScrollInfo(hwnd,SB_HORZ,&si);
            
            if(Frame_GetNCZoneRect(hwnd,ZHSCROLL,&rc_hs,TRUE)!=1) HasHScroll=FALSE;
            else {
                if(rc_hs.bottom-rc_hs.top>=si.nMax-si.nMin) HasHScroll=FALSE;
            }
        }
        prect->left+=mt.border_pixls; 
        prect->right-=mt.border_pixls+(HasHScroll?mt.scroll_pixls:0); 
        prect->top+=mt.icon_margin*2+mt.icon_pixls+(HasMenu?mt.menubar_cy:0); 
        prect->bottom-=mt.border_pixls;
        return 0;
    }
}

int Frame_InitialMenuPopup(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    HMENU menu=(HMENU)wParam;
    int pos=(UINT)LOWORD(lParam);
    BOOL sysmenu_flag=(BOOL)HIWORD(lParam);
    int item_count;
    MENUINFO mif={0};
    MENUITEMINFO miif={0};
    
    pFrameStyle fs=Frame_GetSettings(hwnd);
    if(!fs) return 0;
    
    item_count=GetMenuItemCount(menu);
    for(int index=0;index<item_count;index++) {
        miif.cbSize=sizeof(miif);
        miif.fMask=MIIM_STATE|MIIM_ID;
        GetMenuItemInfo(menu,index,TRUE,&miif);
        
        miif.cbSize=sizeof(miif);
        miif.fMask=MIIM_ID|MIIM_FTYPE;
        miif.fType=MFT_OWNERDRAW;//Menu Item Owner drawing
        SetMenuItemInfo(menu,index,TRUE,&miif);
    }
    //menu item backgnd color settings.
    mif.cbSize=sizeof(MENUINFO);
    mif.fMask=MIM_BACKGROUND;
    mif.hbrBack=fs->brush_mibk;
    SetMenuInfo(menu,&mif);
    return 0;
}

int Frame_TrackNCMouseLeave(HWND hwnd)
{
    TRACKMOUSEEVENT tme;
    tme.cbSize=sizeof(TRACKMOUSEEVENT);
    tme.dwFlags=TME_NONCLIENT|TME_LEAVE;
    tme.hwndTrack=hwnd;
    tme.dwHoverTime=HOVER_DEFAULT;
    TrackMouseEvent(&tme);
    return 0;
}

int Frame_StopNCMouseTrack(HWND hwnd)
{
    TRACKMOUSEEVENT tme;
    tme.cbSize=sizeof(TRACKMOUSEEVENT);
    tme.dwFlags=TME_CANCEL;
    tme.hwndTrack=hwnd;
    tme.dwHoverTime=HOVER_DEFAULT;
    TrackMouseEvent(&tme);
    
    return 0;
}

int Frame_NCHitTest(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    RECT rc,rc_cap,rc_bar;
    RECT rc_min,rc_max,rc_x,rc_ico,rc_hs;
    char text[512]="";
    BOOL IsHScroll;
    POINT hit={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    LRESULT result=DefWindowProc(hwnd,WM_NCHITTEST,wParam,lParam);
    int rt_cap,rt_bar,rt_x,rt_max,rt_min,rt_icon,rt_hs;
    
    rt_cap=Frame_GetNCZoneRect(hwnd,ZCAPTION,&rc_cap,FALSE);
    rt_bar=Frame_GetNCZoneRect(hwnd,ZMENUBAR,&rc_bar,FALSE);
    rt_x=Frame_GetNCZoneRect(hwnd,ZCLOSE,&rc_x,FALSE);
    rt_max=Frame_GetNCZoneRect(hwnd,ZMAX,&rc_max,FALSE);
    rt_min=Frame_GetNCZoneRect(hwnd,ZMIN,&rc_min,FALSE);
    rt_icon=Frame_GetNCZoneRect(hwnd,ZICON,&rc_ico,FALSE);
    IsHScroll=Frame_GetNCZoneRect(hwnd,ZHSCROLL,&rc_hs,FALSE)>0;
    
    if(PtInRect(&rc_min,hit)&&rt_min>0) result=HTMINBUTTON;
    else if(PtInRect(&rc_max,hit)&&rt_max>0) result=HTMAXBUTTON;
    else if(PtInRect(&rc_x,hit)&&rt_x>0) result=HTCLOSE;
    else if(PtInRect(&rc_ico,hit)&&rt_icon>0) result=HTSYSMENU;
    else if(PtInRect(&rc_bar,hit)&&rt_bar>0) return HTMENU;
    else if(PtInRect(&rc_cap,hit)&&rt_cap>0) return HTCAPTION;
    else if(PtInRect(&rc_hs,hit)&&IsHScroll) result=HTHSCROLL;
    
    return result;
}


int Frame_SetViewHeight(HWND hwnd,int height)
{   
    SCROLLINFO si={0}; 
    pFrameStyle fs=Frame_GetSettings(hwnd);
    if(!fs) return -1;
    
    if((WS_HSCROLL&GetWindowLongPtr(hwnd,GWL_STYLE))!=WS_HSCROLL) return 0;
    
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_HORZ,&si);
    
    si.fMask=SIF_RANGE;
    si.nMax=si.nMin+height;
    SetScrollInfo(hwnd,SB_HORZ,&si,FALSE);
    //需要触发WM_NCCALCSIZE
    SetWindowPos(hwnd,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
    Frame_ScrollBarDraw(hwnd);
    return 0;
}

int Frame_InitialViewSI(HWND hwnd)
{
    RECT rc;
    SCROLLINFO si={0};
    pFrameStyle fs=Frame_GetSettings(hwnd);
    if(!fs) return -1;
        
    fs->pvsi=(pViewSI)calloc(sizeof(RViewSI),1);
    if(!fs->pvsi) return -1;
    
    GetClientRect(hwnd,&rc);
    si.cbSize=sizeof(si);
    si.fMask=SIF_RANGE|SIF_POS|SIF_PAGE;
    si.nMin=0;
    si.nPos=0;
    if((GetWindowLongPtr(hwnd,GWL_STYLE)&WS_HSCROLL)==WS_HSCROLL) {
        si.nPage=si.nMax=(rc.bottom-rc.top);
        SetScrollInfo(hwnd,SB_HORZ,&si,FALSE);
    }
    
    if((GetWindowLongPtr(hwnd,GWL_STYLE)&WS_VSCROLL)==WS_VSCROLL) {
        si.nPage=si.nMax=(rc.right-rc.left);
        SetScrollInfo(hwnd,SB_VERT,&si,FALSE);
    }
    
    return 0;
}

pViewSI Frame_GetViewSI(HWND hwnd)
{
    pFrameStyle fs=Frame_GetSettings(hwnd);
    if(!fs) return NULL;
        
    return fs->pvsi;
}

int Frame_ClearViewSI(HWND hwnd) {
    pFrameStyle fs=Frame_GetSettings(hwnd);
    if(!fs) return NULL;
    
    if(fs->pvsi) free(fs->pvsi);
    fs->pvsi=NULL;
    
    return 0;
}

int Frame_ViewHBeginDrag(HWND hwnd,POINT pt)
{   
    pViewSI pvsi=NULL;
    RECT rc_hs;
    
    pvsi=Frame_GetViewSI(hwnd);
    if(!pvsi) return -1;
    
    if(Frame_GetNCZoneRect(hwnd,ZHSTHUMB,&rc_hs,FALSE)!=1) return -1;
    
    if(PtInRect(&rc_hs,pt)) {
        //跟踪WM_MOUSEMOVE
        SetCapture(hwnd);
        
        //初始化期初滚动条nPos
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        pvsi->drag_pos.y=si.nPos;
        
        //初始化拖拽期初坐标(转化为客户区坐标)
        ScreenToClient(hwnd,&pt);
        pvsi->drag_pt.y=pt.y;
        
        //Frame_LogMsg(hmsg,"进入HSCROLLBAR 拖拽");
    }
    return 0; 
}

int Frame_ViewHDraging(HWND hwnd,POINT pt)
{
    RECT rc_hs;//滚动条Rect
    RECT rc_thumb;//滚动条拇指选区Rect
    SCROLLINFO si={0};
    int offset;//滚动量
    int pre_pos;//滚动前的nPos
    pViewSI pvsi=NULL;
    
    pvsi=Frame_GetViewSI(hwnd);
    if(!pvsi) return -1;
        
    if(Frame_GetNCZoneRect(hwnd,ZHSTHUMB,&rc_thumb,TRUE)!=1) return -1;
    if(Frame_GetNCZoneRect(hwnd,ZHSCROLL,&rc_hs,TRUE)!=1) return -1;
    
    //记录当前的SCROLLINFO::nPos
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_HORZ,&si);
    pre_pos=si.nPos;
                    
    //获取新的SCROLLINFO::nPos
    //pt_h.y:开始拖拽的期初坐标
    si.nPos=pvsi->drag_pos.y+(si.nMax-si.nMin+1-si.nPage)*1.0*(pt.y-pvsi->drag_pt.y)/((rc_hs.bottom-rc_hs.top)-(rc_thumb.bottom-rc_thumb.top));
    if(si.nPos==pvsi->drag_pos.y) return -1;
    if(si.nPos<si.nMin) si.nPos=si.nMin;
    else if(si.nPos>si.nMax-si.nPage+1) si.nPos=si.nMax-si.nPage+1;
    
    //更新SCROLLINFO::nPos
    si.fMask=SIF_POS;
    SetScrollInfo(hwnd,SB_HORZ,&si,TRUE);
    
    //滚动视图区域
    offset=-(si.nPos-pre_pos);
    ScrollWindow(hwnd,0,offset,NULL,NULL);
    pvsi->view_offset.y+=offset;
    
    //Frame_LogMsg(hmsg,"HSCROLL 拖拽 Pos:%d...",si.nPos);
    //重绘滚动条
    Frame_ScrollBarDraw(hwnd);
    return 0;
}

int Frame_ViewHStopDrag(HWND hwnd)
{
    pViewSI pvsi=NULL;
    
    pvsi=Frame_GetViewSI(hwnd);
    if(!pvsi) return -1;
    
    ReleaseCapture();
    Frame_ScrollBarDraw(hwnd);
    
    //Frame_LogMsg(hmsg,"HSCROLL 拖拽结束");
    
    return 0;
}

int Frame_ViewHSize(HWND hwnd,int h_offset)
{
    SCROLLINFO si={0};
    int pre_pos;//滚动前的nPos
    int pre_page;//滚动前的nPage
    int offset=0;//垂直滚动量
    pViewSI pvsi=NULL;
    
    if(h_offset<=0) return -1;
        
    pvsi=Frame_GetViewSI(hwnd);
    if(!pvsi) return -1;
    
    //获取当前滚动参数
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_HORZ,&si);
    pre_pos=si.nPos;
    pre_page=si.nPage;
    
    //更新尺寸调整后的滚动参数(page,pos)
    si.fMask=SIF_PAGE|SIF_POS;
    
    //1.窗口扩大，介于nMin~nMax-nPage，仅调整nPos/nPage,无需滚卷。
    //nPage扩大，nPos不变
    si.nPage=h_offset;
    si.nPos=pre_pos;//不变
    
    //2.窗口尺寸已超过视图尺寸。
    //垂直滚动nPos将等于nMin.并将视图滚转至nMin位
    if(si.nPage>si.nMax-si.nMin)  {
        if(pre_pos!=si.nMin) {
            si.nPage=si.nMax-si.nMin;//无需滚动条状态
            offset=-pvsi->view_offset.y;
            ScrollWindow(hwnd,0,offset,NULL,NULL);
            si.nPos=si.nMin;
            
            pvsi->view_offset.y+=offset;
        }
        //Frame_LogMsg(hmsg,"HSCROLLBAR-view已完全展开,视图位(nPos):%d，滚转量:%d,滚动偏移:%d",si.nPos,offset,pvsi->view_offset.y);
    }
    //3.窗口尺nPos加上新的Page尺寸就超过视图大小，需滚卷至视图底端。
    //垂直滚动nPos将等于nMax-nPage,滚动两即为Page差值。
    else if(si.nPos+si.nPage>si.nMax-si.nMin) {
        //新Page未导致View完全展开
        si.nPos=si.nMax-si.nPage;
        
        //新Page导致View完全展开
        if(si.nPos<si.nMin) {
            si.nPos=si.nMin;//无需滚动条状态
            si.nPage=si.nMax-si.nMin;
            offset=-pvsi->view_offset.y;
        }
        else {
            offset=si.nPage-pre_page;
        }
        
        ScrollWindow(hwnd,0,offset,NULL,NULL);   
        pvsi->view_offset.y+=offset;
        
        //Frame_LogMsg(hmsg,"HSCROLLBAR-view已滚动至底部,视图位重置为nMax-nPage(nPos):%d，滚转量(Page差值):%d,滚动偏移:%d",si.nPos,offset,pvsi->view_offset.y);
    }
    
    SetScrollInfo(hwnd,SB_HORZ,&si,FALSE);
    return 0;
}

int Frame_SetHintWindow(HWND frame,HWND hint)
{
    pFrameStyle fs=Frame_GetSettings(frame);
    if(!fs) return NULL;
        
    fs->hint=hint;
    return 0;
}

int Frame_AllignHintWindow(HWND frame)
{
    RECT rc,rc_hint;
    POINT ct;
    pFrameStyle fs=Frame_GetSettings(frame);
    if(!fs) return -1;
            
    if(!fs->hint) return -1;
    
    GetWindowRect(frame,&rc);
    GetWindowRect(fs->hint,&rc_hint);
    ct.x=((rc.left+rc.right)>>1)-((rc_hint.right-rc_hint.left)>>1);
    ct.y=((rc.top+rc.bottom)>>1)-((rc_hint.bottom-rc_hint.top)>>1);
    SetWindowPos(fs->hint,NULL,ct.x,ct.y,(rc_hint.right-rc_hint.left),rc_hint.bottom-rc_hint.top,SWP_FRAMECHANGED|SWP_HIDEWINDOW|SWP_NOZORDER);
    
    return 0;
}

pHintInfo Frame_GetHintSettings(HWND frame)
{
    pFrameStyle fs=Frame_GetSettings(frame);
    if(!fs) return NULL;
        
    return Hint_GetSettings(fs->hint);
}

HWND Frame_GetFrameFromCursor(HWND drag_hwnd)
{
    POINT pt;
    RECT rc;
    HWND hwnd_bk=NULL;
    pFrameStyle fs=NULL;
    pHintInfo phint=NULL;
    
    SetWindowLongPtr(drag_hwnd,GWL_STYLE,GetWindowLongPtr(drag_hwnd,GWL_STYLE)&(~WS_VISIBLE));
    GetCursorPos(&pt);
    hwnd_bk=WindowFromPoint(pt);
    SetWindowLongPtr(drag_hwnd,GWL_STYLE,GetWindowLongPtr(drag_hwnd,GWL_STYLE)|WS_VISIBLE);//坐标穿过当前窗口
    
    if(hwnd_bk==NULL) return NULL;
    DWORD dragID,hwndbkID;
    GetWindowThreadProcessId(drag_hwnd,&dragID);
    GetWindowThreadProcessId(hwnd_bk,&hwndbkID);
    //if(GetWindowLongPtr(drag_hwnd,GWLP_HINSTANCE)!=GetWindowLongPtr(hwnd_bk,GWLP_HINSTANCE)) {
    if(dragID!=hwndbkID) {
        //Frame_LogMsg(hmsg,"非同一个Frame.");
        return NULL;
    }
    
    while(hwnd_bk) {
        fs=Frame_GetSettings(hwnd_bk);  
        if(!fs) {
            //Frame_LogMsg(hmsg,"Frame参数为空,尝试父级窗口...");
            hwnd_bk=GetParent(hwnd_bk);
            continue;
        } 
        phint=Frame_GetHintSettings(hwnd_bk);
        if(!phint) {
            //Frame_LogMsg(hmsg,"Hint参数为空,尝试父级窗口....");
            hwnd_bk=GetParent(hwnd_bk);
            continue;
        }
        return hwnd_bk;
    }
    
    return NULL;
}

pHintInfo Hint_InitialSettings(EDockStyle style)
{
    pHintInfo phint=(pHintInfo)calloc(sizeof(RHintInfo),1);
    if(!phint) return NULL;
    
    phint->style=style;    
    return phint;
}

HWND Hint_Create(HINSTANCE instance,EDockStyle style)
{
    pHintInfo phint=NULL;
    HWND lbl=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_LAYERED,
             		   WC_STATIC,"",
             		   NULL,
             		   0,0,150,150,
             		   NULL, (HMENU)NULL,
             		   instance, NULL);
    SetLayeredWindowAttributes(lbl,NULL,200,LWA_ALPHA); 
    SetWindowLongPtr(lbl,GWL_STYLE,(~WS_CAPTION)&GetWindowLongPtr(lbl,GWL_STYLE));
    
    if(NULL==(phint=Hint_InitialSettings(style))) {
        MessageBox(lbl,"初始化HintInfo失败","异常",MB_OK|MB_ICONINFORMATION);
        return NULL;
    }
    phint->proc=HintWindowProc;
    phint->pre_proc=(WNDPROC)SetWindowLongPtr(lbl,GWLP_WNDPROC,(LONG_PTR)phint->proc);
    SetWindowLongPtr(lbl,GWLP_USERDATA,(LONG_PTR)phint);
    
    switch(style) {
    case DOCK_ALDNXB: {
        phint->getarray=Hint_GetPtArray_DNXB;
        phint->hittest=Hint_HitTest_DNXB;
        phint->getpoly=Hint_GetPosPoly_DNXB;
        phint->hintdock=Hint_Dock_DNXB;
    } break;
    case DOCK_ALLRTRB: {
        phint->getarray=Hint_GetPtArray_LRTRB;
        phint->hittest=Hint_HitTest_LRTRB;
        phint->getpoly=Hint_GetPosPoly_LRTRB;
        phint->hintdock=Hint_Dock_LRTRB;
        phint->docksize=Hint_DockSize_LRTRB;
    } break; 
    case DOCK_ALTLTRTTRBB: {
        phint->getarray=Hint_GetPtArray_TLTRTTRBB;
        phint->hittest=Hint_HitTest_TLTRTTRBB;
        phint->getpoly=Hint_GetPosPoly_TLTRTTRBB;
        phint->hintdock=Hint_Dock_TLTRTTRBB;
        phint->docksize=Hint_DockSize_TLTRTTRBB;
    } break; 
    }
    POINT pt[32];
    int psize[5];
    int pcc=phint->getarray(lbl,pt,psize);
    
    if(pcc>0) {
        phint->dockarray=(HWND*)calloc(sizeof(HWND),pcc);
        HRGN rgn=CreatePolyPolygonRgn(pt,psize,pcc,WINDING);
        SetWindowRgn(lbl,rgn,TRUE);
        DeleteObject(rgn);
    }
    return lbl;
}

/*
返回值为封闭选区的数量
*/
int Hint_GetPtArray_DNXB(HWND hint,LPPOINT pt,int* pcc)
{
    RECT rc;
    POINT ct;

    pcc[0]=4;
    pcc[1]=pcc[2]=pcc[3]=pcc[4]=7;
    GetWindowRect(hint,&rc);
    ct.x=(rc.left+rc.right)>>1;
    ct.y=(rc.top+rc.bottom)>>1;
    pt[0].x=ct.x-15;
    pt[0].y=ct.y-15;
    pt[1].x=ct.x+15;
    pt[1].y=ct.y-15;
#define POINTSWITCH(p,c,t) (t)->x=(c)->x*2-(p)->x;(t)->y=(c)->y*2-(p)->y    
    POINTSWITCH(&pt[0],&ct,&pt[2]);
    POINTSWITCH(&pt[1],&ct,&pt[3]);
    
    //4,5,6,7,8,9,10
    pt[4].x=ct.x;
    pt[4].y=ct.y-50;
    pt[5].x=pt[1].x;
    pt[5].y=pt[4].y+20;
    pt[6].x=pt[1].x-10;
    pt[6].y=pt[5].y;
    pt[7].x=pt[6].x;
    pt[7].y=pt[1].y-10;
    pt[8].x=pt[0].x+10;
    pt[8].y=pt[0].y-10;
    pt[9].x=pt[0].x+10;
    pt[9].y=pt[6].y;
    pt[10].x=pt[0].x;
    pt[10].y=pt[9].y;
    
    POINTSWITCH(&pt[4],&ct,&pt[11]);
    POINTSWITCH(&pt[5],&ct,&pt[12]);
    POINTSWITCH(&pt[6],&ct,&pt[13]);
    POINTSWITCH(&pt[7],&ct,&pt[14]);
    POINTSWITCH(&pt[8],&ct,&pt[15]);
    POINTSWITCH(&pt[9],&ct,&pt[16]);
    POINTSWITCH(&pt[10],&ct,&pt[17]);
    
    pt[18].x=ct.x+50;
    pt[18].y=ct.y;
    pt[19].x=pt[18].x-20;
    pt[19].y=pt[2].y;
    pt[20].x=pt[19].x;
    pt[20].y=pt[2].y-10;
    pt[21].x=pt[2].x+10;
    pt[21].y=pt[20].y;
    pt[22].x=pt[21].x;
    pt[22].y=pt[1].y+10;
    pt[23].x=pt[19].x;
    pt[23].y=pt[22].y;
    pt[24].x=pt[23].x;
    pt[24].y=pt[1].y;
    POINTSWITCH(&pt[18],&ct,&pt[25]);
    POINTSWITCH(&pt[19],&ct,&pt[26]);
    POINTSWITCH(&pt[20],&ct,&pt[27]);
    POINTSWITCH(&pt[21],&ct,&pt[28]);
    POINTSWITCH(&pt[22],&ct,&pt[29]);
    POINTSWITCH(&pt[23],&ct,&pt[30]);
    POINTSWITCH(&pt[24],&ct,&pt[31]);
    
    return 5;
}

EDockerPos Hint_HitTest_DNXB(HWND hint,POINT pt)
{
    pHintInfo phint=Hint_GetSettings(hint);
    if(!phint) return AL_NONE;
    
    POINT pl[32];
    int carray[5];
    int pcc=phint->getarray(hint,pl,carray);
    
    if(Hint_PtInPolygon(pl,4,pt)) return ALDNXB_Z;
    else if(Hint_PtInPolygon(&pl[4],7,pt)) return ALDNXB_B;
    else if(Hint_PtInPolygon(&pl[11],7,pt)) return ALDNXB_N;
    else if(Hint_PtInPolygon(&pl[18],7,pt)) return ALDNXB_D;
    else if(Hint_PtInPolygon(&pl[25],7,pt)) return ALDNXB_X;
    else return AL_NONE;
}

int Hint_GetPosPoly_DNXB(HWND hwnd,EDockerPos pos,LPPOINT plist)
{
    pHintInfo phint=Hint_GetSettings(hwnd);
    if(!phint) return 0;
    
    int plc=0;
    POINT pl[32];
    int carray[5];
    int pcc=phint->getarray(hwnd,pl,carray);
    switch(pos) {
    case ALDNXB_Z:
        plc=4;
        memcpy(plist,pl,sizeof(POINT)*plc); 
    break;
    case ALDNXB_B:
        plc=7;
        memcpy(plist,&pl[4],sizeof(POINT)*plc);
    break;
    case ALDNXB_N:
        plc=7;
        memcpy(plist,&pl[11],sizeof(POINT)*plc);
    break;
    case ALDNXB_D:
        plc=7;
        memcpy(plist,&pl[18],sizeof(POINT)*plc);
    break;
    case ALDNXB_X:
        plc=7;
        memcpy(plist,&pl[25],sizeof(POINT)*plc);
    break;
    default:
        plc=0;
    }
    
    return plc;
}

int Hint_Dock_DNXB(HWND holder,HWND drag_hwnd,HWND hint)
{
    RECT rc;
    UINT style=(UINT)GetWindowLongPtr(drag_hwnd,GWL_STYLE);
    BOOL IsChild=((style&WS_CHILD)==WS_CHILD);
    pHintInfo phint=Hint_GetSettings(hint);
    if(!phint) return -1;
    
    GetClientRect(holder,&rc);
    switch(phint->pos) {
    case ALDNXB_D: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        SetWindowPos(drag_hwnd,NULL,rc.right-300,0,300,rc.bottom-rc.top,SWP_NOZORDER|SWP_SHOWWINDOW);
    } break;
    case ALDNXB_N: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        SetWindowPos(drag_hwnd,NULL,0,rc.bottom-300,rc.right-rc.left,300,SWP_NOZORDER|SWP_SHOWWINDOW);
    } break;
    case ALDNXB_X: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        SetWindowPos(drag_hwnd,NULL,0,0,300,rc.bottom-rc.top,SWP_NOZORDER|SWP_SHOWWINDOW);
    } break;
    case ALDNXB_B: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        SetWindowPos(drag_hwnd,NULL,0,0,rc.right-rc.left,300,SWP_NOZORDER|SWP_SHOWWINDOW);
    } break;
    default: {
        RECT rc_drag;
        if(IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,(~WS_CHILD)&style);
            SetParent(drag_hwnd,NULL);
        }
        GetWindowRect(drag_hwnd,&rc_drag);
        SetWindowPos(drag_hwnd,NULL,rc_drag.left,rc_drag.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
    } break;
    }
    return 0;
}

BOOL Hint_PtInPolygon(LPPOINT pl,int nvert,POINT pt)
{
  int i,j;
  BOOL c=FALSE;
  for(i=0,j=nvert-1;i<nvert;j=i++) {
    if(((pl[i].y>pt.y)!=(pl[j].y>pt.y)) &&
       (pt.x<(pl[j].x-pl[i].x)*(pt.y-pl[i].y)*1.0/(pl[j].y-pl[i].y)+pl[i].x))
       c=!c;
  }
  return c;
}

LRESULT CALLBACK HintWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    pHintInfo phint=Hint_GetSettings(hwnd);
    if(phint==NULL) return 0;
        
	switch (msg)
	{
	case WM_PAINT: {
	    POINT pl[32];
	    int carray[5];
        int index=0;
        PAINTSTRUCT ps;
        BeginPaint(hwnd,&ps);
        HBRUSH brush=CreateSolidBrush(RGB(30,30,30));
        
        int poly_count=phint->getarray(hwnd,pl,carray);
        for(index=0;index<32;index++) ScreenToClient(hwnd,&pl[index]);
        SelectObject(ps.hdc,brush);
        SetPolyFillMode(ps.hdc,WINDING);
        PolyPolygon(ps.hdc,pl,carray,poly_count);
        DeleteObject(brush);
        
        int plc=0;
        if(0!=(plc=phint->getpoly(hwnd,phint->pos,pl))) {
            HBRUSH brush_checked=CreateSolidBrush(RGB(0,128,250));
            
            for(index=0;index<plc;index++) ScreenToClient(hwnd,&pl[index]);
            
            SelectObject(ps.hdc,brush_checked);
            SetPolyFillMode(ps.hdc,WINDING);
            Polygon(ps.hdc,pl,plc);
            
            DeleteObject(brush_checked);
        }
        
        EndPaint(hwnd,&ps);
        return 0;
	} break;
	case WM_DOCKINGONTHEWAY: {
	    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
	    phint->pos=phint->hittest(hwnd,pt);
	    InvalidateRect(hwnd,NULL,TRUE);
	} break;
	}
	
	return CallWindowProc(phint->pre_proc,hwnd,msg,wParam,lParam);
}

pHintInfo Hint_GetSettings(HWND hwnd)
{
    pHintInfo phint=(pHintInfo)GetWindowLongPtr(hwnd,GWLP_USERDATA);
    return phint;
}

int Hint_ClearSettings(HWND hwnd)
{
    pHintInfo phint=Hint_GetSettings(hwnd);
    
    if(phint) {
        if(phint->dockarray) free(phint->dockarray);
        free(phint);
    }
    DestroyWindow(hwnd);
    
    return 0;
}

int Hint_GetPtArray_LRTRB(HWND hint,LPPOINT pt,int* pcc)
{
    RECT rc;
    POINT ct;

    pcc[0]=pcc[1]=pcc[2]=4;
    GetWindowRect(hint,&rc);
    ct.x=(rc.left+rc.right)>>1;
    ct.y=(rc.top+rc.bottom)>>1;
    pt[0].x=ct.x-50;
    pt[0].y=ct.y-50;
    pt[1].x=ct.x-5;
    pt[1].y=ct.y-50;
    pt[2].x=ct.x-5;
    pt[2].y=ct.y+50;
    pt[3].x=ct.x-50;
    pt[3].y=ct.y+50;
    
    pt[4].x=ct.x+5;
    pt[4].y=ct.y-50;
    pt[5].x=ct.x+50;
    pt[5].y=ct.y-50;
    pt[6].x=ct.x+50;
    pt[6].y=ct.y-5;
    pt[7].x=ct.x+5;
    pt[7].y=ct.y-5;
    
    
    pt[8].x=ct.x+5;
    pt[8].y=ct.y+5;
    pt[9].x=ct.x+50;
    pt[9].y=ct.y+5;
    pt[10].x=ct.x+50;
    pt[10].y=ct.y+50;
    pt[11].x=ct.x+5;
    pt[11].y=ct.y+50;

    return 3;
}

EDockerPos Hint_HitTest_LRTRB(HWND hint,POINT pt)
{
    pHintInfo phint=Hint_GetSettings(hint);
    if(!phint) return AL_NONE;
    
    POINT pl[32];
    int carray[3];
    int pcc=phint->getarray(hint,pl,carray);
    
    if(Hint_PtInPolygon(pl,4,pt)) return ALLRTRB_L;
    else if(Hint_PtInPolygon(&pl[4],4,pt)) return ALLRTRB_RT;
    else if(Hint_PtInPolygon(&pl[8],4,pt)) return ALLRTRB_RB;
    else return AL_NONE;
}

int Hint_GetPosPoly_LRTRB(HWND hwnd,EDockerPos pos,LPPOINT plist)
{
    pHintInfo phint=Hint_GetSettings(hwnd);
    if(!phint) return 0;
    
    int plc=0;
    POINT pl[32];
    int carray[3];
    int pcc=phint->getarray(hwnd,pl,carray);
    switch(pos) {
    case ALLRTRB_L:
        plc=4;
        memcpy(plist,pl,sizeof(POINT)*plc); 
    break;
    case ALLRTRB_RT:
        plc=4;
        memcpy(plist,&pl[4],sizeof(POINT)*plc);
    break;
    case ALLRTRB_RB:
        plc=4;
        memcpy(plist,&pl[8],sizeof(POINT)*plc);
    break;
    default:
        plc=0;
    }
    
    return plc;
}

int Hint_Dock_LRTRB(HWND holder,HWND drag_hwnd,HWND hint)
{
    RECT rc;
    UINT style=(UINT)GetWindowLongPtr(drag_hwnd,GWL_STYLE);
    BOOL IsChild=((style&WS_CHILD)==WS_CHILD);
    pHintInfo phint=Hint_GetSettings(hint);
    if(!phint) return -1;
    
    GetClientRect(holder,&rc);
    switch(phint->pos) {
    case ALLRTRB_L: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        if(IsBitSet(phint->dockbits,1)&&phint->dockarray[0]!=drag_hwnd) break;
        phint->dockarray[0]=drag_hwnd;
        SetBit(phint->dockbits,1);
        SetWindowPos(drag_hwnd,NULL,0,0,300,rc.bottom-rc.top,SWP_NOZORDER|SWP_SHOWWINDOW/*|SWP_NOREDRAW*/);
    } break;
    case ALLRTRB_RT: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        if(IsBitSet(phint->dockbits,2)&&phint->dockarray[1]!=drag_hwnd) break;
        phint->dockarray[1]=drag_hwnd;
        SetBit(phint->dockbits,2);
        SetWindowPos(drag_hwnd,NULL,300,0,rc.right-rc.left-300,(rc.bottom-rc.top)-300,SWP_NOZORDER|SWP_SHOWWINDOW/*|SWP_NOREDRAW*/);
    } break;
    case ALLRTRB_RB: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        if(IsBitSet(phint->dockbits,3)&&phint->dockarray[2]!=drag_hwnd) break;
        phint->dockarray[2]=drag_hwnd;
        SetBit(phint->dockbits,3);
        SetWindowPos(drag_hwnd,NULL,300,rc.bottom-300,rc.right-rc.left-300,300,SWP_NOZORDER|SWP_SHOWWINDOW/*|SWP_NOREDRAW*/);
    } break;
    default: {
        RECT rc_drag;
        if(IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,(~WS_CHILD)&style);
            SetParent(drag_hwnd,NULL);
        }
        
        for(int index=0;index<3;index++) {
            if(IsBitSet(phint->dockbits,index+1)&&phint->dockarray[index]==drag_hwnd) {
                ClearBit(phint->dockbits,index+1);
                phint->dockarray[index]=NULL;
                break;
            }
        }
        
        GetWindowRect(drag_hwnd,&rc_drag);
        SetWindowPos(drag_hwnd,NULL,rc_drag.left,rc_drag.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED/*|SWP_NOREDRAW*/);
    } break;
    }
    return 0;
}

int Hint_DockSize_LRTRB(HWND frame)
{
    pFrameStyle fs=Frame_GetSettings(frame);
    if((!fs)||(!fs->hint)) return -1;
    EDockerPos pos;
    pHintInfo phint=Hint_GetSettings(fs->hint);
    if(!phint) return -1;
    
    pos=phint->pos;
    phint->pos=ALLRTRB_L;
    for(int index=0;index<3;index++,phint->pos=(EDockerPos)(phint->pos+1)) {
        if(IsBitSet(phint->dockbits,index+1)&&phint->dockarray[index]) {
            Hint_Dock_LRTRB(frame,phint->dockarray[index],fs->hint);
        }
    }
    phint->pos=pos;
    
    return 0;
}

int Hint_GetPtArray_TLTRTTRBB(HWND hint,LPPOINT pt,int* pcc) {
    RECT rc;
    POINT ct;

    pcc[0]=pcc[1]=pcc[2]=pcc[3]=4;
    GetWindowRect(hint,&rc);
    ct.x=(rc.left+rc.right)>>1;
    ct.y=(rc.top+rc.bottom)>>1;
    pt[0].x=ct.x-50;
    pt[0].y=ct.y-50;
    pt[1].x=ct.x-5;
    pt[1].y=ct.y-50;
    pt[2].x=ct.x-5;
    pt[2].y=ct.y+50;
    pt[3].x=ct.x-50;
    pt[3].y=ct.y+50;
    
    pt[4].x=ct.x+5;
    pt[4].y=ct.y-50;
    pt[5].x=ct.x+50;
    pt[5].y=ct.y-50;
    pt[6].x=ct.x+50;
    pt[6].y=ct.y-5;
    pt[7].x=ct.x+5;
    pt[7].y=ct.y-5;
    
    
    pt[8].x=ct.x+5;
    pt[8].y=ct.y+5;
    pt[9].x=ct.x+50;
    pt[9].y=ct.y+5;
    pt[10].x=ct.x+50;
    pt[10].y=ct.y+50;
    pt[11].x=ct.x+5;
    pt[11].y=ct.y+50;
    
    
    pt[12].x=ct.x-50;
    pt[12].y=ct.y+60;
    pt[13].x=ct.x+50;
    pt[13].y=ct.y+60;
    pt[14].x=ct.x+50;
    pt[14].y=ct.y+80;
    pt[15].x=ct.x-50;
    pt[15].y=ct.y+80;

    return 4;
}

EDockerPos Hint_HitTest_TLTRTTRBB(HWND hint,POINT pt) {
    pHintInfo phint=Hint_GetSettings(hint);
    if(!phint) return AL_NONE;
    
    POINT pl[32];
    int carray[4];
    int pcc=phint->getarray(hint,pl,carray);
    
    if(Hint_PtInPolygon(pl,4,pt)) return ALTLTRTTRBB_TL;
    else if(Hint_PtInPolygon(&pl[4],4,pt)) return ALTLTRTTRBB_TRT;
    else if(Hint_PtInPolygon(&pl[8],4,pt)) return ALTLTRTTRBB_TRB;
    else if(Hint_PtInPolygon(&pl[12],4,pt)) return ALTLTRTTRBB_B;
    else return AL_NONE;
}

int Hint_GetPosPoly_TLTRTTRBB(HWND hwnd,EDockerPos pos,LPPOINT plist) {
    pHintInfo phint=Hint_GetSettings(hwnd);
    if(!phint) return 0;
    
    int plc=0;
    POINT pl[32];
    int carray[4];
    int pcc=phint->getarray(hwnd,pl,carray);
    switch(pos) {
    case ALTLTRTTRBB_TL:
        plc=4;
        memcpy(plist,pl,sizeof(POINT)*plc); 
    break;
    case ALTLTRTTRBB_TRT:
        plc=4;
        memcpy(plist,&pl[4],sizeof(POINT)*plc);
    break;
    case ALTLTRTTRBB_TRB:
        plc=4;
        memcpy(plist,&pl[8],sizeof(POINT)*plc);
    break;
    case ALTLTRTTRBB_B:
        plc=4;
        memcpy(plist,&pl[12],sizeof(POINT)*plc);
    break;
    default:
        plc=0;
    }
    
    return plc;
}

int Hint_Dock_TLTRTTRBB(HWND holder,HWND drag_hwnd,HWND hint) {
    RECT rc;
    UINT style=(UINT)GetWindowLongPtr(drag_hwnd,GWL_STYLE);
    BOOL IsChild=((style&WS_CHILD)==WS_CHILD);
    pHintInfo phint=Hint_GetSettings(hint);
    if(!phint) return -1;
    
    GetClientRect(holder,&rc);
    switch(phint->pos) {
    case ALTLTRTTRBB_TL: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        if(IsBitSet(phint->dockbits,1)&&phint->dockarray[0]!=drag_hwnd) break;
        phint->dockarray[0]=drag_hwnd;
        SetBit(phint->dockbits,1);
        SetWindowPos(drag_hwnd,NULL,0,0,300,rc.bottom-rc.top-24,SWP_NOZORDER|SWP_SHOWWINDOW/*|SWP_NOREDRAW*/);
    } break;
    case ALTLTRTTRBB_TRT: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        if(IsBitSet(phint->dockbits,2)&&phint->dockarray[1]!=drag_hwnd) break;
        phint->dockarray[1]=drag_hwnd;
        SetBit(phint->dockbits,2);
        SetWindowPos(drag_hwnd,NULL,299,0,rc.right-rc.left-300+1,(rc.bottom-rc.top)-298,SWP_NOZORDER|SWP_SHOWWINDOW/*|SWP_NOREDRAW*/);
    } break;
    case ALTLTRTTRBB_TRB: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        if(IsBitSet(phint->dockbits,3)&&phint->dockarray[2]!=drag_hwnd) break;
        phint->dockarray[2]=drag_hwnd;
        SetBit(phint->dockbits,3);
        SetWindowPos(drag_hwnd,NULL,299,rc.bottom-299,rc.right-rc.left-300+1,300-24,SWP_NOZORDER|SWP_SHOWWINDOW/*|SWP_NOREDRAW*/);
    } break;
    case ALTLTRTTRBB_B: {
        if(!IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,WS_CHILD|style);
            SetParent(drag_hwnd,holder);
        }
        if(IsBitSet(phint->dockbits,4)&&phint->dockarray[3]!=drag_hwnd) break;
        phint->dockarray[3]=drag_hwnd;
        SetBit(phint->dockbits,4);
        SetWindowPos(drag_hwnd,NULL,0,rc.bottom-25,rc.right-rc.top,25,SWP_NOZORDER|SWP_SHOWWINDOW/*|SWP_NOREDRAW*/);
        //SetWindowPos(drag_hwnd,NULL,300,rc.bottom-300,rc.right-rc.left-300,300,SWP_NOZORDER|SWP_SHOWWINDOW/*|SWP_NOREDRAW*/);
    } break;
    default: {
        RECT rc_drag;
        if(IsChild) {
            SetWindowLongPtr(drag_hwnd,GWL_STYLE,(~WS_CHILD)&style);
            SetParent(drag_hwnd,NULL);
        }
        
        for(int index=0;index<4;index++) {
            if(IsBitSet(phint->dockbits,index+1)&&phint->dockarray[index]==drag_hwnd) {
                ClearBit(phint->dockbits,index+1);
                phint->dockarray[index]=NULL;
                break;
            }
        }
        
        GetWindowRect(drag_hwnd,&rc_drag);
        SetWindowPos(drag_hwnd,NULL,rc_drag.left,rc_drag.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED/*|SWP_NOREDRAW*/);
    } break;
    }
    return 0;
}

int Hint_DockSize_TLTRTTRBB(HWND frame) {
    pFrameStyle fs=Frame_GetSettings(frame);
    if((!fs)||(!fs->hint)) return -1;
    EDockerPos pos;
    pHintInfo phint=Hint_GetSettings(fs->hint);
    if(!phint) return -1;
    
    pos=phint->pos;
    phint->pos=ALTLTRTTRBB_TL;
    for(int index=0;index<4;index++,phint->pos=(EDockerPos)(phint->pos+1)) {
        if(IsBitSet(phint->dockbits,index+1)&&phint->dockarray[index]) {
            Hint_Dock_TLTRTTRBB(frame,phint->dockarray[index],fs->hint);
        }
    }
    phint->pos=pos;
    
    return 0;
}

int Frame_Register(HINSTANCE instance)
{
    WNDCLASSEX winclass;
    
	winclass.cbSize=sizeof(WNDCLASSEX);
	winclass.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW|CS_DROPSHADOW;
	winclass.lpfnWndProc=FrameProc;
	winclass.cbClsExtra=0;
	winclass.cbWndExtra=0;
	winclass.hInstance=instance;
	winclass.hIcon=LoadIcon(instance,"IDC_ICON_AFRM2");//LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor=LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground=(HBRUSH)CreateSolidBrush(RGB(10,10,10));//GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName=NULL;
	winclass.lpszClassName=WINDOW_CLASS_NAME;
	winclass.hIconSm=LoadIcon(instance,"IDC_ICON_AFRM");//LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&winclass)) {
	    return -1;
	}
	
	return 0;
}