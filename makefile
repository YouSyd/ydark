##------------------------------------------------------------##
##
##  ctrls makefile.
##
##------------------------------------------------------------##
## �����������
!MESSAGE �����������...

TESTDIR = tests
SRCD = src

all: 
    @CD $(SRCD)
    @IF EXIST makefile nmake && @ECHO.����ؼ�...
    @CD ..
    @ECHO.-----------------------------------------------------
    @ECHO......................................................
    @CD $(MKDIR)
    @IF NOT EXIST $(TESTDIR) @ECHO.δ�ҵ�����Ŀ¼$(TESTDIR)
    @ECHO.��ǰ����Ŀ¼ ---------- $(MKDIR)
    for /D /R $(TESTDIR) %f in (*) do @if exist %f/makefile @CD %f&&nmake 
    @CD $(MKDIR)
    @ECHO.....
    @ECHO. �������
    @ECHO.-----------------------------------------------------
    
clean:
    @ECHO.-----------------------------------------------------
    @ECHO......................................................
    @CD $(MKDIR)
    @IF NOT EXIST $(TESTDIR) @ECHO.δ�ҵ�����Ŀ¼$(TESTDIR)
    @ECHO.��ǰ����Ŀ¼ ---------- $(MKDIR)
    for /D /R $(TESTDIR) %f in (*) do @if exist %f/makefile @CD %f&&nmake clean 
    @CD $(MKDIR)
    @ECHO.....
    @ECHO. �������
    @ECHO.-----------------------------------------------------
    
test: all
    @CD $(MKDIR)
    @IF NOT EXIST $(TESTDIR) @ECHO.δ�ҵ�����Ŀ¼$(TESTDIR)
    @ECHO.��ǰ����Ŀ¼ ---------- $(MKDIR)
    for /D /R $(TESTDIR) %f in (*) do @if exist %f/makefile @CD %f&&nmake test
    @CD $(MKDIR)
    @ECHO.....
    @ECHO. ���Խ���
    @ECHO.-----------------------------------------------------