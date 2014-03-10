@echo off

setlocal

rem Parameters:
rem    "/r" - Rebuild

echo - Building All

del build.log

call build_release %1
call build_debug %1
call build_final %1

endlocal
