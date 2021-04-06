cmake_minimum_required (VERSION 3.17)

target_include_directories(bup PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_sources(bup PRIVATE
        # Header files (useful in IDEs)
        "bup/bup.h"
        )

install(DIRECTORY bup DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})