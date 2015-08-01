@echo off

setlocal

rem Parameters:
rem    solutionconfig (debug, release, final)
rem    "/r" - Rebuild

set BuildType=/build

if "%2" == "/R" set BuildType=/rebuild
if "%2" == "/r" set BuildType=/rebuild
if "%2" == "/c" set BuildType=/clean
if "%2" == "/C" set BuildType=/clean

if "%2" == "/rebuild" set BuildType=/rebuild
if "%2" == "/clean" set BuildType=/clean

if "%GAME_TOOLS_DIR%" == "" goto notool

rem Set command line paths options etc for 

call %GAME_TOOLS_DIR%\vcvars.exe 2003 > vcvars.bat
call vcvars.bat

@echo on
devenv Libs.sln %BuildType% %1 >> build.log
@echo off
if errorlevel 1 echo -    Found errors

goto done

:notool
@echo -
@echo - Jupiter game tools dir not defined. ex. "Set GAME_TOOLS_DIR = e:\Jupiter\tools"
@echo -
:done
endlocal
