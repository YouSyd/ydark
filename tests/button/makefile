##------------------------------------------------------##
##     button test complier.                            ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

all: button_test.exe

button_test.exe: button_test.obj
    link button_test.obj /out:button_test.exe
    @if exist button_test @ECHO.button_test生成成功!
button_test.obj: button_test.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     button test complier.                            ##
    @ECHO.##------------------------------------------------------##
    @ECHO.编译 button 测试程序 && @ECHO. && @ECHO.
    $(CC) -c button_test.cpp /link:$(CFLAGS) /Fo:button_test.obj 
    
clean: all
    @DEL -S -Q button_test.obj
    @DEL -S -Q button_test.exe

test: all
    @if exist button_test.exe @start button_test.exe && ECHO.启动测试程序...
    