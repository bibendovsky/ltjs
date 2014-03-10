
@echo off
setlocal


rem Parameters:
rem    "/r" - Rebuild

echo Building - Debug

call _build_solution debug %1

endlocal
