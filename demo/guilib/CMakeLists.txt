#sources
set(SOURCE_LIB 	uimanager.h
				uimanager.cpp
				uistructs.h
				widget.h
				widget.cpp
				overlay.h
				overlay.cpp
				label.h
				label.cpp
)
source_group(core FILES ${SOURCE_LIB})
add_library(guilib STATIC ${SOURCE_LIB})

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#headers search
include_directories(../mathlib ../utils)

#dependences
set (LINKED_PROJECTS mathlib utils)