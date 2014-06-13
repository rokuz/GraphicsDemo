#sources
set(PRECOMPILED stdafx.cpp stdafx.h)
set(SOURCE_CORE framework.h
				application.h 
				application.cpp
				structs.h
				window.h
				window.cpp
				outputd3d11.h
				outputd3d11.cpp
				destroyable.h
				destroyable.cpp
				uirendererd3d11.h
				uirendererd3d11.cpp
				uifactoryd3d11.h
				uifactoryd3d11.cpp				
)
set(SOURCE_PIP  pipelinestage.h
				pipelinestage.cpp
				rasterizerstage.h
				rasterizerstage.cpp
				renderTarget.h
				renderTarget.cpp
				resourceview.h
				resourceview.cpp
				depthstencilstage.h
				depthstencilstage.cpp
				sampler.h
				sampler.cpp
				blendstage.h
				blendstage.cpp
				uniformbuffer.h
				uniformbuffer.cpp
				gpuprogram.h
				gpuprogram.cpp
				standardgpuprograms.h
				standardgpuprograms.cpp
				texture.h
				texture.cpp
				unorderedaccessbuffer.h
				unorderedaccessbuffer.cpp
				unorderedaccessiblebatch.h
				unorderedaccessiblebatch.cpp
)
set(SOURCE_ENT  camera.h
				camera.cpp
				freeCamera.h
				freeCamera.cpp
				line3D.h
				line3D.cpp
				geometry3D.h
				geometry3D.cpp
				lightManager.h
				lightManager.cpp
				materialmanager.h
				materialmanager.cpp
)
set(SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/../../data/gui/shaders/dx11/text.vsh.hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/gui/shaders/dx11/text.gsh.hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/gui/shaders/dx11/text.psh.hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/dx11/standard/arrow.gsh.hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/dx11/standard/arrow.psh.hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/dx11/standard/arrow.vsh.hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/dx11/standard/line.psh.hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/dx11/standard/line.vsh.hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/dx11/standard/skybox.gsh.hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/dx11/standard/skybox.psh.hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/../../data/shaders/dx11/standard/skybox.vsh.hlsl
)
source_group(shaders FILES ${SHADERS})
source_group(core FILES ${SOURCE_CORE})
source_group(pipeline FILES ${SOURCE_PIP})
source_group(entities FILES ${SOURCE_ENT})
source_group(shaders FILES ${SHADERS})
source_group(precompiled FILES ${PRECOMPILED})
add_library(framework STATIC ${PRECOMPILED} ${SOURCE_CORE} ${SOURCE_PIP} ${SOURCE_ENT} ${SHADERS})

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#subdirectories
add_subdirectory(dds)

#headers search
include_directories(../geomlib ../mathlib ../utils ../guilib ../json)

#dependences
set(DEPS_LIBS utils mathlib geomlib guilib ddslib jsonlib)

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