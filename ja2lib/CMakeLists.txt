file(GLOB_RECURSE SRC_FILES *.c)

add_library(ja2lib STATIC ${SRC_FILES})

target_include_directories(ja2lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ja2lib PRIVATE ../rustlib/include)
