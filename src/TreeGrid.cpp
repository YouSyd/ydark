#include "../include/TreeGrid.h"

/*
 有点问题.
 滚动条重绘有点问题，但是早先的版本是正常的，.lib文件重新生成的不行，不知道当时没问题的原始代码是什么样的.
 */

int Tree_InitialSettings(HWND hwnd)
{
    pTreeStyle ts=(pTreeStyle)calloc(sizeof(RTreeStyle),1);
    if(!ts) return -1;
    
    ts->proc=TreeOwnProc;
    ts->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ts->proc);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)ts);
    
    //ts->hdc=GetDC(hwnd);
    //ts->memdc=CreateCompatibleDC(ts->hdc);
    //ts->bmp=CreateCompatibleBitmap(ts->hdc,2500,1500);
    //SelectObject(ts->memdc,ts->bmp);
    
    return 0;
}

pTreeStyle Tree_GetSettings(HWND hwnd)
{
    return (pTreeStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int Tree_ClearSettings(HWND hwnd)
{
    pTreeStyle ts=Tree_GetSettings(hwnd);
    if(!ts) return -1;
    
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)ts->pre_proc);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,NULL);
    
    //DeleteObject(ts->bmp);
    //DeleteDC(ts->memdc);
    //ReleaseDC(hwnd,ts->hdc);
    
    free(ts);
    return 0; 
}

int Tree_Draw(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    LPNMTVCUSTOMDRAW cust_draw=(LPNMTVCUSTOMDRAW)lParam;
    RECT rect;
    HDC hdc;
    TVHITTESTINFO tv_hit;
    TVITEM tv_item;
    HTREEITEM item;
    char item_text[1024]="";
    int step_level=cust_draw->iLevel;
    
    hdc=cust_draw->nmcd.hdc;
    CopyRect(&rect,&(cust_draw->nmcd.rc));
    
    if(cust_draw->nmcd.dwDrawStage==CDDS_PREPAINT) {
        return CDRF_NOTIFYITEMDRAW;
    }
    else if(cust_draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
        item=(HTREEITEM)((LPNMCUSTOMDRAW)cust_draw)->dwItemSpec;// TreeView_HitTest(hwnd,&tv_hit);
        if(item) {
            tv_item.mask=TVIF_TEXT|TVIF_STATE|TVIF_CHILDREN;//获取文本/状态/是否有子节点
            tv_item.hItem=item;
            tv_item.pszText=item_text;
            tv_item.cchTextMax=sizeof(item_text);
            TreeView_GetItem(hwnd,&tv_item);
            Tree_DrawItem(hwnd,hdc,rect,step_level,&tv_item);
        } 
        return CDRF_SKIPDEFAULT;              
    }
    return CDRF_SKIPDEFAULT;
}

