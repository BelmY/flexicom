DEBUG=0
CMDLINE=1

###############################

CC=cl
LINK=link
RC=rc
MT=mt
LIBT=lib
RM=@del /F /Q
COPY=@copy /Y
MOC=C:\Qt\4.8.1\bin\moc

## Script for detecting W64 system or not. It sets IS64 to 1 if is x64 or 0 otherwise
!if [tools\Is64.cmd >Makefile.auto]
!error *** Could not create makefile.auto
!endif
!include makefile.auto
!if $(IS64) == 1
PREF=w64
!else
PREF=w32
!endif

## QT
QT_INC_DIR=C:\Qt\4.8.1\include
QT_LIB_DIR=C:\Qt\4.8.1\lib
## QWT
QWT_INC_DIR=C:\CTTC\qwt\src
QWT_LIB_DIR=C:\CTTC\qwt\lib\$(PREF)
## Boost
BOOST_INC_DIR=C:\CTTC\boost_1_44_0
BOOST_LIB_DIR=C:\CTTC\boost_1_44_0\stage\lib
## Gnuradio
GR_INC_DIR=C:\cygwin\home\Pol\gnuradio\gnuradio\include
GR_LIB_DIR=C:\cygwin\home\Pol\gnuradio\gnuradio\lib
## UHD
UHD_INC_DIR=C:\CTTC\UHD\include
UHD_LIB_DIR=C:\CTTC\UHD\lib

###############################

OUT_DIR=out
OBJ_DIR=$(OUT_DIR)
SRC_DIR=src
INC_DIR=include
LAYOUT_DIR=layouts
LAYOUT_INC=/I $(LAYOUT_DIR)/80211b /I $(LAYOUT_DIR)/VLC
TARGET=flexicom

QT_CORE_DIR=$(QT_INC_DIR)\QtCore
QT_GUI_DIR=$(QT_INC_DIR)\QtGui
QT_INC=/I $(QT_CORE_DIR) /I $(QT_GUI_DIR) /I $(QT_INC_DIR)
!if $(DEBUG) == 1
QT_LIB=/LIBPATH:$(QT_LIB_DIR) QtCore4.lib QtGui4.lib 
!else
QT_LIB=/LIBPATH:$(QT_LIB_DIR) QtCore4.lib QtGui4.lib 
!endif
EIGEN_INC=/I $(EIGEN_INC_DIR)
QWT_INC=/I $(QWT_INC_DIR)
!if $(DEBUG) == 1
QWT_LIB=/LIBPATH:$(QWT_LIB_DIR) qwtd.lib
!else
QWT_LIB=/LIBPATH:$(QWT_LIB_DIR) qwt.lib
!endif
!if $(CMDLINE) == 0
CMDLINE_LFLAG=/subsystem:windows
!else
CMDLINE_CFLAG=/D CMDLINE
!endif
BOOST_INC=/I $(BOOST_INC_DIR)
BOOST_LIB=/LIBPATH:$(BOOST_LIB_DIR)
GR_INC=/I $(GR_INC_DIR) /I $(GR_INC_DIR)/gnuradio
GR_LIB=/LIBPATH:$(GR_LIB_DIR) gnuradio-core.lib gnuradio-uhd.lib
UHD_INC=/I $(UHD_INC_DIR)
UHD_LIB=/LIBPATH:$(UHD_LIB_DIR) uhd.lib

INC_FILES = /I $(INC_DIR) $(QT_INC) $(QWT_INC) $(BOOST_INC) $(LAYOUT_INC) $(GR_INC) $(UHD_INC)
EXECFLAGS=$(DBGCFLAG) $(CFLAGS) $(INC_FILES) /Fo$(OBJ_DIR)/
CFLAGS=/J /D NOMINMAX /D _WIN /D _WINDOWS /D _CRT_NONSTDC_NO_DEPRECATE /D _CRT_SECURE_NO_DEPRECATE /nologo /EHsc /c $(CMDLINE_CFLAG) 
#user32.lib kernel32.lib ws2_32.lib winspool.lib shell32.lib gdi32.lib imm32.lib winmm.lib advapi32.lib
LFLAGS=$(DBGLFLAG) /LIBPATH:$(OBJ_DIR) user32.lib kernel32.lib /nologo $(CMDLINE_LFLAG) $(QT_LIB) $(QWT_LIB) $(BOOST_LIB) $(GR_LIB) $(UHD_LIB) \
	/INCREMENTAL:NO /MANIFEST 

!if $(IS64) == 1
CFLAGS = $(CFLAGS) /D _WIN64
LFLAGS = $(LFLAGS) /MACHINE:X64
!else
CFLAGS = $(CFLAGS) /D _WIN32 /arch:SSE2 /D _USE_32BIT_TIME_T 
!endif

!if $(DEBUG) == 1
DBGCFLAG=/MDd /Zi /Od
DBGLFLAG=/debug
!else
DBGCFLAG=/MD /Zi /Ox /W1
DBGLFLAG=/debug
!endif

OBJ_FILES=$(OBJ_DIR)/MainWindow.obj $(OBJ_DIR)/MainWindow_moc.obj $(OBJ_DIR)/LayoutFactory.obj

LAYOUTS=$(OBJ_DIR)/Layout80211b.obj $(OBJ_DIR)/LayoutVLC.obj $(OBJ_DIR)/Rx80211b.obj $(OBJ_DIR)/BBN_Slicer.obj \
		$(OBJ_DIR)/BBN_DPSKDemod.obj $(OBJ_DIR)/BBN_PLCP.obj

all: exe
	$(RM) Makefile.auto
	$(MT) /nologo -outputresource:"$(TARGET).exe;1" -manifest $(TARGET).exe.manifest
	$(RM) $(TARGET).exe.manifest $(TARGET).map $(TARGET).exp
	
exe: objs $(OBJ_DIR)/main.obj
	$(LINK) $(LFLAGS) $(OBJ_FILES) $(LAYOUTS) $(OBJ_DIR)/main.obj /MAP /OUT:$(TARGET).exe
	
objs: $(OBJ_FILES) $(LAYOUTS)

clean:
	$(RM) "$(OBJ_DIR)\*.obj" 
	
{$(SRC_DIR)}.cc{$(OBJ_DIR)}.obj:
	@if not exist $(OUT_DIR) mkdir $(OUT_DIR)
	$(CC) $(EXECFLAGS) /Fd$(OBJ_DIR) $<
	
$(OBJ_DIR)/MainWindow_moc.obj: $(INC_DIR)/MainWindow.h
	$(MOC) $(INC_DIR)/MainWindow.h -o $(SRC_DIR)/MainWindow_moc.cc
	$(CC) $(EXECFLAGS) /Fo$(OBJ_DIR)/ /Fd$(OBJ_DIR) $(SRC_DIR)/MainWindow_moc.cc

#Layouts
{layouts/80211b}.cc{$(OBJ_DIR)}.obj:
	$(CC) $(EXECFLAGS) /Fd$(OBJ_DIR) $<
	
{layouts/VLC}.cc{$(OBJ_DIR)}.obj:
	$(CC) $(EXECFLAGS) /Fd$(OBJ_DIR) $<