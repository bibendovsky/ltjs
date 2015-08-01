@echo off
setlocal


rem Parameters:
rem    "/r" - Rebuild

echo - Building Final

call _build_solution final %1


endlocal