int Tree_DrawItem(HWND hwnd,HDC hdc,RECT rc,int step_level,LPTVITEM pti)
{
    HWND head=GetDlgItem(GetParent(hwnd),ID_GRIDHEADER_01);
    pHeadStyle hs=Head_GetSettings(head);
    pTreeStyle ts=Tree_GetSettings(hwnd);
    
    if(!hs) return -1;
    if(!ts) return -1;
       
    RECT rcTmp;
    CopyRect(&rcTmp,&rc);
    //此处逻辑通则程序不通，逻辑不通则程序通
    SCROLLINFO si={0};
    si.cbSize=sizeof(si);
    si.fMask=SIF_POS;
    GetScrollInfo(hwnd,SB_VERT,&si);
    int offx=-(hs->offset),offy=0;//si.nPos*TreeView_GetItemHeight(hwnd);
    
    int cx=rcTmp.right-rcTmp.left;
    int cy=rcTmp.bottom-rcTmp.top;
    
    RECT rcMem;
    CopyRect(&rcMem,&rc);
    OffsetRect(&rcMem,-rcMem.left,-rcMem.top);
    HDC memdc=CreateCompatibleDC(hdc);//ts->memdc;
    HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);//ts->bmp;
    
    COLORREF color_bk;
    if(step_level==0) color_bk=RGB(15,15,15);   
    else color_bk=RGB(19,19,19);
    HBRUSH brush=CreateSolidBrush(color_bk);
    FillRect(memdc,&rcMem,brush);
    DeleteObject(brush);
    
    //依赖Head的信息更新细节
    //两项位移偏差:水平滚动与垂直滚动，水平滚动与Head的offset一致；
    //垂直滚动依赖于垂直滚动条信息
    POINT pt_org;
    OffsetViewportOrgEx(memdc,-(rc.left+offx),-(rc.top+offy),&pt_org);
    
    int total_offset=0;
    int col_count=Header_GetItemCount(head);
    RECT rcItem;
    HDITEM item={0};
    if(pti->hItem==ts->hover_item) SetTextColor(memdc,RGB(100,160,220));
    else if(step_level==0) SetTextColor(memdc,RGB(0,128,250));
    else SetTextColor(memdc,RGB(200,200,200));
    SetBkMode(memdc,TRANSPARENT);
    SelectObject(memdc,(HFONT)SendMessage(hwnd,WM_GETFONT,0,0));
    
    //确定水平方向偏移
    for(int index=0;index<col_count;index++) {
        Header_GetItemRect(head,index,&rcItem);
        //转化为客户区坐标
        OffsetRect(&rcItem,-offx,0);
        
        if(rcItem.right<=rc.left) continue;        
        else if(rcItem.left<=rc.left&&rcItem.right>rc.left) {
            //需要部分绘制
            total_offset=rcItem.right-rc.left;
        }
        else if(rcItem.left>=rc.left&&total_offset<cx) {
            //需要整段绘制
            total_offset+=rcItem.right-rcItem.left;
        }
        else if(total_offset>=cx) break;
        
        //还原为Zero-Base坐标
        OffsetRect(&rcItem,offx,0);
        rcItem.top=rc.top+offy;
        rcItem.bottom=rc.bottom+offy;
        
        HBRUSH brush_border=CreateSolidBrush(RGB(50,50,50));
        RECT rcBorder;
        CopyRect(&rcBorder,&rcItem);
        if(step_level==0) {
            //根节点背景绘制
            rcBorder.left=0;rcBorder.top-=1;
            rcBorder.right=rcBorder.left+Head_GetTotalSize(head);
            FrameRect(memdc,&rcBorder,brush_border);
            
            if(index==0) {
                //根节点绘制特殊红点标记
                RECT rcTag;
                CopyRect(&rcTag,&rcItem);
                rcTag.right=rcTag.left+8;
                InflateRect(&rcTag,-1,-1);
                HBRUSH brush_tag=CreateSolidBrush(RGB(120,0,0));
                FillRect(memdc,&rcTag,brush_tag);
                DeleteObject(brush_tag);
            }
        }
        else {
            //非根节点背景绘制
            rcBorder.left-=1;rcBorder.top-=1;
            FrameRect(memdc,&rcBorder,brush_border);
        }
        
        //绘制有子节点的三角标记
        if(pti->cChildren&&index==0) {
            int height,width;
            height=width=rcItem.bottom-rcItem.top-10;
            Rect icon_rect(rcItem.left+width-7+30*step_level,rcItem.top+((rcItem.bottom-rcItem.top-height)>>1),width,height);
            Point icon_points[4]={Point(icon_rect.GetLeft(),icon_rect.GetTop()+(icon_rect.GetBottom()-icon_rect.GetTop())/3),
                                  Point(icon_rect.GetRight(),icon_rect.GetTop()+(icon_rect.GetBottom()-icon_rect.GetTop())/3),
                                  Point((icon_rect.GetLeft()+icon_rect.GetRight())/2,icon_rect.GetBottom()-(icon_rect.GetBottom()-icon_rect.GetTop())/3),
                                  Point(icon_rect.GetLeft(),icon_rect.GetTop()+(icon_rect.GetBottom()-icon_rect.GetTop())/3)};
            
            Point icon_pints_unexpend[4]={Point(icon_rect.GetLeft()+(icon_rect.GetRight()-icon_rect.GetLeft())/3,icon_rect.GetTop()),
                                          Point(icon_rect.GetRight()-(icon_rect.GetRight()-icon_rect.GetLeft())/3,(icon_rect.GetTop()+icon_rect.GetBottom())/2),
                                          Point(icon_rect.GetLeft()+(icon_rect.GetRight()-icon_rect.GetLeft())/3,icon_rect.GetBottom()),
                                          Point(icon_rect.GetLeft()+(icon_rect.GetRight()-icon_rect.GetLeft())/3,icon_rect.GetTop())};
            Graphics graphic(memdc);
            graphic.SetSmoothingMode(SmoothingModeHighQuality);
            GraphicsPath path;
            Pen border_pen(Color(255,75,75,75),1.5);
            path.AddLines(((pti->state&TVIS_EXPANDED)?icon_points:icon_pints_unexpend),3);
            //SolidBrush brush(Color(255,5,5,5));
            LinearGradientBrush brush(icon_rect,Color(255,50,50,50),Color(255,10,10,10),LinearGradientModeVertical);
            graphic.FillPath(&brush,&path);
            graphic.DrawLines(&border_pen,((pti->state&TVIS_EXPANDED)?icon_points:icon_pints_unexpend),4);
        }
        
        DeleteObject(brush_border);
        
        //获取文本 
        //项：pti->hItem ,列：index
        char text[256]="";
        if(index!=0) {
            sprintf(text,"Item %d:(%d,%d)-%d,%d",index,rcItem.left,rcItem.top,rcItem.right-rcItem.left,rcItem.bottom-rcItem.top);
        }
        else {
            sprintf(text,pti->pszText);
        }
        
        //绘制文本
        if(step_level==0&&index!=0) break;//根节点仅需要绘制根
        
        //调整绘制区域
        if(index==0) rcItem.left+=step_level*30+25;
        else InflateRect(&rcItem,-5,0);
        
        //绘制文本    
        DrawText(memdc,text,-1,&rcItem,DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_END_ELLIPSIS);
        
        //锁定首列.
        //...
    }
    SetViewportOrgEx(memdc,pt_org.x,pt_org.y,NULL);
    
    BitBlt(hdc,rc.left,rc.top,cx,cy,memdc,0,0,SRCCOPY);
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    
    return 0;
}

