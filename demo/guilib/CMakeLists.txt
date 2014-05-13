#sources
set(PRECOMPILED stdafx.cpp stdafx.h)
set(SOURCE_LIB 	uimanager.h
				uimanager.cpp
				uistructs.h
				widget.h
				widget.cpp
				overlay.h
				overlay.cpp
				label.h
				label.cpp
				fontmanager.h
				fontmanager.cpp
)
source_group(core FILES ${SOURCE_LIB})
source_group(precompiled FILES ${PRECOMPILED})
add_library(guilib STATIC ${PRECOMPILED} ${SOURCE_LIB})

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#headers search
include_directories(../mathlib ../utils)

#dependences
set (LINKED_PROJECTS mathlib utils)

# FreeType library
set(LIBRARY_NAME freetype)
add_library(${LIBRARY_NAME} STATIC IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES	
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/freetype/include
)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/freetype/lib/win32/freetype250_D.lib
	IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/freetype/lib/win32/freetype250.lib
)
set(LINKED_PROJECTS ${LINKED_PROJECTS} ${LIBRARY_NAME})

#link libraries
target_link_libraries(guilib ${LINKED_PROJECTS})

#precompiled headers
set_target_properties(guilib PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties(guilib PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
cotire(guilib)