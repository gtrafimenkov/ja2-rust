@echo off

call test.bat
if errorlevel 1 exit /b 1

.\ReleaseWithDebug\ja2-win32-rwdi.exe