int Tree_HScroll(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    //水平滚动需要重写
    UINT scroll_state=LOWORD(wParam);
    UINT pos=HIWORD(wParam);
    
    if(scroll_state!=SB_THUMBPOSITION) return -1;
    SCROLLINFO si={0};
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_HORZ,&si);
    int offset=(int)pos-(int)si.nPos;//左负 右正
    if(offset==0) return -1;
    
    RECT rc,rcScroll;
    GetClientRect(hwnd,&rc);
    CopyRect(&rcScroll,&rc);
    //滚动一部分
    //刷新一部分
    if(offset>0) {//右滚动 左移
        rc.left=rc.right-offset;
        rcScroll.right-=offset;//rc.left;
    }
    else {
        rc.right=-offset;
        rcScroll.left-=offset;
    }
    ScrollWindow(hwnd,-offset,0,NULL,&rcScroll);
    
    si.fMask=SIF_POS;
    si.nPos=pos;
    SetScrollInfo(hwnd,SB_HORZ,&si,FALSE);
    Tree_NCDrawScrollBar(hwnd,SB_HORZ);
    InvalidateRect(hwnd,&rc,FALSE);
    
    return 0;
}

int Tree_VScroll(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    UINT scroll_state=LOWORD(wParam);
    UINT pos=HIWORD(wParam);
    
    if(scroll_state!=SB_THUMBPOSITION) return -1;
    
    RECT rc,rcScroll;
    GetClientRect(hwnd,&rc);
    CopyRect(&rcScroll,&rc);
    
    SCROLLINFO si={0};
    si.cbSize=sizeof(si);
    si.fMask=SIF_ALL;
    GetScrollInfo(hwnd,SB_VERT,&si);
    int offset=(int)pos-(int)si.nPos;//上负 下正
    if(offset==0) return -1;
    
    int off_pixls=offset*TreeView_GetItemHeight(hwnd);//转移像素.
    
    //滚动一部分
    //刷新一部分
    if(off_pixls>0) {//下滚动 上移
        rc.top=rc.bottom-off_pixls;
        rcScroll.bottom-=off_pixls;
    }
    else {
        rc.bottom=rc.top+(-off_pixls);
        rcScroll.top-=off_pixls;
    }
    //有点特殊，一个offset对应多少pixls
    //ScrollWindow(hwnd,0,-off_pixls,NULL,&rcScroll);
    
    si.cbSize=sizeof(si);
    si.fMask=SIF_POS;
    si.nPos=pos;
    SetScrollInfo(hwnd,SB_VERT,&si,FALSE);
    InvalidateRect(hwnd,&rc,FALSE);
    //{
    //	char text[256]="";
    //	sprintf(text,"pre_pos:%d,cur_pos:%d,scroll_offset:%d,scroll_rect(%d,%d):%d×%d,update_rect(%d,%d):%d×%d",
    //					      si.nPos,pos,
    //					      off_pixls,
    //					      rcScroll.left,rcScroll.top,rcScroll.right-rcScroll.left,rcScroll.bottom-rcScroll.top,
    //					      rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
    //	SetWindowText(GetParent(GetParent(hwnd)),text);
    //}
    return 0;
}


