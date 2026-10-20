# Microsoft Developer Studio Generated NMAKE File, Based on hour7.dsp
!IF "$(CFG)" == ""
CFG=hour7 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to hour7 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "hour7 - Win32 Release" && "$(CFG)" != "hour7 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "hour7.mak" CFG="hour7 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "hour7 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "hour7 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "hour7 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\hour7.exe"

!ELSE 

ALL : "$(OUTDIR)\hour7.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\hour7.obj"
	-@erase "$(INTDIR)\hour7.pch"
	-@erase "$(INTDIR)\MMIO.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\Wave.obj"
	-@erase "$(OUTDIR)\hour7.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)\hour7.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\hour7.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ddraw.lib dsound.lib dxguid.lib winmm.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\hour7.pdb" /machine:I386\
 /out:"$(OUTDIR)\hour7.exe" 
LINK32_OBJS= \
	"$(INTDIR)\hour7.obj" \
	"$(INTDIR)\MMIO.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Wave.obj"

"$(OUTDIR)\hour7.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "hour7 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\hour7.exe"

!ELSE 

ALL : "$(OUTDIR)\hour7.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\hour7.obj"
	-@erase "$(INTDIR)\hour7.pch"
	-@erase "$(INTDIR)\MMIO.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\Wave.obj"
	-@erase "$(OUTDIR)\hour7.exe"
	-@erase "$(OUTDIR)\hour7.ilk"
	-@erase "$(OUTDIR)\hour7.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)\hour7.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\hour7.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ddraw.lib dsound.lib dxguid.lib winmm.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\hour7.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)\hour7.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\hour7.obj" \
	"$(INTDIR)\MMIO.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Wave.obj"

"$(OUTDIR)\hour7.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(CFG)" == "hour7 - Win32 Release" || "$(CFG)" == "hour7 - Win32 Debug"
SOURCE=.\hour7.cpp

!IF  "$(CFG)" == "hour7 - Win32 Release"

DEP_CPP_HOUR7=\
	".\bitmap.h"\
	".\inits.h"\
	".\input.h"\
	".\msgSystem.h"\
	".\presets.h"\
	".\render.h"\
	".\scriptio.h"\
	".\sound.h"\
	".\statemanager.h"\
	".\StdAfx.h"\
	".\surface.h"\
	".\toolkit.h"\
	".\Wave.h"\
	{$(INCLUDE)}"dls1.h"\
	{$(INCLUDE)}"dmdls.h"\
	{$(INCLUDE)}"dmerror.h"\
	{$(INCLUDE)}"dmusbuff.h"\
	{$(INCLUDE)}"dmusicc.h"\
	{$(INCLUDE)}"dmusici.h"\
	

"$(INTDIR)\hour7.obj" : $(SOURCE) $(DEP_CPP_HOUR7) "$(INTDIR)"\
 "$(INTDIR)\hour7.pch"


!ELSEIF  "$(CFG)" == "hour7 - Win32 Debug"

DEP_CPP_HOUR7=\
	".\bitmap.h"\
	".\inits.h"\
	".\input.h"\
	".\msgSystem.h"\
	".\presets.h"\
	".\render.h"\
	".\scriptio.h"\
	".\sound.h"\
	".\statemanager.h"\
	".\surface.h"\
	".\toolkit.h"\
	".\Wave.h"\
	{$(INCLUDE)}"dls1.h"\
	{$(INCLUDE)}"dmdls.h"\
	{$(INCLUDE)}"dmerror.h"\
	{$(INCLUDE)}"dmusbuff.h"\
	{$(INCLUDE)}"dmusicc.h"\
	{$(INCLUDE)}"dmusici.h"\
	

"$(INTDIR)\hour7.obj" : $(SOURCE) $(DEP_CPP_HOUR7) "$(INTDIR)"\
 "$(INTDIR)\hour7.pch"


!ENDIF 

SOURCE=.\MMIO.cpp

!IF  "$(CFG)" == "hour7 - Win32 Release"

DEP_CPP_MMIO_=\
	".\MMIO.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\MMIO.obj" : $(SOURCE) $(DEP_CPP_MMIO_) "$(INTDIR)"\
 "$(INTDIR)\hour7.pch"


!ELSEIF  "$(CFG)" == "hour7 - Win32 Debug"

DEP_CPP_MMIO_=\
	".\MMIO.h"\
	

"$(INTDIR)\MMIO.obj" : $(SOURCE) $(DEP_CPP_MMIO_) "$(INTDIR)"\
 "$(INTDIR)\hour7.pch"


!ENDIF 

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "hour7 - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)\hour7.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\hour7.pch" : $(SOURCE) $(DEP_CPP_STDAF)\
 "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "hour7 - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)\hour7.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\hour7.pch" : $(SOURCE) $(DEP_CPP_STDAF)\
 "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Wave.cpp

!IF  "$(CFG)" == "hour7 - Win32 Release"

DEP_CPP_WAVE_=\
	".\MMIO.h"\
	".\StdAfx.h"\
	".\Wave.h"\
	

"$(INTDIR)\Wave.obj" : $(SOURCE) $(DEP_CPP_WAVE_) "$(INTDIR)"\
 "$(INTDIR)\hour7.pch"


!ELSEIF  "$(CFG)" == "hour7 - Win32 Debug"

DEP_CPP_WAVE_=\
	".\MMIO.h"\
	".\Wave.h"\
	

"$(INTDIR)\Wave.obj" : $(SOURCE) $(DEP_CPP_WAVE_) "$(INTDIR)"\
 "$(INTDIR)\hour7.pch"


!ENDIF 


!ENDIF 

