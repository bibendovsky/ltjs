@echo off
setlocal


rem Parameters:
rem    "/r" - Rebuild

echo - Building Eval

call _build_solution eval %1


endlocal
