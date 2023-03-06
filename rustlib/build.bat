@echo off

@REM cargo build
@REM cargo build --target x86_64-pc-windows-msvc
cargo build --target i686-pc-windows-msvc

@REM cargo build --release
@REM cargo build --release --target x86_64-pc-windows-msvc
cargo build --release --target i686-pc-windows-msvc

@REM Replacing rustlib.h only when there are actual changes to avoid
@REM unnecessary rebuild of the C code consuming this header.

cbindgen --config cbindgen.toml --crate rustlib --output rustlib.h.new || exit /b 1
fc /B rustlib.h.new rustlib.h >nul
if "%ERRORLEVEL%"=="1" (
    echo "Changes in rustlib.h detected"
    move rustlib.h.new rustlib.h
)
