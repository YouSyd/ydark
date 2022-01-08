##------------------------------------------------------------##
##
##  ctrls makefile.
##
##------------------------------------------------------------##
## 编译组件测试
!MESSAGE 编译组件测试...

TESTDIR = tests
SRCD = src

all: 
    @CD $(SRCD)
    @IF EXIST makefile nmake && @ECHO.编译控件...
    @CD ..
    @ECHO.-----------------------------------------------------
    @ECHO......................................................
    @CD $(MKDIR)
    @IF NOT EXIST $(TESTDIR) @ECHO.未找到测试目录$(TESTDIR)
    @ECHO.当前编译目录 ---------- $(MKDIR)
    for /D /R $(TESTDIR) %f in (*) do @if exist %f/makefile @CD %f&&nmake 
    @CD $(MKDIR)
    @ECHO.....
    @ECHO. 编译结束
    @ECHO.-----------------------------------------------------
    
clean:
    @ECHO.-----------------------------------------------------
    @ECHO......................................................
    @CD $(MKDIR)
    @IF NOT EXIST $(TESTDIR) @ECHO.未找到测试目录$(TESTDIR)
    @ECHO.当前编译目录 ---------- $(MKDIR)
    for /D /R $(TESTDIR) %f in (*) do @if exist %f/makefile @CD %f&&nmake clean 
    @CD $(MKDIR)
    @ECHO.....
    @ECHO. 编译结束
    @ECHO.-----------------------------------------------------
    
test: all
    @CD $(MKDIR)
    @IF NOT EXIST $(TESTDIR) @ECHO.未找到测试目录$(TESTDIR)
    @ECHO.当前编译目录 ---------- $(MKDIR)
    for /D /R $(TESTDIR) %f in (*) do @if exist %f/makefile @CD %f&&nmake test
    @CD $(MKDIR)
    @ECHO.....
    @ECHO. 测试结束
    @ECHO.-----------------------------------------------------