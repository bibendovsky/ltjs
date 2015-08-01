
@echo off
setlocal


rem Parameters:
rem    "/r" - Rebuild

echo - Building Release

call _build_solution release %1

endlocal
