@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64

msbuild /m /p:Configuration="Debug" /p:Platform=Win32 /p:RestorePackagesConfig=true /t:restore ja2/JA2.sln
msbuild /m /p:Configuration="Debug" /p:Platform=Win32 ja2/JA2.sln
.\ja2\Debug\tester.exe
