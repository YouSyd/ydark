##------------------------------------------------------##
##     ListView test complier.                          ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

all: Listview_2.exe
    @DEL -S -Q *.obj

Listview_2.exe: *.obj ListView.res
    link Listview_2.obj ListView.res /out:ListView_2.exe
    @if exist ListView_2.exe @ECHO.ListView_2 生成成功!

ListView.res: res/ListView.rc
    @CD res
    RC ListView.rc
    @move ListView.res ../ListView.res
    @CD ..
    @if exist ListView.res @ECHO.Res generated.
    
*.obj: *.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     ListView   test complier.                        ##
    @ECHO.##------------------------------------------------------##
    @ECHO.编译 ListView 测试程序 && @ECHO. && @ECHO.
    $(CC) -c *.cpp /link:$(CFLAGS) /Fo:*.obj 
    
clean: all
    @if exist *.obj @DEL -S -Q *.obj
    @if exist *.exe @DEL -S -Q *.exe
    @if exist *.res @DEL -S -Q *.res

test: all
    @if exist Listview_2.exe @start Listview_2.exe && ECHO.启动测试程序...
    