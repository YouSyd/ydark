#include "../include/ListReport.h"

int Report_InitialSettings(HWND hwnd)
{
    pReportStyle ps=(pReportStyle)calloc(sizeof(ReportStyle),1);
    if(!ps) return -1;
    
    ps->margin_border=1;
    ps->color_border=RGB(53,53,53);
    
    ps->proc=ListReportOwnProc;
    ps->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ps->proc);
    
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)ps);    
    return 0;
}

pReportStyle Report_GetSettings(HWND hwnd)
{
    return (pReportStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int Report_ClearSettings(HWND hwnd)
{
    pReportStyle ps=Report_GetSettings(hwnd);
    
    if(!ps) return 0;
    
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ps->pre_proc);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,NULL);
    
    return 0;
}

int Report_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam) 
{
    pReportStyle rs=Report_GetSettings(hwnd);
    if(!rs) return -1;
    
    PAINTSTRUCT ps={0};
    BeginPaint(hwnd,&ps);
    HDC hdc=ps.hdc;
    
    HDC memdc=CreateCompatibleDC(hdc);
    RECT rcMem,rcPaint;
    CopyRect(&rcMem,&ps.rcPaint);
    CopyRect(&rcPaint,&ps.rcPaint);
    OffsetRect(&rcMem,-rcMem.left,-rcMem.top);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,rcMem.right,rcMem.bottom);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    SelectObject(memdc,(HFONT)SendMessage(hwnd,WM_GETFONT,0,0));
    SetBkMode(memdc,TRANSPARENT);
    SetTextColor(memdc,ListView_GetTextColor(hwnd));
    COLORREF color_bk=ListView_GetBkColor(hwnd);//RGB(15,15,15);
    COLORREF color_hover=RGB(30,30,30);
    COLORREF color_sel=RGB(5,5,5);
    HBRUSH brush=CreateSolidBrush(color_bk);
    HBRUSH brush_Sel=CreateSolidBrush(color_sel);
    FillRect(memdc,&rcMem,brush);
    
    POINT pt_org;
    OffsetViewportOrgEx(memdc,-ps.rcPaint.left,-ps.rcPaint.top,&pt_org);
    
    HIMAGELIST img=ListView_GetImageList(hwnd,LVSIL_NORMAL);
    int rep_count=ListView_GetItemCount(hwnd);
    LV_ITEM item={0};
    char text[256]="";
    int img_cx,img_cy;
    ImageList_GetIconSize(img,&img_cx,&img_cy);
    HBRUSH brush_border=CreateSolidBrush(RGB(53,53,53));
    
    //if(rcPaint.right<rcPaint.left+80) rcPaint.right=rcPaint.left+80;
    //if(rcPaint.bottom<rcPaint.top+80) rcPaint.bottom=rcPaint.top+80;      
    HBRUSH brush_fill=CreateSolidBrush(color_hover);
    for(int index=0;index<rep_count;index++) {
        RECT rcItem,rcLabel,rcIcon;
        ListView_GetItemRect(hwnd,index,&rcItem,LVIR_BOUNDS);
        ListView_GetItemRect(hwnd,index,&rcIcon,LVIR_ICON);
        ListView_GetItemRect(hwnd,index,&rcLabel,LVIR_LABEL);
        POINT pt[4]={{rcItem.left,rcItem.top},{rcItem.right,rcItem.top},{rcItem.left,rcItem.bottom},{rcItem.right,rcItem.bottom}};
        
        //if(PtInRect(&rcPaint,pt[0])||PtInRect(&rcPaint,pt[1])||
        //   PtInRect(&rcPaint,pt[2])||PtInRect(&rcPaint,pt[3])) {
            item.mask=LVIF_IMAGE|LVIF_TEXT|LVIF_STATE;
            item.iItem=index;
            item.iSubItem=0;
            item.pszText=text;
            item.stateMask=LVIS_SELECTED;
            item.cchTextMax=sizeof(text);
            ListView_GetItem(hwnd,&item);
            
            if((item.state&LVIS_SELECTED)==LVIS_SELECTED) {
                FillRect(memdc,&rcItem,brush_Sel);
                ImageList_SetBkColor(img,color_sel);
            }
            else {
                FillRect(memdc,&rcItem,(rs->index_hover==index)?brush_fill:brush);
                ImageList_SetBkColor(img,(rs->index_hover==index)?color_hover:color_bk);
            }
            
            ImageList_Draw(img,index,memdc,rcIcon.left+((rcIcon.right-rcIcon.left-img_cx)>>1),rcIcon.top+((rcIcon.bottom-rcIcon.top-img_cy)>>1),ILD_NORMAL);
            DrawText(memdc,text,-1,&rcLabel,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
            
            if((rs->index_hover==index)||((item.state&LVIS_SELECTED)==LVIS_SELECTED)) {
                FrameRect(memdc,&rcItem,brush_border);
                
                Graphics graphic(memdc);
                graphic.SetSmoothingMode(SmoothingModeHighQuality);
                Point tri_pt[3]={Point(rcItem.left+1,rcItem.top+15),Point(rcItem.left+1,rcItem.top+1),Point(rcItem.left+15,rcItem.top+1)};
                LinearGradientBrush pbrush(Rect(rcItem.left,rcItem.top,30,30),Color(255,200,0,0),Color(255,50,0,0),
                                           LinearGradientModeHorizontal);
                graphic.FillPolygon(&pbrush,tri_pt,3);
            }
        //}
    }
    
    SetViewportOrgEx(memdc,pt_org.x,pt_org.y,NULL);
    
    BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,rcMem.right,rcMem.bottom,memdc,0,0,SRCCOPY);
    
    DeleteObject(brush);
    DeleteObject(brush_Sel);
    DeleteObject(brush_border);
    DeleteObject(brush_fill);
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    
    EndPaint(hwnd,&ps);
    return 0;
}

