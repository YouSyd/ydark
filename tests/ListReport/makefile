##------------------------------------------------------##
##     ListReport test complier.                        ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

all: GroupView_Test.exe ListReport_test.exe ListReport_test2.exe
    @DEL -S -Q *.obj

GroupView_Test.exe: *.obj report.res
    link GroupView_Test.obj report.res /out:GroupView_Test.exe
    @if exist GroupView_Test.exe @ECHO.GroupView_Test 生成成功!

ListReport_test.exe: *.obj report.res
    link ListReport_test.obj report.res /out:ListReport_test.exe
    @if exist ListReport_test.exe @ECHO.ListReport_test 生成成功!

ListReport_test2.exe: *.obj report.res
    link ListReport_test2.obj report.res /out:ListReport_test2.exe
    @if exist ListReport_test2.exe @ECHO.ListReport_test2 生成成功!

report.res: res/report.rc
    @CD res
    RC report.rc
    @move report.res ../report.res
    @CD ..
    @if exist report.res @ECHO.Res generated.
    
*.obj: *.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     ListReport test complier.                        ##
    @ECHO.##------------------------------------------------------##
    @ECHO.编译 edit 测试程序 && @ECHO. && @ECHO.
    $(CC) -c *.cpp /link:$(CFLAGS) /Fo:*.obj 
    
clean: all
    @if exist *.obj @DEL -S -Q *.obj
    @if exist *.exe @DEL -S -Q *.exe
    @if exist *.res @DEL -S -Q *.res

test: all
    @if exist GroupView_Test.exe @start GroupView_Test.exe && ECHO.启动测试程序...
    @if exist ListReport_test.exe @start ListReport_test.exe && ECHO.启动测试程序...
    @if exist ListReport_test2.exe @start ListReport_test2.exe && ECHO.启动测试程序...
    