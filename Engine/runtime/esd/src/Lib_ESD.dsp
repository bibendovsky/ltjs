# Microsoft Developer Studio Project File - Name="Lib_ESD" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Lib_ESD - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Lib_ESD.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Lib_ESD.mak" CFG="Lib_ESD - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Lib_ESD - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Lib_ESD - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Lib_ESD"
# PROP Scc_LocalPath "..\..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Lib_ESD - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\built\Release\Runtime\Lib_ESD"
# PROP Intermediate_Dir "..\..\..\built\Release\Runtime\Lib_ESD"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\..\Libs\STLPORT-4.0\STLPORT" /I "..\..\kernel\src" /I "..\..\shared\src" /I "..\..\..\..\libs\lith" /I "..\..\..\..\libs\stdlith" /I "..\..\..\libs\externalesd\realmedia\include" /I "..\..\..\sdk\inc" /I "..\..\kernel\mem\src" /I "..\..\sound\src" /I "..\..\client\src" /I "..\..\kernel\io\src" /I "..\..\world\src" /I "..\..\kernel\src\sys\win" /I "..\..\model\src" /I "..\..\kernel\net\src" /I "..\..\comm\src" /I "..\..\shared\src\sys\win" /I "..\..\..\libs\externalesd\realconsole\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "LITHTECH_ESD" /D "_WIN32_DCOM" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Lib_ESD - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\built\Debug\Runtime\Lib_ESD"
# PROP Intermediate_Dir "..\..\..\built\Debug\Runtime\Lib_ESD"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\Libs\STLPORT-4.0\STLPORT" /I "..\..\kernel\src" /I "..\..\shared\src" /I "..\..\..\..\libs\lith" /I "..\..\..\..\libs\stdlith" /I "..\..\..\libs\externalesd\realmedia\include" /I "..\..\..\sdk\inc" /I "..\..\kernel\mem\src" /I "..\..\sound\src" /I "..\..\client\src" /I "..\..\kernel\io\src" /I "..\..\world\src" /I "..\..\kernel\src\sys\win" /I "..\..\model\src" /I "..\..\kernel\net\src" /I "..\..\comm\src" /I "..\..\shared\src\sys\win" /I "..\..\..\libs\externalesd\realconsole\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "LITHTECH_ESD" /D "_WIN32_DCOM" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Lib_ESD - Win32 Release"
# Name "Lib_ESD - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ltraudiodevice.cpp
# End Source File
# Begin Source File

SOURCE=.\ltraudiohook.cpp
# End Source File
# Begin Source File

SOURCE=.\ltraudioinforesponse.cpp
# End Source File
# Begin Source File

SOURCE=.\ltraudioqueue.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrclientadvicesink.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrclientcontext.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrclientenginesetup.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrconout.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrealaudio_impl.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrealconsole_impl.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrealcore.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrealdef.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrealfileobject.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrealvideo_impl.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrerrorsink.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrlist.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrmap.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrsitesupplier.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrvideooverlay.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrvideosurface.cpp
# End Source File
# Begin Source File

SOURCE=.\ltrwindowlesssite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\externalesd\realconsole\src\rngclib\rngclib.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ltraudiodevice.h
# End Source File
# Begin Source File

SOURCE=.\ltraudiohook.h
# End Source File
# Begin Source File

SOURCE=.\ltraudioinforesponse.h
# End Source File
# Begin Source File

SOURCE=.\ltraudioqueue.h
# End Source File
# Begin Source File

SOURCE=.\ltrclientadvicesink.h
# End Source File
# Begin Source File

SOURCE=.\ltrclientcontext.h
# End Source File
# Begin Source File

SOURCE=.\ltrclientenginesetup.h
# End Source File
# Begin Source File

SOURCE=.\ltrconout.h
# End Source File
# Begin Source File

SOURCE=.\ltrdrynotification.h
# End Source File
# Begin Source File

SOURCE=.\ltreal_impl.h
# End Source File
# Begin Source File

SOURCE=.\ltrealaudio_impl.h
# End Source File
# Begin Source File

SOURCE=.\ltrealconsole_impl.h
# End Source File
# Begin Source File

SOURCE=.\ltrealcore.h
# End Source File
# Begin Source File

SOURCE=.\ltrealfileobject.h
# End Source File
# Begin Source File

SOURCE=.\ltrealvideo_impl.h
# End Source File
# Begin Source File

SOURCE=.\ltrerrorsink.h
# End Source File
# Begin Source File

SOURCE=.\ltrlist.h
# End Source File
# Begin Source File

SOURCE=.\ltrmap.h
# End Source File
# Begin Source File

SOURCE=.\ltrsitesupplier.h
# End Source File
# Begin Source File

SOURCE=.\ltrvdef.h
# End Source File
# Begin Source File

SOURCE=.\ltrvideooverlay.h
# End Source File
# Begin Source File

SOURCE=.\ltrvideosurface.h
# End Source File
# Begin Source File

SOURCE=.\ltrwindowlesssite.h
# End Source File
# End Group
# End Target
# End Project
