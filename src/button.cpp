#include "../include/button.h"

int Button_InitialSettings(HWND hwnd)
{
    pButtonStyle bs=(pButtonStyle)calloc(sizeof(RButtonStyle),1);
    if(!bs) return NULL;
    
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)bs);   
    
    return 0;
}

inline pButtonStyle Button_GetSettings(HWND hwnd)
{
    return (pButtonStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int Button_ClearSettings(HWND hwnd)
{
    pButtonStyle bs=Button_GetSettings(hwnd);
    if(!bs) return -1;
    
    SetWindowLongPtr(hwnd,GWLP_USERDATA,NULL);
    
    free(bs);
    return 0;
}

LRESULT CALLBACK BtnProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    pButtonStyle bs=Button_GetSettings(hwnd);
    if(!bs) return 0;
            
    switch(message){
    case WM_NCCALCSIZE: {
    } break;
    case WM_LBUTTONDOWN: {
    
    } break;
    case WM_PAINT: {
    
    } break;
    case WM_SETCURSOR: {
    
    } break;
    case WM_NCDESTROY: {
    
    } break;
    }
    
    return CallWindowProc(bs->pre_proc,hwnd,message,wParam,lParam);
}