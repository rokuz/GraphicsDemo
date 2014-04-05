#sources
if( GRAPHICS_API STREQUAL GRAPHICS_API_OGL )
set(SOURCE_EXE main_ogl.cpp)
endif()
if( GRAPHICS_API STREQUAL GRAPHICS_API_DX11 )
set(SOURCE_EXE main_dx11.cpp)
endif()
add_executable(${PROJECT_NAME} ${SOURCE_EXE})

#options
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#headers search
if( GRAPHICS_API STREQUAL GRAPHICS_API_OGL )
set(FRAMEWORK_DIR "../framework-gl")
endif()
if( GRAPHICS_API STREQUAL GRAPHICS_API_DX11 )
set(FRAMEWORK_DIR "../framework-dx11")
endif()
include_directories(../mathlib ../geomlib ../utils ${FRAMEWORK_DIR})

#link projects
target_link_libraries(${PROJECT_NAME} framework)

# set copying CEGUI binaries
foreach(item IN LISTS CEGUI_COPIABLE_BINARIES_DEBUG)
add_custom_command(TARGET ${PROJECT_NAME}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${CEGUI_COPIABLE_BINARIES_DIR_DEBUG}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Debug/${item}")
endforeach()
foreach(item IN LISTS CEGUI_COPIABLE_BINARIES_RELEASE)
add_custom_command(TARGET ${PROJECT_NAME}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${CEGUI_COPIABLE_BINARIES_DIR_RELEASE}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Release/${item}")
endforeach()

# set copying FBX SDK binaries
IF (USE_FBX)
foreach(item IN LISTS FBXSDK_COPIABLE_BINARIES_DEBUG)
add_custom_command(TARGET ${PROJECT_NAME}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${FBXSDK_COPIABLE_BINARIES_DIR_DEBUG}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Debug/${item}")
endforeach()
foreach(item IN LISTS FBXSDK_COPIABLE_BINARIES_RELEASE)
add_custom_command(TARGET ${PROJECT_NAME}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${FBXSDK_COPIABLE_BINARIES_DIR_RELEASE}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Release/${item}")
endforeach()
ENDIF(USE_FBX)