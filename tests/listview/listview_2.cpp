#include<windows.h>
#include<windowsx.h>
#include<math.h>
#include<stdio.h>
#include<commctrl.h>

#include "../../include/listview.h"
#pragma comment(lib,"../../lib/listview.lib")

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"comctl32.lib")

#define IDC_LISTVIEW_01 0x0001
#define IDC_BUTTON_01 0x0002
#define IDC_BUTTON_02 0x0003
#define IDC_BUTTON_03 0x0004
#define IDC_BUTTON_04 0x0005
 
#define WINDOW_CLASS_NAME "ListView Owner Drawing"
#define RWIDTH(A)   abs(A.right - A.left)   
#define RHEIGHT(A)  abs(A.bottom - A.top)  

HINSTANCE instance;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

LRESULT CALLBACK WindowProc(HWND,UINT,WPARAM,LPARAM);
static char listindex[][256]={\
    "编号%05d",
    "房间号-RoomNo.:%d",
    //"mechine:%d",
    //"car:%d",
    "%d",
    "video:%d",
    //"Desc.:%d"
    "%.2lf",
    "information %d"
};

static char title[][256]={\
    "操作(&O)",
    "Row No.",
    //"Col 1",
    //"Col 2",
    "发生总金额（賬）",
    "Col 3",
    //"Col 4",
    "金额（賬）",//
    "Col 7"
};

static int title_width[]={\
    200,150,250,150,250,250,150,150,200
};

HIMAGELIST InitialImgList(void)
{
    HIMAGELIST list=ImageList_Create(32,32,ILC_MASK|ILC_COLOR32,1,1);
    
    ImageList_AddIcon(list,LoadIcon(instance,"IDC_ICON_1"));
    ImageList_AddIcon(list,LoadIcon(instance,"IDC_ICON_2"));
    ImageList_AddIcon(list,LoadIcon(instance,"IDC_ICON_3"));
    ImageList_AddIcon(list,LoadIcon(instance,"IDC_ICON_4"));
    
    return list;
}

void InsertColumn(HWND listctrl)
{
    int columns=sizeof(title)/sizeof(char[256]);
    int col_i=0;
    
    while(col_i<columns) {
        LV_COLUMN lvc={0};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        lvc.fmt=LVCFMT_LEFT;
        lvc.pszText=title[col_i];
        lvc.cchTextMax=255;
        lvc.cx=title_width[col_i];
        
        ListView_InsertColumn(listctrl,col_i,&lvc);
        
        col_i++;
    }
}
 
