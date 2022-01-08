#include "../include/statusbar.h"

int StatusBar_InitialSettings(HWND hwnd) {
    pStatusStyle ss=(pStatusStyle)calloc(sizeof(RStatusStyle),1);
    if(!ss) return -1;
        
    ss->proc=StatusBarProc;
    ss->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ss->proc);
    
    ss->bk_colr=RGB(25,25,25);
    ss->border_colr=RGB(50,50,50);
    ss->txt_colr=RGB(200,200,200);
    ss->cy=25;
    ss->off_flow=0;
    ss->active_index=-1;
    
    ss->font=CreateFont(15,0,0,0,
                    FW_LIGHT,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "Courier New");
    
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)ss);
    return 0;
}

pStatusStyle StatusBar_GetSettings(HWND hwnd) {
    return (pStatusStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int StatusBar_ClearSettings(HWND hwnd) {
    pStatusStyle ss=StatusBar_GetSettings(hwnd);
    if(!ss) return -1;
    
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ss->pre_proc);
    free(ss);
    
    return 0;
}

LRESULT CALLBACK StatusBarProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
    pStatusStyle ss=StatusBar_GetSettings(hwnd);
    if(!ss) return -1;
        
    switch(msg) {
    case WM_PAINT:
        StatusBar_Paint(hwnd,wParam,lParam);
        return 0;
    break;
    case WM_MOUSEMOVE: {
        ss->active_index=-1;
        ss->off_flow=0;
        
        TRACKMOUSEEVENT tme={0};
        tme.cbSize=sizeof(tme);
        tme.dwFlags=TME_HOVER|TME_LEAVE;
        tme.dwHoverTime=100;
        tme.hwndTrack=hwnd;
        
        TrackMouseEvent(&tme); 
    }   
    break;
    case WM_MOUSELEAVE:
        KillTimer(hwnd,TIMER_TRIGGERMSGFLOW);
        KillTimer(hwnd,TIMER_MSGFLOW);
        //if(ss->active_index!=-1) {
            RECT rcItem;
            SendMessage(hwnd,SB_GETRECT,ss->active_index,(LPARAM)&rcItem);
            ss->off_flow=0;
            ss->active_index=-1;
            InvalidateRect(hwnd,&rcItem,TRUE);
            UpdateWindow(hwnd);//强制直接更新
        //}
    break;
    case WM_MOUSEHOVER: {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd,&pt);
        int itemIdx=SendMessage(hwnd,WMYU_HITITEM,NULL,MAKELPARAM(pt.x,pt.y));
        if(itemIdx!=-1) {
            ss->active_index=itemIdx;
            SetTimer(hwnd,TIMER_TRIGGERMSGFLOW,50,NULL);
        } 
    } break;
    case WMYU_HITITEM: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        int coordinate[256]={0};
        SendMessage(hwnd,SB_GETPARTS,256,(LPARAM)coordinate);
        for(int index=0;index<256&&coordinate[index]!=-1;index++) {
            if(pt.x<coordinate[index]) {
                return index;
            }
        }
        return -1;
    } break;
    case WM_ERASEBKGND:
        return 1;
    break;
    case WM_TIMER:
        if(wParam==TIMER_TRIGGERMSGFLOW) {
            InvalidateRect(hwnd,NULL,FALSE);
            SetTimer(hwnd,TIMER_MSGFLOW,80,NULL);
            KillTimer(hwnd,wParam);
        }
        else if(wParam=TIMER_MSGFLOW) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hwnd,&pt);
            ss->off_flow+=5;
            int itemIdx=SendMessage(hwnd,WMYU_HITITEM,NULL,MAKELPARAM(pt.x,pt.y));
            if(itemIdx!=ss->active_index) {
                KillTimer(hwnd,TIMER_MSGFLOW);
                ss->active_index=-1;
                ss->off_flow=0;
                break;
            }
            RECT rcItem;
            SendMessage(hwnd,SB_GETRECT,itemIdx,(LPARAM)&rcItem);
            InvalidateRect(hwnd,&rcItem,FALSE);
        }
    break;
    case WM_NCDESTROY: 
        WNDPROC pre_proc=ss->pre_proc;
        StatusBar_ClearSettings(hwnd);
        return CallWindowProc(ss->pre_proc,hwnd,msg,wParam,lParam);
    break;
    }
    
    return CallWindowProc(ss->pre_proc,hwnd,msg,wParam,lParam);
}

