cmake_minimum_required (VERSION 2.8.11)

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

mark_as_advanced(CMAKE_CONFIGURATION_TYPES)
mark_as_advanced(CMAKE_INSTALL_PREFIX)
mark_as_advanced(GRAPHICS_API_LIST)
mark_as_advanced(GRAPHICS_API_OGL)
mark_as_advanced(GRAPHICS_API_DX11)

# Project
project(${PROJECT_NAME})

# Subdirectories
add_subdirectory(mathlib)
add_subdirectory(utils)
add_subdirectory(geomlib)

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