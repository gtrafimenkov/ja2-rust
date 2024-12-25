file(GLOB_RECURSE SRC_FILES *.c)

add_library(platform-linux STATIC ${SRC_FILES})

target_include_directories(platform-linux PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(platform-linux PRIVATE ../rustlib/include)

# TODO: platform code should not depend on ja2lib
target_include_directories(platform-linux PRIVATE "../ja2lib")
