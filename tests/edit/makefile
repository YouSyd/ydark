##------------------------------------------------------##
##     edit test complier.                          ##
##------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

all: edit_test.exe edit_alter.exe Edit_single_test.exe  Edit_test2.exe
    @DEL -S -Q *.obj

edit_test.exe: *.obj
    link edit_test.obj /out:edit_test.exe
    @if exist edit_test.exe @ECHO.edit_test 生成成功!

edit_alter.exe: *.obj
    link edit_alter.obj /out:edit_alter.exe
    @if exist edit_alter.exe @ECHO.edit_alter 生成成功!

Edit_single_test.exe: *.obj
    link Edit_single_test.obj /out:Edit_single_test.exe
    @if exist Edit_single_test.exe @ECHO.Edit_single_test 生成成功!

Edit_test2.exe: *.obj
    link Edit_test2.obj /out:Edit_test2.exe
    @if exist Edit_test2.exe @ECHO.Edit_test2 生成成功!

*.obj: *.cpp
    @ECHO.##------------------------------------------------------##
    @ECHO.##     edit test complier.                            ##
    @ECHO.##------------------------------------------------------##
    @ECHO.编译 edit 测试程序 && @ECHO. && @ECHO.
    $(CC) -c *.cpp /link:$(CFLAGS) /Fo:*.obj 
    
clean: all
    @if exist *.obj @DEL -S -Q *.obj
    @if exist *.exe @DEL -S -Q *.exe

test: all
    @if exist edit_test.exe @start edit_test.exe && ECHO.启动测试程序...
    @if exist edit_alter.exe @start edit_alter.exe && ECHO.启动测试程序...
    @if exist Edit_single_test.exe @start Edit_single_test.exe && ECHO.启动测试程序...
    @if exist Edit_test2.exe @start Edit_test2.exe && ECHO.启动测试程序...
    