int Tree_ScrollBefore(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    UINT hit=wParam;
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    pTreeStyle ts=Tree_GetSettings(hwnd);
    if(!ts) return -1;
    
    if(hit==HTHSCROLL) {
        RECT rc_hs;
        if(Tree_GetZoneRect(hwnd,ZHSTHUMB,&rc_hs,FALSE)!=0) return -1;
        if(!PtInRect(&rc_hs,pt)) return -1;
            
        
        ScreenToClient(hwnd,&pt);
        ts->drag_pt.x=pt.x;
        ts->drag_pt.y=-1;
        
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        
        ts->drag_pos.x=si.nPos;
        ts->drag_pos.y=-1;    
        
        SetCapture(hwnd);
        return 0;
    }
    else if(hit==HTVSCROLL) {
        RECT rc_vs;
        if(Tree_GetZoneRect(hwnd,ZVSTHUMB,&rc_vs,FALSE)!=0) return -1;
        if(!PtInRect(&rc_vs,pt)) return -1;
        
        ScreenToClient(hwnd,&pt);
        ts->drag_pt.y=pt.y;
        ts->drag_pt.x=-1;
        
        SCROLLINFO si={0};
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS;
        GetScrollInfo(hwnd,SB_VERT,&si);
        
        ts->drag_pos.y=si.nPos;
        ts->drag_pos.x=-1;    
                    
        SetCapture(hwnd);
        return 0;
    }
    else {
        return CallWindowProc(ts->pre_proc,hwnd,WM_NCLBUTTONDOWN,wParam,lParam);
    }
}

int Tree_Scrolling(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    pTreeStyle ts=Tree_GetSettings(hwnd);
    if(!ts) return -1;
        
    if(GetCapture()!=hwnd) return -1;
    if(ts->drag_pos.x!=-1) { // HORZ 水平滚动
        RECT rc_hs,rc_thumb;
        
        if(Tree_GetZoneRect(hwnd,ZHSCROLL,&rc_hs,TRUE)!=0) return -1;
        if(Tree_GetZoneRect(hwnd,ZHSTHUMB,&rc_thumb,TRUE)!=0) return -1;
        
        SCROLLINFO si;
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_HORZ,&si);
        
        si.nPos=(int)ts->drag_pos.x+(int)(si.nMax-si.nMin+1-si.nPage)*1.0*(int)(pt.x-ts->drag_pt.x)/((rc_hs.right-rc_hs.left)-(rc_thumb.right-rc_thumb.left));   
        if(si.nPos==ts->drag_pos.x) return -1;
        
        if((int)si.nPos>(int)(si.nMax-si.nPage+1)) si.nPos=si.nMax-si.nPage+1;
        else if(si.nPos<si.nMin) si.nPos=si.nMin;
        
        SendMessage(hwnd,WM_HSCROLL,MAKEWPARAM(SB_THUMBPOSITION,si.nPos),0);
        
        //更新Head的offset.
        HWND head=GetDlgItem(GetParent(hwnd),ID_GRIDHEADER_01);
        SendMessage(head,WM_HSCROLL,MAKEWPARAM(SB_THUMBPOSITION,si.nPos),0);
    }
    else if(ts->drag_pos.y!=-1) { //垂直滚动
        RECT rc_vs,rc_thumb;
        
        if(Tree_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,TRUE)!=0) return -1;
        if(Tree_GetZoneRect(hwnd,ZVSTHUMB,&rc_thumb,TRUE)!=0) return -1;
        
        SCROLLINFO si;
        si.cbSize=sizeof(si);
        si.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_VERT,&si);
        
        si.nPos=(int)ts->drag_pos.y+((int)(si.nMax-si.nMin+1-si.nPage))*1.0*((int)(pt.y-ts->drag_pt.y))/((rc_vs.bottom-rc_vs.top)-(rc_thumb.bottom-rc_thumb.top));   
        if(si.nPos==ts->drag_pos.y) return -1;
        
        if((int)si.nPos>(int)(si.nMax-si.nPage+1)) si.nPos=si.nMax-si.nPage+1;
        else if(si.nPos<si.nMin) si.nPos=si.nMin;
        
        SendMessage(hwnd,WM_VSCROLL,MAKEWPARAM(SB_THUMBPOSITION,si.nPos),0);
        //{
        //    char text[256]="";
        //    sprintf(text,"Range(%d~%d),pos:%d,page:%d",si.nMin,si.nMax,si.nPos,si.nPage);
        //    SetWindowText(GetParent(GetParent(hwnd)),text);    
        //}
    }
    return 0;
}

