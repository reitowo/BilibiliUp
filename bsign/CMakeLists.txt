cmake_minimum_required(VERSION 3.17)
project(bsign VERSION 1.0 LANGUAGES CXX)

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    message(FATAL_ERROR "Unable To Build x64")
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

option(BUILD_SHARED_LIBS "Shared Library" ON)

include(GNUInstallDirs)

add_executable(bsign
        bsign.cpp)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION "./"
        )