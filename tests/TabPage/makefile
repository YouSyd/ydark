##------------------------------------------------------##
##     TabPage test complier.                          ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

LIBPATH = ../../lib/

all: TabPage_Test2.exe
    @DEL -S -Q *.obj

TabPage_Test2.exe: *.obj TabPage.res
    link TabPage_Test2.obj TabPage.res /out:TabPage_Test2.exe /LIBPATH:$(LIBPATH)
    @if exist TabPage_Test2.exe @ECHO.TabPage_Test2 ���ɳɹ�!

TabPage.res: res/TabPage.rc
    @CD res
    RC TabPage.rc
    @move TabPage.res ../TabPage.res
    @CD ..
    @if exist TabPage.res @ECHO.Res generated.
    
*.obj: *.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     TabPage test complier.                           ##
    @ECHO.##------------------------------------------------------##
    @ECHO.���� TabPage ���Գ��� && @ECHO. && @ECHO.
    $(CC) -c *.cpp /link:$(CFLAGS) /Fo:*.obj 
    
clean: all
    @if exist *.obj @DEL -S -Q *.obj
    @if exist *.exe @DEL -S -Q *.exe
    @if exist *.res @DEL -S -Q *.res

test: all
    @if exist TabPage_test2.exe @start TabPage_Test2.exe "left bar" && ECHO.�������Գ���...
    @if exist TabPage_test2.exe @start TabPage_Test2.exe "top bar" && ECHO.�������Գ���...
    