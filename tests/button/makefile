##------------------------------------------------------##
##     button test complier.                            ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

all: button_test.exe

button_test.exe: button_test.obj
    link button_test.obj /out:button_test.exe
    @if exist button_test @ECHO.button_test���ɳɹ�!
button_test.obj: button_test.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     button test complier.                            ##
    @ECHO.##------------------------------------------------------##
    @ECHO.���� button ���Գ��� && @ECHO. && @ECHO.
    $(CC) -c button_test.cpp /link:$(CFLAGS) /Fo:button_test.obj 
    
clean: all
    @DEL -S -Q button_test.obj
    @DEL -S -Q button_test.exe

test: all
    @if exist button_test.exe @start button_test.exe && ECHO.�������Գ���...
    