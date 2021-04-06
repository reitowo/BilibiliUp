cmake_minimum_required(VERSION 3.17)
project(bup VERSION 1.0 LANGUAGES CXX)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

option(BUILD_BUP_TESTS "Build Tests" ON)
option(BUILD_SHARED_LIBS "Shared Library" ON)

include(GNUInstallDirs)

add_subdirectory(src)
add_subdirectory(include)

if(BUILD_BUP_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# 似乎不需要签名
# install(FILES ${CMAKE_CURRENT_LIST_DIR}/bsign/bsign.exe DESTINATION tools/bup)