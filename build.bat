@echo off

@REM === building rustlib

pushd rustlib
call build.bat || exit /b 1
popd

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

copy rustlib\target\i686-pc-windows-msvc\debug\rustlib.dll .\Debug

msbuild /m /p:Configuration="ReleaseWithDebug" /p:Platform=Win32 /p:RestorePackagesConfig=true /t:restore JA2.sln
msbuild /m /p:Configuration="ReleaseWithDebug" /p:Platform=Win32 JA2.sln
IF %ERRORLEVEL% NEQ 0 (
  exit %ERRORLEVEL%
)

copy rustlib\target\i686-pc-windows-msvc\release\rustlib.dll .\Release
copy rustlib\target\i686-pc-windows-msvc\release\rustlib.dll .\ReleaseWithDebug
