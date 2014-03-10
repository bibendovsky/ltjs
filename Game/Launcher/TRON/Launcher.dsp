# Microsoft Developer Studio Project File - Name="Launcher" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=LAUNCHER - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Launcher.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Launcher.mak" CFG="LAUNCHER - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Launcher - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Launcher - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Launcher"
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Launcher - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Built\Release\Launcher"
# PROP Intermediate_Dir "..\..\Built\Release\Launcher"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\regmgr32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /incremental:yes /machine:I386 /out:"..\..\Built\Release\Launcher/TRON.exe"
# Begin Custom Build
TargetPath=\PROJ\TRON\source\Game\Built\Release\Launcher\TRON.exe
InputPath=\PROJ\TRON\source\Game\Built\Release\Launcher\TRON.exe
SOURCE="$(InputPath)"

"..\..\..\development\to2\TO2.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo copy   $(TargetPath)   ..\..\..\development\to2
	copy   $(TargetPath)   ..\..\..\development\to2

# End Custom Build

!ELSEIF  "$(CFG)" == "Launcher - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Built\Debug\Launcher"
# PROP Intermediate_Dir "..\..\Built\Debug\Launcher"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".\\" /I "..\\" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\regmgr32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"nafxcw.lib" /nodefaultlib:"libcmt.lib" /out:"..\..\Built\Debug\Launcher/TRON.exe" /pdbtype:sept
# Begin Custom Build
TargetPath=\PROJ\TRON\source\Game\Built\Debug\Launcher\TRON.exe
InputPath=\PROJ\TRON\source\Game\Built\Debug\Launcher\TRON.exe
SOURCE="$(InputPath)"

"..\..\..\development\to2\TO2.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo copy   $(TargetPath)   ..\..\..\development\to2
	copy   $(TargetPath)   ..\..\..\development\to2

# End Custom Build

!ENDIF 

# Begin Target

# Name "Launcher - Win32 Release"
# Name "Launcher - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\AniButton.cpp
# End Source File
# Begin Source File

SOURCE=..\AnimDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\BitmapCheckButton.cpp
# End Source File
# Begin Source File

SOURCE=..\ButtonEx.cpp
# End Source File
# Begin Source File

SOURCE=..\DetailSettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\DisplayDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\DisplayMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\DlgEx.cpp
# End Source File
# Begin Source File

SOURCE=..\EditEx.cpp
# End Source File
# Begin Source File

SOURCE=..\Launcher.cpp
# End Source File
# Begin Source File

SOURCE=.\Launcher.rc
# End Source File
# Begin Source File

SOURCE=..\LauncherDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\MessageBoxDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\MoveDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\MultiplayerDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\OptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\RezFind.cpp
# End Source File
# Begin Source File

SOURCE=..\StaticEx.cpp
# End Source File
# Begin Source File

SOURCE=..\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\TextCheckBox.cpp
# End Source File
# Begin Source File

SOURCE=..\Utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\AniButton.h
# End Source File
# Begin Source File

SOURCE=..\AnimDlg.h
# End Source File
# Begin Source File

SOURCE=..\BitmapCheckButton.h
# End Source File
# Begin Source File

SOURCE=..\ButtonEx.h
# End Source File
# Begin Source File

SOURCE=..\DetailSettingsDlg.h
# End Source File
# Begin Source File

SOURCE=..\DisplayDlg.h
# End Source File
# Begin Source File

SOURCE=..\DisplayMgr.h
# End Source File
# Begin Source File

SOURCE=..\DlgEx.h
# End Source File
# Begin Source File

SOURCE=..\EditEx.h
# End Source File
# Begin Source File

SOURCE=..\Launcher.h
# End Source File
# Begin Source File

SOURCE=..\LauncherDlg.h
# End Source File
# Begin Source File

SOURCE=..\MessageBoxDlg.h
# End Source File
# Begin Source File

SOURCE=..\MoveDlg.h
# End Source File
# Begin Source File

SOURCE=..\MultiplayerDlg.h
# End Source File
# Begin Source File

SOURCE=..\OptionsDlg.h
# End Source File
# Begin Source File

SOURCE=..\PlaySound.h
# End Source File
# Begin Source File

