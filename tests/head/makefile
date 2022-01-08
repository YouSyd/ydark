##------------------------------------------------------##
##     head test complier.                              ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

all: header_test.exe

header_test.exe: header_test.obj
    link header_test.obj /out:header_test.exe
    @if exist header_test.exe @ECHO.Header_Test生成成功!
header_test.obj: header_test.cpp
    @ECHO.编译Header测试程序 && @ECHO. && @ECHO.
    $(CC) -c header_test.cpp /link:$(CFLAGS) /Fo:header_test.obj 
    
clean: all
    @DEL -S -Q header_test.obj
    @DEL -S -Q header_test.exe

test: all
    @if exist header_test.exe @start header_test.exe && ECHO.启动测试程序...
    