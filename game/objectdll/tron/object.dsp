# Microsoft Developer Studio Project File - Name="Object" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Object - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Object.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Object.mak" CFG="Object - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Object - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Object - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Object"
# PROP Scc_LocalPath "..\..\.."
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Built\Release\ObjectDLL\TRON"
# PROP Intermediate_Dir "..\..\Built\Release\ObjectDLL\TRON"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\..\Shared\Tron" /I ".\\" /I "..\ObjectShared" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\..\shared" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\..\libs\butemgr" /I "..\..\..\libs\cryptmgr" /I "..\..\..\libs\lith" /I "..\..\..\libs\mfcstub" /I "..\..\..\libs\regmgr" /I "..\..\libs" /D "NDEBUG" /D "_NOLFBUILD" /D "USE_INTEL_COMPILER" /D "WIN32" /D "_WINDOWS" /D "NOPS2" /D "_SERVERBUILD" /Yu"stdafx.h" /FD /Zm1000 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\Built\Release\ObjectDLL\TRON\Object.lto"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
TargetPath=\proj\TRON\Game\Built\Release\ObjectDLL\TRON\Object.lto
InputPath=\proj\TRON\Game\Built\Release\ObjectDLL\TRON\Object.lto
SOURCE="$(InputPath)"

"..\..\..\development\to2\game\object.lto" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo copy $(TargetPath) ..\..\..\development\to2\game 
	copy $(TargetPath) ..\..\..\development\to2\game 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Built\Debug\ObjectDLL\TRON"
# PROP Intermediate_Dir "..\..\Built\Debug\ObjectDLL\TRON"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".\\" /I "..\ObjectShared" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\..\shared" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\..\libs\butemgr" /I "..\..\..\libs\cryptmgr" /I "..\..\..\libs\lith" /I "..\..\..\libs\mfcstub" /I "..\..\..\libs\regmgr" /I "..\..\libs" /I "..\..\Shared\Tron" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "NOPS2" /D "_SERVERBUILD" /FR /Yu"stdafx.h" /FD /Zm1000 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\Built\Debug\ObjectDLL\TRON\Object.lto" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /map
# Begin Custom Build
TargetPath=\proj\TRON\Game\Built\Debug\ObjectDLL\TRON\Object.lto
InputPath=\proj\TRON\Game\Built\Debug\ObjectDLL\TRON\Object.lto
SOURCE="$(InputPath)"

"..\..\..\development\to2\game\object.lto" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo copy $(TargetPath) ..\..\..\development\to2\game 
	copy $(TargetPath) ..\..\..\development\to2\game 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "Object - Win32 Release"
# Name "Object - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c"
# Begin Source File

SOURCE=.\BinaryBit.cpp
# End Source File
# Begin Source File

SOURCE=.\CoreDump.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalsInit.cpp
# End Source File
# Begin Source File

SOURCE=.\LightCycleMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\PatchRoutine.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectileClusterAutoBurstDisc.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectileClusterDisc.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectileDisc.cpp
# End Source File
# Begin Source File

SOURCE=.\Stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TronGameServerShell.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shared\TRON\TronMissionButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\TronPlayerObj.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shared\Tron\TronVersionMgr.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp"
# Begin Source File

SOURCE=.\BinaryBit.h
# End Source File
# Begin Source File

SOURCE=.\CoreDump.h
# End Source File
# Begin Source File

SOURCE=.\LightCycleMgr.h
# End Source File
# Begin Source File

SOURCE=.\PatchRoutine.h
# End Source File
# Begin Source File

SOURCE=.\ProjectileClusterAutoBurstDisc.h
# End Source File
# Begin Source File

SOURCE=.\ProjectileClusterDisc.h
# End Source File
# Begin Source File

SOURCE=.\ProjectileDisc.h
# End Source File
# Begin Source File

SOURCE=.\Stdafx.h
# End Source File
# Begin Source File

SOURCE=.\TronGameServerShell.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\TRON\TronMissionButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\TronPlayerObj.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\Tron\TronVersionMgr.h
# End Source File
# End Group
# Begin Group "libs_debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\built\Debug\ObjectDLL\ObjectShared\ObjectShared.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\ButeMgr\ButeMgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\MFCStub\MFCStub.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\CryptMgr\cryptmgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\Lib_StdLith\Lib_StdLith.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\Lib_Lith\Lib_Lith.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\RegMgr\regmgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\built\Debug\libs\GameSpy\GameSpyMgr\GameSpyMgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "libs_release"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\built\Release\ObjectDLL\ObjectShared\ObjectShared.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\ButeMgr\ButeMgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\MFCStub\MFCStub.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\CryptMgr\cryptmgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\Lib_StdLith\Lib_StdLith.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\Lib_Lith\Lib_Lith.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\built\Release\RegMgr\regmgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\built\Release\libs\GameSpy\GameSpyMgr\GameSpyMgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