SOURCE=..\Resource.h
# End Source File
# Begin Source File

SOURCE=..\RezFind.h
# End Source File
# Begin Source File

SOURCE=..\StaticEx.h
# End Source File
# Begin Source File

SOURCE=..\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\TextCheckBox.h
# End Source File
# Begin Source File

SOURCE=..\Utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Group "bmp"

# PROP Default_Filter ".bmp"
# Begin Group "Language"

# PROP Default_Filter ""
# Begin Group "English"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RES\bmp\Language\English\BackD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\BackF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\BackU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\CancelD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\CancelF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\CancelU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\DisplayD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\DisplayF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\DisplayU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\DisplayX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\FindInternetServersD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\FindInternetServersF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\findinternetserversu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\HelpD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\HelpF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\HelpU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\HighDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\HighDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\HighDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\HostGameD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\HostGameF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\HostGameU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\InstallD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\InstallF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\InstallU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\JoinGameD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\JoinGameF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\JoinGameU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\LowDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\LowDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\LowDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\MediumDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\MediumDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\MediumDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\multiplayerd.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\MultiplayerF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\multiplayeru.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\MultiplayerX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\OKD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\OkF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\OKU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\optionsd.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\OptionsF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\optionsu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\OptionsX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\PlayD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\PlayF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\PlayU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\QuitD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\QuitF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\QuitU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\StandAloneServerD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\StandAloneServerF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\StandAloneServerU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\StandAloneServerX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\UninstallD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\UninstallF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\UninstallU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\English\UninstallX.bmp
# End Source File
# End Group
# Begin Group "French"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RES\bmp\Language\French\BackD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\BackF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\BackU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\CancelD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\CancelF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\CancelU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\DisplayD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\DisplayF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\DisplayU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\DisplayX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\FindInternetServersD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\FindInternetServersF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\findinternetserversu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\HelpD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\HelpF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\HelpU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\HighDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\HighDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\HighDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\HostGameD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\HostGameF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\HostGameU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\InstallD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\InstallF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\InstallU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\JoinGameD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\JoinGameF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\JoinGameU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\LowDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\LowDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\LowDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\MediumDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\MediumDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\MediumDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\multiplayerd.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\MultiplayerF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\multiplayeru.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\MultiplayerX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\OKD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\OkF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\OKU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\optionsd.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\OptionsF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\optionsu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\OptionsX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\PlayD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\PlayF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\PlayU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\QuitD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\QuitF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\QuitU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\StandAloneServerD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\StandAloneServerF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\StandAloneServerU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\StandAloneServerX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\UninstallD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\UninstallF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\UninstallU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\French\UninstallX.bmp
# End Source File
# End Group
# Begin Group "German"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RES\bmp\Language\German\BackD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\BackF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\BackU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\CancelD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\CancelF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\CancelU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\DisplayD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\DisplayF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\DisplayU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\DisplayX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\FindInternetServersD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\FindInternetServersF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\findinternetserversu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\HelpD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\HelpF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\HelpU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\HighDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\HighDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\HighDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\HostGameD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\HostGameF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\HostGameU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\InstallD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\InstallF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\InstallU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\JoinGameD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\JoinGameF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\JoinGameU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\LowDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\LowDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\LowDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\MediumDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\MediumDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\MediumDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\multiplayerd.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\MultiplayerF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\multiplayeru.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\MultiplayerX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\OKD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\OkF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\OKU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\optionsd.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\OptionsF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\optionsu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\OptionsX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\PlayD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\PlayF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\PlayU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\QuitD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\QuitF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\QuitU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\StandAloneServerD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\StandAloneServerF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\StandAloneServerU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\StandAloneServerX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\UninstallD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\UninstallF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\UninstallU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\German\UninstallX.bmp
# End Source File
# End Group
# Begin Group "Spanish"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\BackD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\BackF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\BackU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\CancelD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\CancelF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\CancelU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\DisplayD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\DisplayF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\DisplayU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\DisplayX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\FindInternetServersD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\FindInternetServersF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\findinternetserversu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\HelpD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\HelpF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\HelpU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\HighDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\HighDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\HighDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\HostGameD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\HostGameF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\HostGameU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\InstallD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\InstallF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\InstallU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\JoinGameD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\JoinGameF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\JoinGameU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\LowDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\LowDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\LowDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\MediumDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\MediumDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\MediumDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\multiplayerd.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\MultiplayerF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\multiplayeru.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\MultiplayerX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\OKD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\OkF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\OKU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\optionsd.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\OptionsF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\optionsu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\OptionsX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\PlayD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\PlayF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\PlayU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\QuitD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\QuitF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\QuitU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\StandAloneServerD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\StandAloneServerF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\StandAloneServerU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\StandAloneServerX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\UninstallD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\UninstallF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\UninstallU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Spanish\UninstallX.bmp
# End Source File
# End Group
# Begin Group "Swedish"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\BackD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\BackF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\BackU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\boxbackground.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\CancelD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\CancelF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\CancelU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\DisplayD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\DisplayF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\DisplayU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\DisplayX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\FindInternetServersD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\FindInternetServersF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\findinternetserversu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\HelpD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\HelpF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\HelpU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\HighDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\HighDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\HighDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\HostGameD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\HostGameF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\HostGameU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\InstallD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\InstallF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\InstallU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\JoinGameD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\JoinGameF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\JoinGameU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\LowDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\LowDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\LowDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\MediumDetailD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\MediumDetailF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\MediumDetailU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\multiplayerd.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\MultiplayerF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\multiplayeru.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\MultiplayerX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\OKD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\OkF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\OKU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\optionsd.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\OptionsF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\optionsu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\OptionsX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\PlayD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\PlayF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\PlayU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\QuitD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\QuitF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\QuitU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\StandAloneServerD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\StandAloneServerF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\StandAloneServerU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\StandAloneServerX.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\UninstallD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\UninstallF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\UninstallU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Language\Swedish\UninstallX.bmp
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\RES\bmp\BoxBackground.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\CheckBoxC.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\CheckBoxF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\CheckBoxN.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\closed.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\closeu.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\CompanyWebD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\CompanyWebF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\CompanyWebU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\DetailSettingsBackground.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\DisplayBackground.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Error.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Information.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\MainAppBackground.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\MinimizeD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\MinimizeU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\MultiplayerBackground.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\OptionsBackground.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\PublisherWebD.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\PublisherWebF.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\PublisherWebU.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\bmp\Warning.bmp
# End Source File
# End Group
# Begin Group "snd"

