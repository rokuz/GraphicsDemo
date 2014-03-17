#sources
set(TOOL_NAME geomconv)
set(SOURCE_TOOL geomconv.cpp)
add_executable(${TOOL_NAME} ${SOURCE_TOOL})

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#link libraries
target_link_libraries(${TOOL_NAME} geomlib)

#headers search
include_directories(../mathlib ../geomlib)

IF (USE_FBX)
# set copying FBX SDK binaries
foreach(item IN LISTS FBXSDK_COPIABLE_BINARIES_DEBUG)
add_custom_command(TARGET ${TOOL_NAME}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${FBXSDK_COPIABLE_BINARIES_DIR_DEBUG}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Debug/${item}")
endforeach()
foreach(item IN LISTS FBXSDK_COPIABLE_BINARIES_RELEASE)
add_custom_command(TARGET ${TOOL_NAME}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${FBXSDK_COPIABLE_BINARIES_DIR_RELEASE}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Release/${item}")
endforeach()
ENDIF(USE_FBX)