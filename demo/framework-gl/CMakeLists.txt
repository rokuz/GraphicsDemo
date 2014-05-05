#sources
set(SOURCE_LIB  application.h 
				camera.h 
				freeCamera.h 
				geometry3D.h 
				gpuprogram.h 
				lightManager.h 
				line3D.h 
				standardGpuPrograms.h 
				texture.h 
				application.cpp 
				camera.cpp 
				freeCamera.cpp 
				geometry3D.cpp 
				gpuprogram.cpp 
				lightManager.cpp 
				line3D.cpp 
				standardGpuPrograms.cpp 
				texture.cpp 
				renderTarget.h
				renderTarget.cpp
				uniformBuffer.h
				uniformBuffer.cpp
				destroyable.h
				destroyable.cpp
				openglcontext.h
				openglcontext.cpp
				window.h
				window.cpp
)
source_group(core FILES ${SOURCE_LIB})
add_library(framework STATIC ${SOURCE_LIB})

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#subdirectories
add_subdirectory(GL)

#headers search
include_directories(../geomlib ../mathlib ../guilib ../utils)

#dependences
set(DEPS_LIBS gl3w utils mathlib geomlib guilib)

#link KTX
set(LIBRARY_NAME ktx)
add_library(${LIBRARY_NAME} STATIC IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES	
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/ktx/include
)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/ktx/lib/debug/libktx.lib
	IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/ktx/lib/release/libktx.lib
)
set(DEPS_LIBS ${DEPS_LIBS} ${LIBRARY_NAME})

#link libraries
target_link_libraries(framework ${DEPS_LIBS})