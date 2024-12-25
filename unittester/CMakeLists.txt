if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # Rust always uses release C runtime.
    # Using the same runtime for this project.
    # This is needed if we link gtest library.
    add_compile_options(/MT) # Multi-threaded static release C runtime
endif()

file(GLOB_RECURSE SRC_FILES *.cpp)

add_executable(unittester ${SRC_FILES})

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # fixing warning LNK4098: defaultlib 'LIBCMT' conflicts with use of other libs; use /NODEFAULTLIB:library
    target_link_options(unittester PRIVATE /NODEFAULTLIB:libcmt)
endif()

target_link_libraries(unittester PRIVATE gtest gtest_main)
target_link_libraries(unittester PRIVATE ja2lib)
target_link_libraries(unittester PRIVATE platform-dummy)
if(WIN32)
    target_link_libraries(unittester PRIVATE platform-win32)
    target_link_libraries(unittester PRIVATE "netapi32.lib") # ggstd needs it
    target_link_libraries(unittester PRIVATE "secur32.lib")  # ggstd needs it
else()
    target_link_libraries(unittester PRIVATE platform-linux)
endif()
target_link_libraries(unittester PRIVATE rustlib)

target_include_directories(unittester PRIVATE ../ja2lib)
target_include_directories(unittester PRIVATE ../rustlib/include)
