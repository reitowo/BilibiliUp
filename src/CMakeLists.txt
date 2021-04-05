cmake_minimum_required (VERSION 3.17)

add_library(bup 
        "bup.cpp"
         )

add_library(k1ee::bup ALIAS bup)
 
if(${CMAKE_USE_FIDDLER_PROXY}) 
    target_compile_definitions(bup PRIVATE USE_FIDDLER_PROXY)
endif()

find_package(cpr CONFIG REQUIRED)
target_link_libraries(bup PRIVATE cpr)

set(VCPKG_INSTALLED_TRIPLET_DIR ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}) 

target_sources(bup PRIVATE ${VCPKG_INSTALLED_TRIPLET_DIR}/include/cpp-base64/base64.cpp) 
target_include_directories(bup PRIVATE ${VCPKG_INSTALLED_TRIPLET_DIR}/include)

find_package(k1ee CONFIG REQUIRED)
target_link_libraries(bup PRIVATE k1ee)    
 
target_include_directories(bup PRIVATE "${VCPKG_INSTALLED_TRIPLET_DIR}/include")
if(DEBUG)
    target_link_directories(bup PRIVATE "${VCPKG_INSTALLED_TRIPLET_DIR}/debug/lib")
    target_link_libraries(bup PRIVATE rhash)    
else()
    target_link_directories(bup PRIVATE "${VCPKG_INSTALLED_TRIPLET_DIR}/lib")
    target_link_libraries(bup PRIVATE rhash)    
endif() 

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        VERSION ${${PROJECT_NAME}_VERSION}
        SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})

set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")

install(TARGETS ${PROJECT_NAME}
        EXPORT ${TARGETS_EXPORT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
        )

include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME})

install(
        EXPORT ${TARGETS_EXPORT_NAME}
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)