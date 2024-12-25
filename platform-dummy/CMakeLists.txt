file(GLOB_RECURSE SRC_FILES *.c)

add_library(platform-dummy STATIC ${SRC_FILES})

target_include_directories(platform-dummy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(platform-dummy PRIVATE ../rustlib/include)

# XXX platform code should not depend on ja2lib
target_include_directories(platform-dummy PRIVATE "../ja2lib")
