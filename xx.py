#!/usr/bin/env python3
#
# Helper script to run different useful commands on the repository.

import os
import platform
import subprocess
import sys
import shutil

USAGE = """Helper script to run one or more predefiend commands.

Usage:
  python xx.py COMMAND [COMMAND...]

Commands:
  build                 - build debug version
  build-debug           - build debug version
  build-release         - build release version
  format-modified       - format modified files using clang-format
  copy-data             - find and copy game data to the debug build localtion
  clean                 - cleanup repository from all unwanted files
  run                   - run debug build
  test                  - run unit test

Examples:
  python xx.py build copy-data run
"""


def get_modified_files():
    result = subprocess.run(
        ["git", "status", "--porcelain"], capture_output=True, text=True
    )
    modified_files = []
    for line in result.stdout.splitlines():
        flags, file_path = line.split()[:2]
        if "M" in flags:
            modified_files.append(file_path)
    return modified_files


def filter_source_files(files):
    source_extensions = [".h", ".c", ".cpp", ".cc"]
    return [file for file in files if os.path.splitext(file)[1] in source_extensions]


def format_files(files):
    if len(files) > 0:
        print(f"Formatting {len(files)} files ...", file=sys.stderr)
        subprocess.run(["clang-format", "-i", "--style=file"] + files)
    else:
        print("No files to format", file=sys.stderr)


def find_ja2_data_files():
    try_dirs = [
        r"C:\Program Files (x86)\Steam\steamapps\common\Jagged Alliance 2 Gold\Data",
    ]
    for d in try_dirs:
        if os.path.isdir(d):
            return d
    return None


def copy_ja2_data_files(dest_dir):
    if os.path.isdir(dest_dir):
        print("Data already copied", file=sys.stderr)
    else:
        source = find_ja2_data_files()
        if source is None:
            print("Cannot find data files for JA2", file=sys.stderr)
        else:
            shutil.copytree(source, dest_dir)


# def get_debug_build_location():
#     if platform.system() == "Windows":
#         return "build/Debug"
#     return "build"


# def get_debug_build_exe():
#     if platform.system() == "Windows":
#         return "build/Debug/ja2vcp.exe"
#     return "build/ja2vcp"


def get_release_build_location():
    if platform.system() == "Windows":
        return "build/bin-win32/RelWithDebInfo"
        return "ReleaseWithDebug"
    return "build"


def get_release_build_exe():
    if platform.system() == "Windows":
        return "build/bin-win32/RelWithDebInfo/ja2v.exe"
    return "build-release/ja2v"


def get_release_test_exe():
    if platform.system() == "Windows":
        return "build/unittester/RelWithDebInfo/unittester.exe"
    return "build-release/unittester/unittester"


def run_command(command):
    if command in ["build-debug"]:
        if platform.system() == "Windows":
            subprocess.run(
                [
                    "cmake",
                    "-G",
                    "Visual Studio 17 2022",
                    "-A",
                    "Win32",
                    "-B",
                    "build",
                    "-DBUILD_UNITTESTER=OFF",
                ],
                check=True,
            )
            subprocess.run(
                ["cmake", "--build", "build", "--parallel", "--config", "Debug"],
                check=True,
            )
        else:
            subprocess.run(
                ["cmake", "-B", "build", "-DCMAKE_BUILD_TYPE=Debug"], check=True
            )
            subprocess.run(["cmake", "--build", "build", "--parallel"], check=True)

    elif command in ["build", "build-release"]:
        if platform.system() == "Windows":
            subprocess.run(
                [
                    "cmake",
                    "-B",
                    "build",
                    "-A",
                    "Win32",
                    "-DBUILD_UNITTESTER=ON",
                ],
                check=True,
            )
            subprocess.run(
                [
                    "cmake",
                    "--build",
                    "build",
                    "--parallel",
                    "--config",
                    "RelWithDebInfo",
                ],
                check=True,
            )
        else:
            subprocess.run(
                [
                    "cmake",
                    "-B",
                    "build-release",
                    "-DCMAKE_BUILD_TYPE=Release",
                    "-DBUILD_UNITTESTER=ON",
                ],
                check=True,
            )
            subprocess.run(
                ["cmake", "--build", "build-release", "--parallel"], check=True
            )

    elif command == "clean":
        subprocess.run(["git", "clean", "-fdx"], check=True)

    elif command == "copy-data":
        dest_dir = os.path.join(get_release_build_location(), "data")
        copy_ja2_data_files(dest_dir)

    elif command == "format-modified":
        modified_files = get_modified_files()
        source_files = filter_source_files(modified_files)
        format_files(source_files)

    elif command == "run":
        subprocess.run([get_release_build_exe()])

    elif command == "test":
        subprocess.run([get_release_test_exe()], check=True)

    else:
        print(f"Unknown command {command}", file=sys.stderr)
        sys.exit(1)


def main():
    args = sys.argv[1:]
    if len(args) == 0:
        print(USAGE, file=sys.stderr)
        sys.exit(1)

    commands = args
    for command in commands:
        run_command(command)


if __name__ == "__main__":
    main()
