cmake_minimum_required(VERSION 3.10)

# Currently it is not possible to build unittester in debug mode
# because of linking problems when linking both gtest and rust static
# library.
option(BUILD_UNITTESTER "Build unit test binary" OFF)

add_subdirectory(libs/corrosion)
corrosion_import_crate(MANIFEST_PATH rustlib/Cargo.toml)

if(WIN32)
    add_compile_options(/utf-8)
endif()

project(ja2v)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

# if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
#     # add_compile_options(-Wno-unused-parameter)  # Example for GCC
# elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#     # add_compile_options(-Wno-unused-parameter)  # Example for Clang
# endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "Compiler: MSVC")

    # treating warnings as errors
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

    add_compile_options(/D_CRT_SECURE_NO_WARNINGS)

    # it's needed for some reason
    add_definitions(-DWINDOWS_IGNORE_PACKING_MISMATCH)
endif()

add_subdirectory(ja2lib)
add_subdirectory(platform-dummy)

if(WIN32)
    add_subdirectory(platform-win32)
    add_subdirectory(bin-win32)
else()
    add_subdirectory(platform-linux)
endif()

if(BUILD_UNITTESTER)
    add_subdirectory(libs/googletest)
    add_subdirectory(unittester)
endif()
