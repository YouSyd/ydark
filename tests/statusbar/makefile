##------------------------------------------------------##
##     StatusBar  test complier.                          ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

all: StatusBar_test.exe
    @DEL -S -Q *.obj

StatusBar_Test.exe: StatusBar_Test.obj
    link StatusBar_Test.obj /out:StatusBar_Test.exe
    @if exist StatusBar_test.exe @ECHO.StatusBar_test ���ɳɹ�!

*.obj: *.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     StatusBar test complier.                           ##
    @ECHO.##------------------------------------------------------##
    @ECHO.���� StatusBar ���Գ��� && @ECHO. && @ECHO.
    $(CC) -c *.cpp /link:$(CFLAGS) /Fo:*.obj 
    
clean: all
    @if exist *.obj @DEL -S -Q *.obj
    @if exist *.exe @DEL -S -Q *.exe

test: all
    @if exist StatusBar_Test.exe @start StatusBar_Test.exe && ECHO.�������Գ���...
    