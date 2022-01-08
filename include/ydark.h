#pragma once
#include<stdarg.h>
#include<windows.h>
#include<windowsx.h>
#include<math.h>
#include<stdio.h>
#include<commctrl.h>
#include<Richedit.h>
#include<gdiplus.h>
using namespace Gdiplus; //not use yet.
 
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"gdiplus.lib")

typedef struct _RWINDOW_STYLE_ {
    int icon_margin;
    int icon_pixls;
    int ncbtn_margin;
    int ncbtn_cx;
    int ncbtn_cy;
    int menubar_cy;
    int border_pixls;
    int menubar_margin;
    int scroll_pixls;
    int min_thumb_pixls;
}RMetrics,*pMetrics;

typedef enum FRAME_AREA_TYPE {
    ZCLOSE,
    ZMAX,
    ZMIN,
    ZICON,
    ZMENUBAR,
    ZCAPTION,
    ZTEXTTITLE,
    ZCONFIG,
    ZMSG,
    ZHSCROLL,
    ZHSTHUMB,
    ZVSCROLL,
    ZVSTHUMB,
    ZLVTITILE,
    ZNOCARE
} EFNCZone;
