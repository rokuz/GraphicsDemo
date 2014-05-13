cmake_minimum_required (VERSION 2.8.11)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(cotire)

# Set up the name of the demo
set (PROJECT_NAME Demo)

# Remove some configurations
list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES MinSizeRel)
list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES RelWithDebInfo)
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Semicolon separated list of supported configuration types" FORCE)

# Parameters
set(USE_FBX ON CACHE BOOL "Use fbx sdk")
set(BUILD_TOOLS OFF CACHE BOOL "Build tools")
set(BUILD_TESTS OFF CACHE BOOL "Build tests")

set(GRAPHICS_API_OGL "OpenGL Core Profile 4.x" CACHE STRING "")
set(GRAPHICS_API_DX11 "Direct3D 11" CACHE STRING "")
set(GRAPHICS_API ${GRAPHICS_API_OGL} CACHE STRING "Chosen gpaphics API")
set(GRAPHICS_API_LIST "${GRAPHICS_API_OGL};${GRAPHICS_API_DX11}" CACHE STRING "List of possible graphics APIs")
set_property(CACHE GRAPHICS_API PROPERTY STRINGS ${GRAPHICS_API_LIST})

set(OS_WINDOWS7 "Windows 7" CACHE STRING "")
set(OS_WINDOWS8 "Windows 8" CACHE STRING "")
set(OS_WINDOWS81 "Windows 8.1" CACHE STRING "")
set(OS ${OS_WINDOWS7} CACHE STRING "Chosen OS")
set(OS_LIST "${OS_WINDOWS7};${OS_WINDOWS8};${OS_WINDOWS81}" CACHE STRING "List of possible OS")
set_property(CACHE OS PROPERTY STRINGS ${OS_LIST})

mark_as_advanced(CMAKE_CONFIGURATION_TYPES)
mark_as_advanced(CMAKE_INSTALL_PREFIX)
mark_as_advanced(GRAPHICS_API_LIST)
mark_as_advanced(GRAPHICS_API_OGL)
mark_as_advanced(GRAPHICS_API_DX11)

mark_as_advanced(OS_LIST)
mark_as_advanced(OS_WINDOWS7)
mark_as_advanced(OS_WINDOWS8)
mark_as_advanced(OS_WINDOWS81)

mark_as_advanced(COTIRE_DEBUG)
mark_as_advanced(COTIRE_VERBOSE)
mark_as_advanced(COTIRE_UNITY_SOURCE_EXCLUDE_EXTENSIONS)
mark_as_advanced(COTIRE_MAXIMUM_NUMBER_OF_UNITY_INCLUDES)
mark_as_advanced(COTIRE_MINIMUM_NUMBER_OF_TARGET_SOURCES)
mark_as_advanced(COTIRE_ADDITIONAL_PREFIX_HEADER_IGNORE_EXTENSIONS)
mark_as_advanced(COTIRE_ADDITIONAL_PREFIX_HEADER_IGNORE_PATH)

# Project
project(${PROJECT_NAME})

if (OS STREQUAL OS_WINDOWS7)
add_definitions(-D_OS_WINDOWS7)
elseif (OS STREQUAL OS_WINDOWS8)
add_definitions(-D_OS_WINDOWS8)
elseif (OS STREQUAL OS_WINDOWS81)
add_definitions(-D_OS_WINDOWS81)
endif()

# Subdirectories
add_subdirectory(mathlib)
add_subdirectory(utils)
add_subdirectory(geomlib)
add_subdirectory(guilib)

if( GRAPHICS_API STREQUAL GRAPHICS_API_OGL )
add_subdirectory(framework-gl)
endif()

if( GRAPHICS_API STREQUAL GRAPHICS_API_DX11 )
add_subdirectory(framework-dx11)
endif()

add_subdirectory(src)

IF (BUILD_TESTS)
add_subdirectory(tests)
ENDIF(BUILD_TESTS)

IF (BUILD_TOOLS)
add_subdirectory(tools)
ENDIF(BUILD_TOOLS)