##------------------------------------------------------------##
# 
#   controls src.
#           button / edit / head/ listbox / listview(report_icon)
#           listreport(report listview ICON)
#           tabpage
#           container( modified panel for sub controls)
#           TreeGrid(combination for treeview with title columns.)
#           datepick
#           statusbar
#
##------------------------------------------------------------##
CC = cl
CFLAGS = user32.lib gdi32.lib kernel32.lib gdiplus.lib

SRCD = .
SRCF = *.cpp
OBJF = *.obj

LIBD = ../lib
LIBF = *.lib

all: dirs \
    objs \
    libs 
    
dirs: 
    @CD ..
    @if not exist lib MKDIR lib && ECHO.lib folder generated.
    @CD src

objs:   $(SRCF)
    $(CC) -c $(SRCF) /link $(CFLAGS)
    
libs: $(OBJF)
    @if exist button.obj lib button.obj /out:$(LIBD)/button.lib
    @if exist $(LIBD)/button.lib ECHO.Button lib generate.
    @if exist edit.obj lib edit.obj /out:$(LIBD)/edit.lib
    @if exist $(LIBD)/edit.lib ECHO.Edit lib generate.
    @if exist head.obj lib head.obj /out:$(LIBD)/head.lib
    @if exist $(LIBD)/head.lib ECHO.Head lib generate.
    @if exist listbox.obj lib listbox.obj /out:$(LIBD)/listbox.lib
    @if exist $(LIBD)/listbox.lib ECHO.ListBox lib generate.
    @if exist listview.obj lib listview.obj /out:$(LIBD)/listview.lib
    @if exist $(LIBD)/listview.lib ECHO.ListView lib generate.
    @if exist listreport.obj lib listreport.obj /out:$(LIBD)/listreport.lib
    @if exist $(LIBD)/listreport.lib ECHO.ListReport lib generate.
    @if exist tabpage.obj lib tabpage.obj /out:$(LIBD)/tabpage.lib
    @if exist $(LIBD)/tabpage.lib ECHO.TabPage lib generate.
    @if exist container.obj lib container.obj /out:$(LIBD)/container.lib
    @if exist $(LIBD)/container.lib ECHO.Container lib generate.
    @if exist TreeGrid.obj lib TreeGrid.obj /out:$(LIBD)/TreeGrid.lib
    @if exist $(LIBD)/TreeGrid.lib ECHO.TreeGrid lib generate.
    @if exist datepick.obj lib datepick.obj /out:$(LIBD)/datepick.lib
    @if exist $(LIBD)/datepick.lib ECHO.DatePick lib generate.
    @if exist statusbar.obj lib statusbar.obj /out:$(LIBD)/statusbar.lib
    @if exist $(LIBD)/statusbar.lib ECHO.StatusBar lib generate.
    @DEL /Q /S $(OBJF) && ECHO.清理目标文件，编译结束
    @ECHO. 生成lib文件列表如下：
    @CD ../lib
    @FOR %%i in (*.lib) DO @ECHO. %%i
    @CD ../src
    
clean: all
    @if exist $(OBJF) DEL /Q /S $(OBJF)
    @CD ..
    @RMDIR /Q /S lib
    @CD src
     