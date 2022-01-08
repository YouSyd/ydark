##------------------------------------------------------##
##     listbox  test complier.                          ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

all: listbox_test.exe
    @DEL -S -Q *.obj

listbox_test.exe: listbox_test.obj
    link listbox_test.obj /out:listbox_test.exe
    @if exist listbox_test.exe @ECHO.listbox_test 生成成功!

*.obj: *.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     listbox test complier.                           ##
    @ECHO.##------------------------------------------------------##
    @ECHO.编译 ListBox 测试程序 && @ECHO. && @ECHO.
    $(CC) -c *.cpp /link:$(CFLAGS) /Fo:*.obj 
    
clean: all
    @if exist *.obj @DEL -S -Q *.obj
    @if exist *.exe @DEL -S -Q *.exe

test: all
    @if exist listbox_test.exe @start listbox_test.exe && ECHO.启动测试程序...
    