int Tree_ScrollDone(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    pTreeStyle ts=Tree_GetSettings(hwnd);
    if(!ts) return -1;
    
    if(GetCapture()==hwnd) {
        ReleaseCapture();
        if(ts->drag_pos.y!=-1) Tree_NCDrawScrollBar(hwnd,SB_VERT);
        if(ts->drag_pos.x!=-1) Tree_NCDrawScrollBar(hwnd,SB_HORZ);
        
        ts->drag_pt.x=ts->drag_pt.y=ts->drag_pos.x=ts->drag_pos.y=-1;            
    }
    
    return 0;
}

int Tree_Size(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    int height=(int)HIWORD(lParam);
    int width=(int)LOWORD(lParam);
    HWND head=GetDlgItem(GetParent(hwnd),ID_GRIDHEADER_01);
    pHeadStyle hs=Head_GetSettings(head);
    
    SCROLLINFO si={0};
    si.cbSize=sizeof(si);
    si.fMask=SIF_RANGE|SIF_POS|SIF_PAGE;
    si.nMin=0;
    si.nMax=Head_GetTotalSize(head);
    si.nPage=width;
    si.nPos=-(hs->offset);
    SetScrollInfo(hwnd,SB_HORZ,&si,TRUE);
    
    InvalidateRect(hwnd,NULL,TRUE);
    return 0;
}

int Tree_Hover(HWND hwnd,WPARAM wParam,LPARAM lParam)
{    
    pTreeStyle ts=Tree_GetSettings(hwnd);
    if(!ts) return -1;
        
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    if(GetCapture()!=hwnd) {
        TVHITTESTINFO tv_hit={0};
        HTREEITEM item={0},pre_item=ts->hover_item;
        
        memcpy(&tv_hit.pt,&pt,sizeof(pt));
        item=TreeView_HitTest(hwnd,&tv_hit);
        ts->hover_item=item;
        RECT rcItem; 
        if(pre_item&&pre_item!=item) {
            TreeView_GetItemRect(hwnd,pre_item,&rcItem,FALSE);                
            InvalidateRect(hwnd,&rcItem,TRUE);
        }
        if(item&&pre_item!=item) {
            TreeView_GetItemRect(hwnd,item,&rcItem,FALSE);
            InvalidateRect(hwnd,&rcItem,TRUE); 
        }
    }
    return 0;
}

int Tree_NCCalcSize(HWND hwnd,WPARAM wParam,LPARAM lParam)
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

int Tree_NCDrawScrollBar(HWND hwnd,UINT scrolltype)
{
    RECT rc;
    RECT rcThumb;
    HDC hdc=GetWindowDC(hwnd);
    UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
            
    if(scrolltype==SB_VERT) {
        if(Tree_GetZoneRect(hwnd,ZVSCROLL,&rc,TRUE)==0&&
           Tree_GetZoneRect(hwnd,ZVSTHUMB,&rcThumb,TRUE)==0) {
            int cx=rc.right-rc.left,cy=rc.bottom-rc.top;
           
            if((style&WS_HSCROLL)) cy+=SCROLLBAR_PIXLS;
           
            HDC memdc=CreateCompatibleDC(hdc);
            HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy); 
            HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
           
            RECT rcMem;
            CopyRect(&rcMem,&rc);
            OffsetRect(&rcMem,-rcMem.left,-rcMem.top);
            
            //GradientRect(memdc,rcMem,RGB(15,15,15),RGB(20,20,20),0);
            HBRUSH brush=CreateSolidBrush(RGB(5,5,5));
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
        if(Tree_GetZoneRect(hwnd,ZHSCROLL,&rc,TRUE)==0&&
           Tree_GetZoneRect(hwnd,ZHSTHUMB,&rcThumb,TRUE)==0) {
            int cx=rc.right-rc.left,cy=rc.bottom-rc.top;
           
            HDC memdc=CreateCompatibleDC(hdc);
            HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy); 
            HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
           
            RECT rcMem;
            CopyRect(&rcMem,&rc);
            OffsetRect(&rcMem,-rcMem.left,-rcMem.top);
            HBRUSH brush=CreateSolidBrush(RGB(5,5,5));
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
        {rcGrid.right,rcGrid.top},\
        {rcGrid.right,rcGrid.bottom},\
        {rcGrid.top,rcGrid.bottom},\
        {0,0}\
        };
        HPEN pen=CreatePen(PS_SOLID,1,RGB(15,15,15));
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

int Tree_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL isallign_top_left)
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
        if(Tree_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,FALSE)!=0) break;
        
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
        if(Tree_GetZoneRect(hwnd,ZHSCROLL,&rc_hs,FALSE)!=0) break;
        
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
    }
    
    if((result==0)&&isallign_top_left) {
        OffsetRect(prc,-rc.left,-rc.top);
    }
    
    return result;
}

