file(GLOB_RECURSE SRC_FILES *.c)

add_library(platform-win32 STATIC ${SRC_FILES})

target_include_directories(platform-win32 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(platform-win32 PRIVATE ../rustlib/include)

# TODO: platform code should not depend on ja2lib
target_include_directories(platform-win32 PRIVATE "../ja2lib")
