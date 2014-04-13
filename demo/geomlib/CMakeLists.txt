#sources
set(SOURCE_LIB 	geometryloader.h
				geometryloader.cpp
				geometry.h
				geometry.cpp
				data.h
				data.cpp
				geomloader.h
				geomloader.cpp
				geometrysaver.h
				geometrysaver.cpp
				geomsaver.h
				geomsaver.cpp
				geomformat.h
)
IF (USE_FBX)
set (SOURCE_LIB ${SOURCE_LIB} fbxloader.h fbxloader.cpp)
ENDIF(USE_FBX)
source_group(geomlib FILES ${SOURCE_LIB})
add_library(geomlib STATIC ${SOURCE_LIB})

#preprocessor
IF (USE_FBX)
add_definitions("-D_USE_FBX")
ENDIF(USE_FBX)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#headers search
include_directories(../mathlib ../utils)

#dependences
set (LINKED_PROJECTS mathlib utils)

#link FBX SDK
IF (USE_FBX)
set(LIBRARY_NAME fbxsdk)
add_library(${LIBRARY_NAME} SHARED IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES	
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/fbxsdk/include
)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/fbxsdk/lib/vs/x86/debug/libfbxsdk.lib
	IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/fbxsdk/lib/vs/x86/release/libfbxsdk.lib
)
set(LINKED_PROJECTS ${LINKED_PROJECTS} fbxsdk)

#link libraries
target_link_libraries(geomlib ${LINKED_PROJECTS})

#copyable binaries FBX SDK
set (FBXSDK_COPIABLE_BINARIES_DIR_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/fbxsdk/lib/vs/x86/debug CACHE INTERNAL "")
set(FBXSDK_COPIABLE_BINARIES_DEBUG libfbxsdk.dll
CACHE INTERNAL "")
set (FBXSDK_COPIABLE_BINARIES_DIR_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/fbxsdk/lib/vs/x86/release CACHE INTERNAL "")
set(FBXSDK_COPIABLE_BINARIES_RELEASE libfbxsdk.dll
CACHE INTERNAL "")
ENDIF(USE_FBX)