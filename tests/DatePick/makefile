##------------------------------------------------------##
##     datepick test complier.                          ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

all: datepick_test.exe

datepick_test.exe: datepick_test.obj
    link datepick_test.obj /out:datepick_test.exe
    @if exist datepick_test.exe @ECHO.datepick_test 生成成功!
datepick_test.obj: datepick_test.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     datepick test complier.                            ##
    @ECHO.##------------------------------------------------------##
    @ECHO.编译 datepick 测试程序 && @ECHO. && @ECHO.
    $(CC) -c datepick_test.cpp /link:$(CFLAGS) /Fo:datepick_test.obj 
    
clean: all
    @DEL -S -Q datepick_test.obj
    @DEL -S -Q datepick_test.exe

test: all
    @if exist datepick_test.exe @start datepick_test.exe && ECHO.启动测试程序...
    