#include "ydark.h"

typedef struct _RSTRUCT_BUTTONINFO_ {
    WNDPROC pre_proc;
    WNDPROC proc;
    
typedef int *pBtnProc(HWND,int*) ;
    pBtnProc click;
}RButtonStyle,*pButtonStyle;

int Button_InitialSettings(HWND hwnd);
inline pButtonStyle Button_GetSettings(HWND hwnd);
int Button_ClearSettings(HWND hwnd);