int Tree_HitTest(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    pTreeStyle ts=Tree_GetSettings(hwnd);
    if(!ts) return -1;
    
    RECT rc_hs,rc_vs;
    if(Tree_GetZoneRect(hwnd,ZHSCROLL,&rc_hs,FALSE)==0) {
        if(PtInRect(&rc_hs,pt)) return HTHSCROLL;
    }
    
    if(Tree_GetZoneRect(hwnd,ZVSCROLL,&rc_vs,FALSE)==0) {
        if(PtInRect(&rc_vs,pt)) return HTVSCROLL;
    }
    
    return CallWindowProc(ts->pre_proc,hwnd,WM_NCHITTEST,wParam,lParam);
}

LRESULT CALLBACK TreeOwnProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    pTreeStyle ts=Tree_GetSettings(hwnd);
    if(!ts) return -1;
        
    switch(message) {
    case WM_NCCALCSIZE: {
        return Tree_NCCalcSize(hwnd,wParam,lParam);
    } break;
    case WM_PAINT: {
        LRESULT ret=CallWindowProc(ts->pre_proc,hwnd,message,wParam,lParam);
        Tree_NCDrawScrollBar(hwnd,SB_HORZ);
        Tree_NCDrawScrollBar(hwnd,SB_VERT);
        
        //刷新WM_ERASEBKGND取消的背景部分（未被item覆盖的区域）
        RECT rc,rcLast={0},rcBack;
        GetClientRect(hwnd,&rc);
        CopyRect(&rcBack,&rc);
        
        HTREEITEM item=TreeView_GetLastVisible(hwnd);
        if(item) TreeView_GetItemRect(hwnd,item,&rcLast,FALSE);
        
        rcBack.top=rcLast.bottom;
        if(rcBack.left<rcBack.right&&rcBack.top<rcBack.bottom) {
            HDC hdc=GetDC(hwnd);
            HDC memdc=CreateCompatibleDC(hdc);
            RECT rcMem;
            CopyRect(&rcMem,&rcBack);
            OffsetRect(&rcMem,-rcMem.left,-rcMem.top);
            int cx=rcBack.right-rcBack.left,cy=rcBack.bottom-rcBack.top;
            HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy);
            HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
            HBRUSH brush=CreateSolidBrush(RGB(20,20,20));
            FillRect(memdc,&rcMem,brush);
            
            BitBlt(hdc,rcBack.left,rcBack.top,cx,cy,memdc,0,0,SRCCOPY);
            DeleteObject(SelectObject(memdc,pre_bmp));
            DeleteObject(brush);
            DeleteDC(memdc);
            ReleaseDC(hwnd,hdc);
        }    
        return ret;
    } break;
    case WM_NCACTIVATE:
    case WM_NCPAINT: {
        Tree_NCDrawScrollBar(hwnd,SB_HORZ);
        Tree_NCDrawScrollBar(hwnd,SB_VERT);
        return 0;
    } break;
    case WM_HSCROLL: {
        Tree_HScroll(hwnd,wParam,lParam);
    } break;
    case WM_VSCROLL: {
        Tree_VScroll(hwnd,wParam,lParam);
        //return 0;
    } break;
    case WM_NCDESTROY: {
        WNDPROC pre_proc=ts->pre_proc;
        Tree_ClearSettings(hwnd);
        return CallWindowProc(ts->pre_proc,hwnd,message,wParam,lParam);
    } break;
    case WM_ERASEBKGND: {        
        return 1;
    } break;
    case WM_NCHITTEST: {
        return Tree_HitTest(hwnd,wParam,lParam);
    }
    case WM_NCLBUTTONDOWN: {
        return Tree_ScrollBefore(hwnd,wParam,lParam);
    } break;
    case WM_MOUSEMOVE: {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
        if(GetCapture()!=hwnd) return Tree_Hover(hwnd,wParam,lParam);        
        Tree_Scrolling(hwnd,wParam,lParam);
    } break;
    case WM_LBUTTONUP: {
        Tree_ScrollDone(hwnd,wParam,lParam);
    } break;
    case WM_SIZE: {            
        Tree_Size(hwnd,wParam,lParam);        
    } break;
    case WM_TREEVIEWCUSTOMDRAW: {
        return Tree_Draw(hwnd,wParam,lParam);
    } break;
    }
    
    return CallWindowProc(ts->pre_proc,hwnd,message,wParam,lParam);     
}


