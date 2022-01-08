##------------------------------------------------------##
##     TreeGrid test complier.                          ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib
LIBPATH = ../../lib/

all: TreeGrid_Test.exe
    @DEL -S -Q *.obj

TreeGrid_Test.exe: TreeGrid_Test.obj
    link TreeGrid_Test.obj /out:TreeGrid_Test.exe /LIBPATH:$(LIBPATH)
    @if exist TreeGrid_Test.exe @ECHO.TreeGrid_Test 生成成功!
    
TreeGrid_Test.obj: TreeGrid_Test.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     TreeGrid test complier.                          ##
    @ECHO.##------------------------------------------------------##
    @ECHO.编译 TreeGrid 测试程序 && @ECHO. && @ECHO.
    $(CC) -c TreeGrid_Test.cpp /link:$(CFLAGS) /Fo:TreeGrid_Test.obj /D"UNITTEST_TREEGRID"
    
clean: all
    @if exist *.obj @DEL -S -Q *.obj
    @if exist *.exe @DEL -S -Q *.exe
    @if exist *.res @DEL -S -Q *.res

test: all
    @if exist TreeGrid_Test.exe @start TreeGrid_Test.exe && ECHO.启动测试程序...   