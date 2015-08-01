# Microsoft Developer Studio Project File - Name="enginemodellib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=enginemodellib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "enginemodellib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "enginemodellib.mak" CFG="enginemodellib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "enginemodellib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "enginemodellib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "enginemodellib"
# PROP Scc_LocalPath "..\.."
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "enginemodellib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Built\Release\libs\enginemodellib"
# PROP Intermediate_Dir "..\..\Built\Release\libs\enginemodellib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\tools\shared\engine" /I "..\..\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\Runtime\shared\src" /I "..\..\Runtime\model\src" /D "WIN32" /D "NDEBUG" /D "BDEFS_WINDOWS_H" /D "LTA2LTB_D3D" /D "MODEL_LIB" /D "DE_SERVER_COMPILE" /D "DE_HEADLESS_CLIENT" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "enginemodellib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Built\Debug\libs\enginemodellib"
# PROP Intermediate_Dir "..\..\Built\Debug\libs\enginemodellib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\tools\shared\engine" /I "..\..\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\Runtime\lithtemplate\\" /I "..\..\Runtime\shared\src" /I "..\..\Runtime\model\src" /D "NDEBUG" /D "_DEBUG" /D "BDEFS_WINDOWS_H" /D "LTA2LTB_D3" /D "D" /D "MODEL_LIB" /D "DE_SERVER_COMPILE" /D "DE_HEADLESS_CLIENT" /D "__CRTCOMPAT_H__" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "enginemodellib - Win32 Release"
# Name "enginemodellib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Runtime\model\src\animtracker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\animtracker.h
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\sys\d3d\d3d_model_load.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\model.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\model_load.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\model_ops.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\modelallocations.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\modelallocations.h
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\transformmaker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\transformmaker.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Runtime\model\src\ltb.h
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\model.h
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\model_ops.h
# End Source File
# Begin Source File

SOURCE=..\..\Runtime\model\src\sysmodelpiece.h
# End Source File
# End Group
# End Target
# End Project
