#include "../include/datepick.h"

int DatePick_InitialSettings(HWND hwnd)
{
    pDatePickStyle ds=(pDatePickStyle)calloc(sizeof(RDatePickStyle),1);
    if(!ds) return -1;
    
    ds->clrbkgnd=RGB(10,10,10);
    ds->font=CreateFont(17,0,0,0,
                        FW_MEDIUM,//FW_SEMIBOLD,
                        FALSE,FALSE,FALSE,
                        DEFAULT_CHARSET,
                        OUT_OUTLINE_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY,
                        VARIABLE_PITCH,
                        "Courier New");
    SendMessage(hwnd,WM_SETFONT,(WPARAM)ds->font,0);
    
    ds->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)DTPOwnerProc);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)ds);
    
    //alter calender bkcolor.
    DateTime_SetMonthCalColor(hwnd,MCSC_BACKGROUND,RGB(30,30,30));// blue
    DateTime_SetMonthCalColor(hwnd,MCSC_MONTHBK,RGB(30,30,30));// green
    DateTime_SetMonthCalColor(hwnd,MCSC_TITLEBK,RGB(50,50,50));//
    DateTime_SetMonthCalColor(hwnd,MCSC_TITLETEXT,RGB(200,200,200));//
    DateTime_SetMonthCalColor(hwnd,MCSC_TRAILINGTEXT,RGB(250,0,0));// green
    DateTime_SetMonthCalColor(hwnd,MCSC_TEXT,RGB(200,200,200));// green
    
    return 0;
}

