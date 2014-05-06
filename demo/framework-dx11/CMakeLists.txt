#sources
set(SOURCE_CORE application.h 
				application.cpp
				structs.h
				window.h
				window.cpp
				outputd3d11.h
				outputd3d11.cpp
				destroyable.h
				destroyable.cpp
				guirenderer.h
				guirenderer.cpp
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
)
source_group(core FILES ${SOURCE_CORE})
source_group(pipeline FILES ${SOURCE_PIP})
source_group(entities FILES ${SOURCE_ENT})
add_library(framework STATIC ${SOURCE_CORE} ${SOURCE_PIP} ${SOURCE_ENT})

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#subdirectories
add_subdirectory(dds)

#headers search
include_directories(../geomlib ../mathlib ../utils ../guilib)

#dependences
set(DEPS_LIBS utils mathlib geomlib guilib ddslib)

#link libraries
target_link_libraries(framework ${DEPS_LIBS})