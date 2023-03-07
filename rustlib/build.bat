@echo off

@REM cargo build
@REM cargo build --target x86_64-pc-windows-msvc
cargo build --target i686-pc-windows-msvc

@REM cargo build --release
@REM cargo build --release --target x86_64-pc-windows-msvc
cargo build --release --target i686-pc-windows-msvc

call generate_single_file.bat sector
call generate_single_file.bat sam_sites
