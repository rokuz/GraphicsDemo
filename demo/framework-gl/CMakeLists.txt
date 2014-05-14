#sources
set(PRECOMPILED stdafx.cpp stdafx.h)
set(SOURCE_LIB  framework.h
				application.h 
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
				uifactory.h
				uifactory.cpp
				uirenderer.h
				uirenderer.cpp
				pipelinestate.h
				pipelinestate.cpp
)
set(SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/gl/win32/standard/arrow.gsh.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/gl/win32/standard/arrow.fsh.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/gl/win32/standard/arrow.vsh.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/gl/win32/standard/line.fsh.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/gl/win32/standard/line.vsh.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/gui/shaders/gl/win32/text.gsh.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/gui/shaders/gl/win32/text.fsh.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/gui/shaders/gl/win32/text.vsh.glsl
)
source_group(shaders FILES ${SHADERS})
source_group(core FILES ${SOURCE_LIB})
source_group(precompiled FILES ${PRECOMPILED})
add_library(framework STATIC ${PRECOMPILED} ${SOURCE_LIB} ${SHADERS})

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

#link FreeImage
set(LIBRARY_NAME freeimage)
add_library(${LIBRARY_NAME} SHARED IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES	
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/freeimage
)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/freeimage/FreeImaged.lib
	IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/freeimage/FreeImage.lib
)
set(DEPS_LIBS ${DEPS_LIBS} ${LIBRARY_NAME})

set (FREEIMAGE_COPIABLE_BINARIES_DIR_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/freeimage CACHE INTERNAL "")
set(FREEIMAGE_COPIABLE_BINARIES_DEBUG FreeImaged.dll
CACHE INTERNAL "")
set (FREEIMAGE_COPIABLE_BINARIES_DIR_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/freeimage CACHE INTERNAL "")
set(FREEIMAGE_COPIABLE_BINARIES_RELEASE FreeImage.dll
CACHE INTERNAL "")

#link libraries
target_link_libraries(framework ${DEPS_LIBS})

#precompiled headers
set_target_properties(framework PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties(framework PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
cotire(framework)