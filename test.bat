@echo off

call build.bat
if errorlevel 1 exit /b 1

.\Debug\unittester.exe
if errorlevel 1 exit /b 1

.\Release\unittester.exe
if errorlevel 1 exit /b 1