int TreeGrid_InitialSettings(HWND hwnd)
{
    pTreeGrid tg=(pTreeGrid)calloc(sizeof(RTreeGrid),1);
    if(!tg) return -1;

    tg->head=GetDlgItem(hwnd,ID_GRIDHEADER_01);
    tg->tree=GetDlgItem(hwnd,ID_TREEVIEW_01);
    
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)tg);
    return 0;
}

pTreeGrid TreeGrid_GetSettings(HWND hwnd)
{
    return (pTreeGrid)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int TreeGrid_ClearSettings(HWND hwnd)
{
    pTreeGrid tg=TreeGrid_GetSettings(hwnd);
    
    if(tg) free(tg);
    
    return 0;
}

LRESULT CALLBACK TreeGridOwnProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    switch(message)
    {
    case WM_CREATE: {
        TreeGrid_CtrlCreate(hwnd);
    } break;
    case WM_ERASEBKGND: {
        return 1;
    } break;
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
            
            client_rect_new = {rect_new.left+(HasBorder?1:0),
                               rect_new.top+(HasBorder?1:0),
                               rect_new.right-((HasBorder?1:0)),
                               rect_new.bottom-((HasBorder?1:0))};
            CopyRect(&(calc_param->rgrc[0]),&client_rect_new);
            CopyRect(&(calc_param->rgrc[1]),&rect_new);
            CopyRect(&(calc_param->rgrc[2]),&rect_old);
            
            return WVR_VALIDRECTS;
        }
        else {
            RECT* prect = (RECT*)lParam;
            prect->right-=(HasBorder?1:0); 
            prect->bottom-=(HasBorder?1:0);
            prect->left+=(HasBorder?1:0);
            prect->top+=(HasBorder?1:0);
            return 0;
        }
    } break;
    case WM_NCACTIVATE:
    case WM_NCPAINT: {
        HDC hdc=GetWindowDC(hwnd);
        UINT style=(UINT)GetWindowLongPtr(hwnd,GWL_STYLE);
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
            HPEN pen=CreatePen(PS_SOLID,1,RGB(53,53,53));
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
    case WM_NCDESTROY: {
        TreeGrid_ClearSettings(hwnd);
    } break;
    case WM_NOTIFY: {
        UINT ctrl_id = wParam;
        LPNMHDR pnmh = (LPNMHDR)lParam;
        
        HWND ctrl_hwnd = pnmh->hwndFrom;
        char class_name[256] = "";
        LRESULT result;
        
        GetClassName(ctrl_hwnd,class_name,sizeof(class_name));
        if(strstr(class_name,WC_TREEVIEW)) {
            LPNMTREEVIEW item =(LPNMTREEVIEW)pnmh;
            
            switch(item->hdr.code) {
            case NM_CUSTOMDRAW: {
                result=SendMessage(ctrl_hwnd,WM_TREEVIEWCUSTOMDRAW,wParam,lParam);
                return result;
            } break;
            }
        }
        else if(strstr(class_name,WC_HEADER)) {
            if(pnmh->code==GRID_UPDATE) {
                RECT rc;
                pTreeGrid tg=TreeGrid_GetSettings(hwnd);
                if(!tg) break;
                    
                GetClientRect(tg->tree,&rc);
                pHeadStyle hs=Head_GetSettings(tg->head);
                
                LPNHDRGrid grid=(LPNHDRGrid)pnmh;
                rc.left=grid->cx1;
                rc.right=grid->cx2;
                
                InvalidateRect(tg->tree,&rc,TRUE);
                
                SCROLLINFO si={0};
                si.cbSize=sizeof(si);
                si.fMask=SIF_RANGE|SIF_POS|SIF_PAGE;
                si.nMin=0;
                si.nMax=Head_GetTotalSize(tg->head);
                si.nPage=rc.right-rc.left;
                si.nPos=-(hs->offset);
                SetScrollInfo(tg->tree,SB_HORZ,&si,TRUE);
                SendMessage(tg->tree,WM_HSCROLL,MAKEWPARAM(SB_THUMBPOSITION,si.nPos),0);
            }
        }
    } break;
    case WM_SIZE: {
        int height=(int)HIWORD(lParam);
        int width=(int)LOWORD(lParam);
        
        pTreeGrid tg=TreeGrid_GetSettings(hwnd);
        if(!tg) break;
            
        HWND head=tg->head;
        HWND tree=tg->tree;
        
        RECT head_rect;
        int head_height=0;
        GetClientRect(head,&head_rect);
        head_height=head_rect.bottom-head_rect.top;
        SetWindowPos(head,NULL,0,0,width,head_height,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
        SetWindowPos(tree,NULL,0,head_height,width,height-head_height,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOREDRAW);
    } break;
    }
    return DefWindowProc(hwnd,message,wParam,lParam);
}