int StatusBar_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam) {
    pStatusStyle ss=StatusBar_GetSettings(hwnd);
    if(!ss) return -1;
        
    int coordinate[256]={0};
    RECT rcItem,rc;
    int cx,cy;
    char txtItem[256]="";
    PAINTSTRUCT ps={0};
    
    BeginPaint(hwnd,&ps);
    GetWindowRect(hwnd,&rc);
    OffsetRect(&rc,-rc.left,-rc.top);
    cx=rc.right;cy=rc.bottom;
    
    HDC hdc=ps.hdc;
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy);
    HBITMAP preBmp=(HBITMAP)SelectObject(memdc,bmp);
    SelectObject(memdc,ss->font);
    HBRUSH bkbrush=CreateSolidBrush(ss->bk_colr);
    FillRect(memdc,&rc,bkbrush);
        
    HBRUSH borderbrush=CreateSolidBrush(ss->border_colr);
    FrameRect(memdc,&rc,borderbrush);
    
    SetTextColor(memdc,ss->txt_colr);
    SetBkMode(memdc,TRANSPARENT);
    SendMessage(hwnd,SB_GETPARTS,256,(LPARAM)coordinate);
    
    HPEN pen1=CreatePen(PS_SOLID,1,RGB(GetRValue(ss->border_colr)-15,GetGValue(ss->border_colr)-15,GetBValue(ss->border_colr)-15));
    HPEN pen2=CreatePen(PS_SOLID,1,RGB(GetRValue(ss->border_colr)+15,GetGValue(ss->border_colr)+15,GetBValue(ss->border_colr)+15));
    HPEN prepen;
    for(int index=0;index<256&&coordinate[index]!=-1;index++) {
        memset(txtItem,0x00,sizeof(txtItem));
        SendMessage(hwnd,SB_GETRECT,index,(LPARAM)&rcItem);
        SendMessage(hwnd,SB_GETTEXT,index,(LPARAM)&txtItem);
        
        if(!(ps.rcPaint.left<=rcItem.left&&rcItem.left<=ps.rcPaint.right ||
           ps.rcPaint.left<=rcItem.right&&rcItem.right<=ps.rcPaint.right)) continue;
        //paint.
        rcItem.top=rc.top;rcItem.bottom=rc.bottom;
        
        POINT pt[5]={{rcItem.right,rcItem.top},{rcItem.right,rcItem.bottom},
                     {pt[0].x+1,pt[0].y},{pt[1].x+1,pt[1].y}};
        prepen=(HPEN)SelectObject(memdc,pen1);
        MoveToEx(memdc,pt[0].x,pt[0].y,&pt[4]);
        LineTo(memdc,pt[1].x,pt[1].y);
        SelectObject(memdc,pen2);
        MoveToEx(memdc,pt[2].x,pt[2].y,&pt[4]);
        LineTo(memdc,pt[3].x,pt[3].y);
        
        if((ss->active_index!=-1)&&(ss->active_index==index)) {
            RECT rcCalc;
            DrawText(memdc,txtItem,-1,&rcCalc,DT_SINGLELINE|DT_VCENTER|DT_CALCRECT);
            rcCalc.top=rcItem.top;rcCalc.bottom=rcItem.bottom;
            if(rcCalc.right-rcCalc.left>rcItem.right-rcItem.left) {
                int offset_item_y=1;
                HDC itemDC=CreateCompatibleDC(hdc);
                HBITMAP itembmp=CreateCompatibleBitmap(hdc,rcCalc.right-rcCalc.left,cy);//-offset_item_y*2);
                HBITMAP preitemBmp=(HBITMAP)SelectObject(itemDC,itembmp);
                FillRect(itemDC,&rc,bkbrush);
                SetTextColor(itemDC,RGB(0,125,250));
                SetBkMode(itemDC,TRANSPARENT);
                SelectObject(itemDC,ss->font);
                OffsetRect(&rcCalc,-rcCalc.left,-rcCalc.top);
                DrawText(itemDC,txtItem,-1,&rcCalc,DT_SINGLELINE|DT_VCENTER|DT_WORD_ELLIPSIS);
                BitBlt(memdc,rcItem.left,rcItem.top+offset_item_y,rcItem.right-rcItem.left,cy-offset_item_y*2,
                       itemDC,ss->off_flow,offset_item_y,SRCCOPY);
                DeleteObject(SelectObject(itemDC,preitemBmp));
                DeleteDC(itemDC);
                
                if(ss->off_flow+rcItem.right-rcItem.left>=rcCalc.right-rcCalc.left) {
                    ss->off_flow=0;
                    ss->active_index=-1;
                    KillTimer(hwnd,TIMER_MSGFLOW);
                }
            }
            else {
                KillTimer(hwnd,TIMER_MSGFLOW);
                ss->active_index=-1;
                DrawText(memdc,txtItem,-1,&rcItem,DT_SINGLELINE|DT_VCENTER|DT_WORD_ELLIPSIS);
            }   
        }
        else {
            //很遗憾，DrawText的效率极低，以至于程序的显示都出现问题。
            //如果需要调用DrawText，应该想办法尽量减少调用次数
            //尤其是使用WM_TIMER之后，最好改造WM_PAINT，仅仅显示刷新的rc.
            DrawText(memdc,txtItem,-1,&rcItem,DT_SINGLELINE|DT_VCENTER|DT_WORD_ELLIPSIS);
        }
    }
    
    BitBlt(hdc,0,0,cx,cy,memdc,0,0,SRCCOPY);
    
    DeleteObject(pen1);
    DeleteObject(pen2);
    DeleteObject(bkbrush);
    DeleteObject(borderbrush);
    DeleteObject(SelectObject(memdc,preBmp));
    DeleteDC(memdc);
    EndPaint(hwnd,&ps);
    
    return 0;
}