# PROP Default_Filter ".wav"
# Begin Source File

SOURCE=.\RES\snd\AnimDlg.wav
# End Source File
# Begin Source File

SOURCE=.\RES\snd\buttondown.wav
# End Source File
# Begin Source File

SOURCE=.\RES\snd\Click.wav
# End Source File
# Begin Source File

SOURCE=.\RES\snd\Intro.wav
# End Source File
# Begin Source File

SOURCE=.\RES\snd\Select.wav
# End Source File
# Begin Source File

SOURCE=.\RES\snd\type1.WAV
# End Source File
# Begin Source File

SOURCE=.\RES\snd\type2.WAV
# End Source File
# Begin Source File

SOURCE=.\RES\snd\type3.WAV
# End Source File
# Begin Source File

SOURCE=.\RES\snd\TypeBack.wav
# End Source File
# End Group
# Begin Group "ani"

# PROP Default_Filter ".avi"
# Begin Source File

SOURCE=.\RES\ani\Buttons.avi
# End Source File
# Begin Source File

SOURCE=.\RES\ani\OK.avi
# End Source File
# End Group
# Begin Source File

SOURCE=.\RES\Launcher.ico
# End Source File
# Begin Source File

SOURCE=.\RES\Launcher.rc2
# End Source File
# Begin Source File

SOURCE=.\RES\WebHand.cur
# End Source File
# End Group
# Begin Group "Libs_Debug"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=..\..\..\libs\built\Debug\RegMgr32\regmgr32.lib

!IF  "$(CFG)" == "Launcher - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Launcher - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Libs_Release"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=..\..\..\libs\built\Release\RegMgr32\regmgr32.lib

!IF  "$(CFG)" == "Launcher - Win32 Release"

!ELSEIF  "$(CFG)" == "Launcher - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
