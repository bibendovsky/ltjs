@echo off
call ..\..\ICVars.bat
%VC5_NMAKE% /f d3drender_60.mak CFG="d3drender - Win32 Release" %1 %2 %3 %4 %5 %6 %7 %8