pDatePickStyle DatePick_GetSettings(HWND hwnd)
{
    return (pDatePickStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int DatePick_ClearSettings(HWND hwnd)
{
    pDatePickStyle ds=DatePick_GetSettings(hwnd);
    if(!ds) return -1;
    
    DeleteObject(ds->font);
    free(ds);
    
    return 0;
}

LRESULT CALLBACK DTPOwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    pDatePickStyle ds=DatePick_GetSettings(hwnd);
    if(!ds) return 0;
    
    switch(msg) {
    case WM_ERASEBKGND: {
        return 1;
    } break;
    case WM_PAINT: {
        return DatePick_Paint(hwnd,wParam,lParam);
    } break;/*
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    {
        HDC dc=(HDC)wParam;
        HWND ctrl=(HWND)lParam;
        SetTextColor(dc,RGB(250,0,0));
        
        return (HRESULT)CreateSolidBrush(RGB(255,0,0));
    } break;*/
    case WM_NCDESTROY: {
        WNDPROC pre_proc=ds->pre_proc;
        DatePick_ClearSettings(hwnd);
        return CallWindowProc(pre_proc,hwnd,msg,wParam,lParam);
    } break;
    }
    
    return CallWindowProc(ds->pre_proc,hwnd,msg,wParam,lParam);
}

int DatePick_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    pDatePickStyle pds=DatePick_GetSettings(hwnd);
    if(!pds) return -1;
        
    PAINTSTRUCT ps;
    BeginPaint(hwnd,&ps);
    HDC hDC=ps.hdc;
    COLORREF clrBkgnd=pds->clrbkgnd;
    
    RECT rect;
    GetWindowRect(hwnd, &rect);
    MapWindowPoints(NULL,hwnd,(LPPOINT)&rect,2);
    long nWidth=rect.right-rect.left, nHeight=rect.bottom-rect.top;

    HDC hDCMem=CreateCompatibleDC(hDC);
    HBITMAP hBitmap=CreateBitmap(nWidth,nHeight,GetDeviceCaps(hDC,PLANES),GetDeviceCaps(hDC,BITSPIXEL),(const void*)NULL);
    if(hBitmap) {
        HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hDCMem, hBitmap);

        //Render control itself
        SendMessage(hwnd, WM_PRINT, (WPARAM)hDCMem, PRF_CLIENT | PRF_CHILDREN | PRF_NONCLIENT);

        //Only if we have the color
        if(clrBkgnd != NULL) {
            //Only if control is enabled
            if((GetWindowLongPtr(hwnd,GWL_STYLE)&(WS_VISIBLE|WS_DISABLED))==(WS_VISIBLE|0)) {
                #define ALLOWED_DIFF 64

                DWORD dwBkgClr=GetSysColor(COLOR_WINDOW);   //0xFFFFFF;

                DWORD br0=dwBkgClr&0xFF;
                DWORD br1=(dwBkgClr&0xFF00)>>8;
                DWORD br2=(dwBkgClr&0xFF0000)>>(8*2);

                for(int y=0;y<nHeight;y++) {
                    for(int x=0; x<nWidth;x++) {
                        COLORREF clrPxl=GetPixel(hDCMem,x,y);

                        DWORD r0=clrPxl&0xFF;
                        DWORD r1=(clrPxl&0xFF00)>>8;
                        DWORD r2=(clrPxl&0xFF0000)>>(8 * 2);

                        int nDiff_r0=r0-br0;
                        int nDiff_r1=r1-br1;
                        int nDiff_r2=r2-br2;

                        if(abs(nDiff_r0) < ALLOWED_DIFF &&
                           abs(nDiff_r1) < ALLOWED_DIFF &&
                           abs(nDiff_r2) < ALLOWED_DIFF) {
                            SetPixel(hDCMem, x, y, clrBkgnd);
                        }
                    }
                }
            }
        }
        
        DATETIMEPICKERINFO dtpi={0};
        dtpi.cbSize=sizeof(dtpi);
        DateTime_GetDateTimePickerInfo(hwnd,&dtpi);
        
        SYSTEMTIME st={0};
        DateTime_GetSystemtime(hwnd,&st);
        char text[256]="";
        
        SelectObject(hDCMem,(HFONT)SendMessage(hwnd,WM_GETFONT,0,0));
        HBRUSH brush=CreateSolidBrush(RGB(0,120,250));
        
        RECT rcText;
        CopyRect(&rcText,&rect);
        rcText.left+=(rect.bottom-rect.top-1);
        rcText.top-=1;
        
        HBRUSH brushbk=CreateSolidBrush(clrBkgnd);
        FillRect(hDCMem,&dtpi.rcCheck,brushbk);
        DeleteObject(brushbk);
        SetBkColor(hDCMem,clrBkgnd);
        SetBkMode(hDCMem,OPAQUE);
        if((STATE_SYSTEM_CHECKED&dtpi.stateCheck)==STATE_SYSTEM_CHECKED) {
            SetTextColor(hDCMem,RGB(0,100,200)); 
            //DrawText(hDCMem,"□",-1,&dtpi.rcCheck,DT_SINGLELINE|DT_VCENTER|DT_CENTER);   
            DrawText(hDCMem,"√",-1,&dtpi.rcCheck,DT_SINGLELINE|DT_VCENTER|DT_CENTER);   
            sprintf(text,"%04d/%02d/%02d",st.wYear,st.wMonth,st.wDay);
        }
        else {
            SetTextColor(hDCMem,RGB(100,100,100));
            DrawText(hDCMem,"□",-1,&dtpi.rcCheck,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
            sprintf(text,"0000/00/00");
        }
        DrawText(hDCMem,text,-1,&rcText,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
                
        FrameRect(hDCMem,&rect,brush);
        DeleteObject(brush);

        BitBlt(hDC, rect.left, rect.top, nWidth, nHeight, hDCMem, 0, 0, SRCCOPY);

        SelectObject(hDCMem, hBitmapOld);
        DeleteObject(hBitmap);
    }

    DeleteDC(hDCMem);
    EndPaint(hwnd,&ps);
    return 0;
}

void DatePick_Notify(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    LPNMHDR phdr=(LPNMHDR)lParam;
    UINT ctrlid=wParam;
    if(phdr->code==DTN_DATETIMECHANGE) {
        //DateTimePicker Alter Notify.
    }
    else if(phdr->code==DTN_DROPDOWN) {
        HWND hMC=DateTime_GetMonthCal(GetDlgItem(hwnd,ctrlid));
        SetWindowTheme(hMC,L"",L"");//必须先取消theme
        HWND parent=GetParent(hMC);
        RECT rcP;
        GetWindowRect(parent,&rcP);
        RECT rc;
        MonthCal_GetMinReqRect(hMC, &rc);
        MoveWindow(parent,0,0,rc.right-rc.left,rc.bottom-rc.top,FALSE);
        MoveWindow(hMC,rc.left,rc.top,rc.right,rc.bottom,FALSE);
    }
}