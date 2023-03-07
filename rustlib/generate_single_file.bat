@echo off

@ echo .. generating rust_%1.h

@REM Replacing generated headers only when there are actual changes to avoid
@REM unnecessary rebuild of the C code consuming this header.

cbindgen --config cbindgen.toml src/%1.rs --output %TEMP%\rust_%1.h || exit /b 1
fc /B %TEMP%\rust_%1.h rust_%1.h >nul || move %TEMP%\rust_%1.h rust_%1.h
