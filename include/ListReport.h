#pragma once
#include "ydark.h"

typedef struct _STRUCT_LISTREPORTINFO_ {
    WNDPROC proc;
    WNDPROC pre_proc;
    
    int margin_border;
    COLORREF color_border;
    
    int index_hover;
}ReportStyle,*pReportStyle;

int Report_InitialSettings(HWND hwnd);
pReportStyle Report_GetSettings(HWND hwnd);
int Report_ClearSettings(HWND hwnd);
int Report_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Report_MoveItems(HWND hwnd);
LRESULT CALLBACK ListReportOwnProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);