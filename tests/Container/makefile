##------------------------------------------------------##
##     Container test complier.                         ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib
LIBPATH = ../../lib/

all: Container_Test.exe
    @DEL -S -Q *.obj
    @DEL -S -Q *.res

Container_Test.exe: Container_Test.obj Container.res
    link Container_Test.obj container.res /out:Container_Test.exe /LIBPATH:$(LIBPATH)
    @if exist Container_Test.exe @ECHO.Container_Test 生成成功!
    
Container_Test.obj: Container_Test.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     Container test complier.                         ##
    @ECHO.##------------------------------------------------------##
    @ECHO.编译 Container 测试程序 && @ECHO. && @ECHO.
    $(CC) -c Container_Test.cpp /link:$(CFLAGS) /Fo:Container_Test.obj

Container.res: res/Container.rc
    @CD res
    RC Container.rc
    @move Container.res ../Container.res
    @CD ..
    @if exist Container.res @ECHO.Res generated.
      
clean: all
    @if exist *.obj @DEL -S -Q *.obj
    @if exist *.exe @DEL -S -Q *.exe
    @if exist *.res @DEL -S -Q *.res

test: all
    @if exist Container_Test.exe @start Container_Test.exe && ECHO.启动测试程序...   