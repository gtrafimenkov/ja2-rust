@echo off

@REM === building VS projects

@REM Load VS environment if necessary
if "%VisualStudioVersion%"=="" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
)

msbuild /m /p:Configuration="Debug" /p:Platform=Win32 /p:RestorePackagesConfig=true /t:restore JA2.sln
msbuild /m /p:Configuration="Debug" /p:Platform=Win32 JA2.sln
IF %ERRORLEVEL% NEQ 0 (
  exit %ERRORLEVEL%
)

msbuild /m /p:Configuration="ReleaseWithDebug" /p:Platform=Win32 /p:RestorePackagesConfig=true /t:restore JA2.sln
msbuild /m /p:Configuration="ReleaseWithDebug" /p:Platform=Win32 JA2.sln
IF %ERRORLEVEL% NEQ 0 (
  exit %ERRORLEVEL%
)
