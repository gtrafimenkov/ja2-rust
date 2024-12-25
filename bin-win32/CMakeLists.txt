file(GLOB_RECURSE SRC_FILES *.c)

add_executable(ja2v
    ${SRC_FILES}
    Res/ja2.rc
    Res/jagged3.ico
    )

target_include_directories(ja2v PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ja2v PRIVATE "../ja2lib")
target_include_directories(ja2v PRIVATE ../rustlib/include)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_link_options(ja2v PUBLIC "/SUBSYSTEM:WINDOWS")
    target_link_options(ja2v PUBLIC "/SAFESEH:NO")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # fixing warning LNK4098: defaultlib 'LIBCMT' conflicts with use of other libs; use /NODEFAULTLIB:library
    target_link_options(ja2v PRIVATE /NODEFAULTLIB:libcmt)
endif()

target_link_libraries(ja2v PRIVATE "${CMAKE_SOURCE_DIR}/bin-win32/mss32.lib")
target_link_libraries(ja2v PRIVATE "${CMAKE_SOURCE_DIR}/bin-win32/smackw32.lib")
target_link_libraries(ja2v PRIVATE "winmm.lib")
target_link_libraries(ja2v PRIVATE "ddraw.lib")
target_link_libraries(ja2v PRIVATE "netapi32.lib") # ggstd needs it
target_link_libraries(ja2v PRIVATE "secur32.lib")  # ggstd needs it
target_link_libraries(ja2v PRIVATE ja2lib)
target_link_libraries(ja2v PRIVATE platform-win32)
target_link_libraries(ja2v PRIVATE rustlib)
