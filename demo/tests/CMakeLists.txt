#sources
set(SOURCE_TESTS mathlibtests.cpp)
source_group(tests FILES ${SOURCE_TESTS})
add_executable(tests ${SOURCE_TESTS})

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS -DGTEST_HAS_TR1_TUPLE=0)

#dependences
set (LINKED_PROJECTS mathlib)

#headers search
include_directories(../mathlib)

#link gtest
set(LIBRARY_NAME gtest)
add_library(${LIBRARY_NAME} STATIC IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES	
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/gtest/include
)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/gtest/lib/debug/gtest.lib
	IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/gtest/lib/release/gtest.lib
)
set(LINKED_PROJECTS ${LINKED_PROJECTS} ${LIBRARY_NAME})

set(LIBRARY_NAME gtest_main)
add_library(${LIBRARY_NAME} STATIC IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/gtest/lib/debug/gtest_main.lib
	IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/gtest/lib/release/gtest_main.lib
)
set(LINKED_PROJECTS ${LINKED_PROJECTS} ${LIBRARY_NAME})

#link libraries
target_link_libraries(tests ${LINKED_PROJECTS})