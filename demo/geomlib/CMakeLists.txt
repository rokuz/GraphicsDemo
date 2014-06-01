#sources
set(PRECOMPILED stdafx.cpp stdafx.h)
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
				planegenerator.h
				planegenerator.cpp
				geometrygenerator.h
				geometrygenerator.cpp
)
IF (USE_FBX)
set (SOURCE_LIB ${SOURCE_LIB} fbxloader.h fbxloader.cpp)
ENDIF(USE_FBX)
source_group(core FILES ${SOURCE_LIB})
source_group(precompiled FILES ${PRECOMPILED})
add_library(geomlib STATIC ${PRECOMPILED} ${SOURCE_LIB})

#preprocessor
IF (USE_FBX)
add_definitions("-D_USE_FBX")
ENDIF(USE_FBX)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#headers search
include_directories(../mathlib ../utils ../json)

#dependences
set (LINKED_PROJECTS mathlib utils jsonlib)

#link FBX SDK
IF (USE_FBX)
set(LIBRARY_NAME fbxsdk)
add_library(${LIBRARY_NAME} SHARED IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES	
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/fbxsdk/include
)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/fbxsdk/lib/vs/x86/debug/libfbxsdk-md.lib
	IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/fbxsdk/lib/vs/x86/release/libfbxsdk-md.lib
)
set(LINKED_PROJECTS ${LINKED_PROJECTS} fbxsdk)

#link libraries
target_link_libraries(geomlib ${LINKED_PROJECTS})

ENDIF(USE_FBX)

#precompiled headers
set_target_properties(geomlib PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties(geomlib PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
cotire(geomlib)