void InitListView(HWND listctrl)
{
    int i_row=0;
    int i_col=0;
    int rows=1000;
    int columns=sizeof(listindex)/sizeof(char[256]);
 
    while(i_row<rows) {
        i_col=0;
        while(i_col<columns) {
            LV_ITEM lv={0};
            char text[256]="";
            
            if(strstr(title[i_col],"金额")) sprintf(text,listindex[i_col],i_row*100000000.0/1.22*i_col*(i_row%3?-1:1));
            else sprintf(text,listindex[i_col],i_row);
            lv.mask=LVIF_TEXT;
            lv.iItem=i_row;
            lv.iSubItem=i_col;
            lv.pszText=text;
            lv.cchTextMax=256;
            
            if(i_col==0) ListView_InsertItem(listctrl,&lv);
            ListView_SetItem(listctrl,&lv);
            i_col++;
        }
        
        i_row++;
    }
}
 
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, PSTR szcmdLine, int icmdshow)
{
    HWND hwnd;
    MSG msg;
    WNDCLASSEX winclass;
    InitCommonControls();
    instance=hinstance;
     
    winclass.cbSize=sizeof(WNDCLASSEX);
    winclass.style=CS_HREDRAW|CS_VREDRAW;//NULL;//CS_DBLCLKS
    winclass.lpfnWndProc=WindowProc;
    winclass.cbClsExtra=0;
    winclass.cbWndExtra=0;
    winclass.hInstance=hinstance;
    winclass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    winclass.hCursor=LoadCursor(NULL,IDC_ARROW);
    winclass.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
    winclass.lpszMenuName=NULL;
    winclass.lpszClassName=WINDOW_CLASS_NAME;
    winclass.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
 
    if(!RegisterClassEx(&winclass)) return (0);
    
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
           
    if(!(hwnd = CreateWindowEx(NULL,
                               WINDOW_CLASS_NAME,
                               "ListGrid",
                               WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                               240,262,
                               800,500,
                               NULL, 
                               NULL, 
                               hinstance, 
                               NULL)))
        return (0);
    
    while(GetMessage(&msg,NULL,0,0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    GdiplusShutdown(gdiplusToken);
    
    return(msg.wParam);
}
 
LRESULT CALLBACK WindowProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE: {
             
            HWND listctrl=CreateWindowEx(0,
            WC_LISTVIEW,NULL,
            WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS|WS_BORDER,
            20,40,400,150,
            hwnd,(HMENU)IDC_LISTVIEW_01,
            (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),NULL);                       
            InsertColumn(listctrl);
            InitListView(listctrl);
            
            ListView_InitialSettings(listctrl);
            HWND head=ListView_GetHeader(listctrl);
            SetWindowLongPtr(head,GWL_STYLE,(~WS_VISIBLE)&((UINT)GetWindowLongPtr(head,GWL_STYLE)));
            LVHead_InitialSettings(head);
            
            ListView_GetSettings(listctrl)->imglist=InitialImgList();
            //ListView_SetImageList(listctrl,LVSIL_NORMAL,InitialImgList());
            
            
            HWND btnInsert=CreateWindowEx(0,
            WC_BUTTON,"Insert",
            WS_CHILD|WS_VISIBLE|WS_BORDER,
            20,40,400,150,
            hwnd,(HMENU)IDC_BUTTON_01,
            (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),NULL); 
            HWND btnClear=CreateWindowEx(0,
            WC_BUTTON,"Clear",
            WS_CHILD|WS_VISIBLE|WS_BORDER,
            20,40,400,150,
            hwnd,(HMENU)IDC_BUTTON_02,
            (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),NULL);
            HWND btnTest=CreateWindowEx(0,
            WC_BUTTON,"Test",
            WS_CHILD|WS_VISIBLE|WS_BORDER,
            20,40,400,150,
            hwnd,(HMENU)IDC_BUTTON_03,
            (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),NULL);
            HWND altTest=CreateWindowEx(0,
            WC_BUTTON,"Alter",
            WS_CHILD|WS_VISIBLE|WS_BORDER,
            20,40,400,150,
            hwnd,(HMENU)IDC_BUTTON_04,
            (HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),NULL);
            
            LOGFONT lf={0};
            GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
            lf.lfHeight=18;
            lf.lfWeight=20;
            strcpy(lf.lfFaceName,"微软雅黑");
            HFONT font=CreateFontIndirect(&lf);
            SendMessage(btnInsert,WM_SETFONT,(WPARAM)font,0);  
            SendMessage(btnClear,WM_SETFONT,(WPARAM)font,0);  
            SendMessage(btnTest,WM_SETFONT,(WPARAM)font,0);  
            SendMessage(altTest,WM_SETFONT,(WPARAM)font,0);   
        } break;
        case WM_MEASUREITEM: {
            LPMEASUREITEMSTRUCT lpmis=(LPMEASUREITEMSTRUCT)lParam;
            if(lpmis->CtlType == ODT_LISTVIEW) {
                SendMessage(GetDlgItem(hwnd,lpmis->CtlID),WMYU_MEASURELISTVIEWITEM,wParam,lParam);
            }
        } break;/*
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdraw=(LPDRAWITEMSTRUCT)lParam;
            if(pdraw->CtlType == ODT_LISTVIEW) {
                return SendMessage(pdraw->hwndItem,WMYU_DRAWLISTVIEWITEM,wParam,lParam);
            }
        } break;*/
        case WM_SIZE: {
            int cx=LOWORD(lParam);
            int cy=HIWORD(lParam);
            MoveWindow(GetDlgItem(hwnd,IDC_LISTVIEW_01),5,5,cx-150,cy-10,TRUE);
        
            HWND btnInsert=GetDlgItem(hwnd,IDC_BUTTON_01);
            HWND btnClear=GetDlgItem(hwnd,IDC_BUTTON_02);
            HWND btnTest=GetDlgItem(hwnd,IDC_BUTTON_03);
            HWND btnAlt=GetDlgItem(hwnd,IDC_BUTTON_04);
            
            MoveWindow(btnInsert,cx-120,10,100,30,TRUE);
            MoveWindow(btnClear,cx-120,50,100,30,TRUE);
            MoveWindow(btnTest,cx-120,90,100,30,TRUE);
            MoveWindow(btnAlt,cx-120,130,100,30,TRUE);
        } break;
        case WM_COMMAND: {
            if(wParam==IDC_BUTTON_01) {
                InitListView(GetDlgItem(hwnd,IDC_LISTVIEW_01));
            }
            else if(wParam==IDC_BUTTON_02) {
                ListView_DeleteAllItems(GetDlgItem(hwnd,IDC_LISTVIEW_01));
            }
            else if(wParam==IDC_BUTTON_03) {
                HWND listctrl=GetDlgItem(hwnd,IDC_LISTVIEW_01);
                ListView_SetColumnWidth(listctrl,2,55);
                int cols=Header_GetItemCount(ListView_GetHeader(listctrl));
                int* col_orders=(int*)calloc(cols,sizeof(int));
                col_orders[0]=5;
                col_orders[1]=4;
                col_orders[2]=2;
                col_orders[3]=3;
                col_orders[4]=1;
                col_orders[5]=0;
                col_orders[6]=6;
                col_orders[7]=7;
                //Fucked.
                //ListView_SetColumnOrderArray(listctrl,cols,col_orders);
                free(col_orders);
            }
            else if(wParam==IDC_BUTTON_04) {
                char search_text[256]="126736";
                HWND listctrl=GetDlgItem(hwnd,IDC_LISTVIEW_01);
                LVFINDINFO fi={0};
                fi.flags=LVFI_SUBSTRING;
                fi.psz=search_text;
                
                UINT itemID=ListView_FindItem(listctrl,-1,&fi);
                if(itemID!=-1) {
                    ListView_SetItemState(listctrl,itemID,LVIS_FOCUSED,LVIS_FOCUSED);
                    ListView_SetItemText(listctrl,itemID,4,"SAC");
                }
                else {
                    char msg_text[256]="";
                    sprintf(msg_text,"Item not found\nSearch text:[%s]",search_text);
                    MessageBox(hwnd,msg_text,"ListGrid",MB_OK);
                }
            }
        } break;
        case WM_DESTROY: {
            PostQuitMessage(0);
            return (0);
        } break;
    }
    return DefWindowProc(hwnd,msg,wParam,lParam);
}