int TreeGrid_CtrlCreate(HWND hwnd)
{
    HINSTANCE instance=(HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);
    HWND tree=CreateWindow(WC_TREEVIEW,NULL,WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL,\
    50,85,1000,450,hwnd,(HMENU)ID_TREEVIEW_01,instance,NULL);
    Tree_InitialSettings(tree);
    
    HWND head=CreateWindow(WC_HEADER,NULL,WS_CHILD|WS_VISIBLE|HDS_BUTTONS|HDS_HORZ,\
    50,50,1000,25,hwnd,(HMENU)ID_GRIDHEADER_01,instance,NULL);
    Head_InitialSettings(head);    
    HFONT font=CreateFont(15,0,0,0,
                    FW_MEDIUM,//FW_SEMIBOLD,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "Courier New");
    SendMessage(head,WM_SETFONT,(WPARAM)font,0);
    
    /*add items.*/                  
    {
        HTREEITEM tree_root;
        for(int i=0;i<1000;++i) {
            char buffer[255] = "";
            TV_INSERTSTRUCT tree_item;
            TV_ITEM tree_node;
            
            if(i%5==0) {
                tree_item.hParent = TVI_ROOT;
                tree_item.hInsertAfter = TVI_LAST;
                
                tree_node.mask = TVIF_TEXT;
                tree_node.pszText = buffer;
                sprintf(buffer,"ROOT-根节点-%d",i);
                tree_item.item = tree_node;
                tree_root = TreeView_InsertItem(tree,&tree_item);
            }
            else {
                tree_item.hParent = tree_root;
                sprintf(buffer,"CHILD-子节点-%d",i);
                HTREEITEM tree_tmp = TreeView_InsertItem(tree,&tree_item);
                if(i%4==0) tree_root = tree_tmp;
            }
        }
        
        HFONT tree_font=CreateFont(14,0,0,0,
                    FW_MEDIUM,//FW_SEMIBOLD,
                    FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, 
                    VARIABLE_PITCH,
                    "Courier New");
        TreeView_SetBkColor(tree,RGB(20,20,20));
        TreeView_SetTextColor(tree,RGB(0,120,250));
        TreeView_SetItemHeight(tree,25);
        SendMessage(tree,WM_SETFONT,(WPARAM)tree_font,0);
    }
    
    {
        HDITEM hdi={0};
        char treegrid_title[][256]={\
        "Column 1",\
        "Column 2",\
        "Column 3",\
        "Column 4",\
        "时间戳",  \
        "链接操作",\
        "Column 5",\
        "Column A",\
        "Column B",\
        "Column C",\
        "Column D",\
        "Column E",\
        "Column F",\
        "Column G",\
        "Column H",\
        "利",\
        "威",\
        "名"};
        
        for(int index=0;index<sizeof(treegrid_title)/sizeof(char[256]);index++) {
            hdi.mask=HDI_TEXT|HDI_FORMAT|HDI_WIDTH;
            hdi.cxy=180;
            hdi.pszText=treegrid_title[index];
            hdi.cchTextMax=256;
            hdi.fmt=HDF_CENTER|HDF_STRING;
            
            SendMessage(head,HDM_INSERTITEM,(WPARAM)index,(LPARAM)&hdi);
            
            RECT rcClient;
            GetClientRect(tree,&rcClient);
            
            SCROLLINFO si={0};
            si.cbSize=sizeof(si);
            si.fMask=SIF_RANGE|SIF_POS|SIF_PAGE;
            si.nMin=0;
            si.nMax=Head_GetTotalSize(head);
            si.nPage=rcClient.right-rcClient.left;
            si.nPos=0;
            SetScrollInfo(tree,SB_HORZ,&si,TRUE);
        }
    }
    
    TreeGrid_InitialSettings(hwnd);
    
    return 0;
}

