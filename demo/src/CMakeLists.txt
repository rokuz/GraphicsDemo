macro (INIT_DEMO demo_name ogl_sources dx11_sources)
#sources
if( GRAPHICS_API STREQUAL GRAPHICS_API_OGL )
	if (${ogl_sources} STREQUAL "skipped")
	message("Demo '" ${demo_name} "' does not contain " ${GRAPHICS_API_OGL} " implementation")
	set(IS_INITED OFF)
	else()
	set(SOURCE_EXE ${ogl_sources})
	set(IS_INITED ON)
	endif()
endif()
if( GRAPHICS_API STREQUAL GRAPHICS_API_DX11 )
	if (${dx11_sources} STREQUAL "skipped")
	message("Demo '" ${demo_name} "' does not contain " ${GRAPHICS_API_DX11} " implementation")
	set(IS_INITED OFF)
	else()
	set(SOURCE_EXE ${dx11_sources})
	set(IS_INITED ON)
	endif()
endif()

if (IS_INITED)
#executable
add_executable(${demo_name} ${SOURCE_EXE})

#options
set_target_properties(${demo_name} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#headers search
if( GRAPHICS_API STREQUAL GRAPHICS_API_OGL )
set(FRAMEWORK_DIR "../../framework-gl")
endif()
if( GRAPHICS_API STREQUAL GRAPHICS_API_DX11 )
set(FRAMEWORK_DIR "../../framework-dx11")
endif()
include_directories(../../mathlib ../../geomlib ../../utils ${FRAMEWORK_DIR})

#link projects
target_link_libraries(${demo_name} framework)

# set copying CEGUI binaries
foreach(item IN LISTS CEGUI_COPIABLE_BINARIES_DEBUG)
add_custom_command(TARGET ${demo_name}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${CEGUI_COPIABLE_BINARIES_DIR_DEBUG}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Debug/${item}")
endforeach()
foreach(item IN LISTS CEGUI_COPIABLE_BINARIES_RELEASE)
add_custom_command(TARGET ${demo_name}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${CEGUI_COPIABLE_BINARIES_DIR_RELEASE}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Release/${item}")
endforeach()

# set copying FBX SDK binaries
IF (USE_FBX)
foreach(item IN LISTS FBXSDK_COPIABLE_BINARIES_DEBUG)
add_custom_command(TARGET ${demo_name}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${FBXSDK_COPIABLE_BINARIES_DIR_DEBUG}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Debug/${item}")
endforeach()
foreach(item IN LISTS FBXSDK_COPIABLE_BINARIES_RELEASE)
add_custom_command(TARGET ${demo_name}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${FBXSDK_COPIABLE_BINARIES_DIR_RELEASE}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Release/${item}")
endforeach()
ENDIF(USE_FBX)
endif (IS_INITED)
endmacro(INIT_DEMO)

#list of demos
add_subdirectory(template)
add_subdirectory(OIT)