int Report_MoveItems(HWND hwnd)
{
    RECT rect,item_rect;
    POINT pt={0};
    int item_count=0;
    int item_height,item_width;
    int repos_row=0;
    
    GetClientRect(hwnd,&rect);
    item_count=ListView_GetItemCount(hwnd);
    
    pt.x=rect.left+40;
    pt.y=rect.top+20;
    for(int index=0;index<item_count;index++) {
        if(index==0) {
            ListView_GetItemRect(hwnd,0,&item_rect,LVIR_BOUNDS);
            item_width=item_rect.right-item_rect.left;
            item_height=item_rect.bottom-item_rect.top;
        }
        
        if(pt.x+item_width/2>rect.right) {
            pt.x=rect.left+40;
            pt.y=pt.y+item_height+20;
        }
        ListView_SetItemPosition(hwnd,index,pt.x,pt.y);
        pt.x+=(20+item_width);
    }
    
    return 0;
}

LRESULT CALLBACK ListReportOwnProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    pReportStyle ps=(pReportStyle)Report_GetSettings(hwnd);
    if(!ps) return 0;
    
    switch(message){
    case WM_NCCALCSIZE: {
        RECT rect_new;
        RECT rect_old;
        RECT client_rect_new;
        RECT client_rect_old;
        UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
        BOOL HasBorder=((style&WS_BORDER)==WS_BORDER);
        
        if(wParam == TRUE) {
            LPNCCALCSIZE_PARAMS calc_param = (LPNCCALCSIZE_PARAMS)lParam;
            
            CopyRect(&rect_new,&(calc_param->rgrc[0]));
            CopyRect(&rect_old,&(calc_param->rgrc[1]));
            CopyRect(&client_rect_old,&(calc_param->rgrc[2]));
            
            client_rect_new = {rect_new.left+(HasBorder?ps->margin_border:0),
                               rect_new.top+(HasBorder?ps->margin_border:0),
                               rect_new.right-((HasBorder?ps->margin_border:0)),
                               rect_new.bottom-((HasBorder?ps->margin_border:0))};
            CopyRect(&(calc_param->rgrc[0]),&client_rect_new);
            CopyRect(&(calc_param->rgrc[1]),&rect_new);
            CopyRect(&(calc_param->rgrc[2]),&rect_old);
            
            return WVR_VALIDRECTS;
        }
        else {
            RECT* prect = (RECT*)lParam;
            prect->right-=(HasBorder?ps->margin_border:0); 
            prect->bottom-=(HasBorder?ps->margin_border:0);
            prect->left+=(HasBorder?ps->margin_border:0);
            prect->top+=(HasBorder?ps->margin_border:0);
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
        
        HPEN pen=CreatePen(PS_SOLID,1,ps->color_border);
        SelectObject(hdc,pen);
        
        for(int i=0;i<ps->margin_border;i++) {
            POINT pt[5]={{rc.left,rc.top},\
            {rc.right-1,rc.top},\
            {rc.right-1,rc.bottom-1},\
            {rc.top,rc.bottom-1},\
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
    case WM_NCHITTEST: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        RECT rc;
        BOOL HasBorder=((WS_BORDER&(UINT)GetWindowLongPtr(hwnd,GWL_STYLE))==WS_BORDER);
        GetWindowRect(hwnd,&rc);
        if(HasBorder&&(pt.x==rc.left||pt.x==rc.right||pt.y==rc.top||pt.y==rc.bottom)) return HTBORDER;
    } break;
    case WM_PAINT: {
        return Report_Paint(hwnd,wParam,lParam);
    } break;
    case WM_ERASEBKGND: {
        return 1;
    } break;
    case WM_MOUSEMOVE: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        LVHITTESTINFO hit={0};
        memcpy(&hit.pt,&pt,sizeof(POINT));
        RECT rcHover;
        if(ps->index_hover!=-1) {
            ListView_GetItemRect(hwnd,ps->index_hover,&rcHover,LVIR_BOUNDS);
            InvalidateRect(hwnd,&rcHover,FALSE);
        }
        
        int cur_hover=ListView_HitTest(hwnd,&hit);
        if(cur_hover!=-1) {
            ListView_GetItemRect(hwnd,cur_hover,&rcHover,LVIR_BOUNDS);
            InvalidateRect(hwnd,&rcHover,FALSE);
        }
        ps->index_hover=cur_hover;
    } break;
    case WM_SIZE: {
        Report_MoveItems(hwnd);
    } break;
    case LVM_HITTEST: {
       LPLVHITTESTINFO phit=(LPLVHITTESTINFO)lParam;
       
       //...
    } break;
    case WM_NCDESTROY: {
        WNDPROC pre_proc=ps->pre_proc;
        Report_ClearSettings(hwnd);
        return CallWindowProc(pre_proc,hwnd,message,wParam,lParam);
    } break;
    }
    
    return CallWindowProc(ps->pre_proc,hwnd,message,wParam,lParam);   
}
