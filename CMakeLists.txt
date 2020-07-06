cmake_minimum_required(VERSION 3.15)
project(srt_to_udp_server)

set(CMAKE_CXX_STANDARD 17)

#If no build type is set then force Release
IF( NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
            "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
            FORCE)
ENDIF()

#Include cppSRTWrapper
include(ExternalProject)
ExternalProject_Add(project_cppsrtwrapp
        GIT_REPOSITORY https://github.com/andersc/cppSRTWrapper.git
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/srtwrap
        BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/srtwrap
        GIT_PROGRESS 1
        BUILD_COMMAND cmake --build ${CMAKE_CURRENT_SOURCE_DIR}/srtwrap --config ${CMAKE_BUILD_TYPE} --target srtnet
        STEP_TARGETS build
        EXCLUDE_FROM_ALL TRUE
        INSTALL_COMMAND ""
        )
add_library(srtnet STATIC IMPORTED)

IF (WIN32)
    set_property(TARGET srtnet PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/srtwrap/${CMAKE_BUILD_TYPE}/srtnet.lib)
ELSE()
    set_property(TARGET srtnet PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/srtwrap/libsrtnet.a)
ENDIF()

add_dependencies(srtnet project_cppsrtwrapp)

#Find all required dependencies
find_package (Threads REQUIRED)
find_package(OpenSSL REQUIRED)

#Pass information about where to find headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/srtwrap/srt/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/srtwrap/srt/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/srtwrap/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/iniparser/)

#Build the server
add_executable(srt_to_udp_server main.cpp)
IF (WIN32)
    target_link_libraries(srt_to_udp_server srtnet ${CMAKE_CURRENT_SOURCE_DIR}/srtwrap/srt/${CMAKE_BUILD_TYPE}/srt_static.lib ${OPENSSL_LIBRARIES} Threads::Threads)
ELSE()
    target_link_libraries(srt_to_udp_server srtnet ${CMAKE_CURRENT_SOURCE_DIR}/srtwrap/srt/libsrt.a ${OPENSSL_LIBRARIES} Threads::Threads)
ENDIF()

