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
				blendstage.h
				blendstage.cpp
				uniformbuffer.h
				uniformbuffer.cpp
				gpuprogram.h
				gpuprogram.cpp
				standardGpuPrograms.h
				standardGpuPrograms.cpp
				texture.h
				texture.cpp
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
include_directories(../geomlib ../mathlib ../utils)

#dependences
set(DEPS_LIBS utils mathlib geomlib ddslib)

#link DirectX SDK (yes, we are going to support Windows 7 as long as it lives)
set(LIBRARY_NAME d3d11)
add_library(${LIBRARY_NAME} STATIC IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES	
	INTERFACE_INCLUDE_DIRECTORIES $ENV{DXSDK_DIR}Include
)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_LOCATION $ENV{DXSDK_DIR}Lib/x86/d3d11.lib
)
set(DEPS_LIBS ${DEPS_LIBS} ${LIBRARY_NAME})

set(LIBRARY_NAME dxgi)
add_library(${LIBRARY_NAME} STATIC IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_LOCATION_DEBUG $ENV{DXSDK_DIR}Lib/x86/dxgi.lib
	IMPORTED_LOCATION_RELEASE $ENV{DXSDK_DIR}Lib/x86/dxgi.lib
)
set(DEPS_LIBS ${DEPS_LIBS} ${LIBRARY_NAME})

set(LIBRARY_NAME dxcompiler)
add_library(${LIBRARY_NAME} STATIC IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_LOCATION_DEBUG $ENV{DXSDK_DIR}Lib/x86/d3dcompiler.lib
	IMPORTED_LOCATION_RELEASE $ENV{DXSDK_DIR}Lib/x86/d3dcompiler.lib
)
set(DEPS_LIBS ${DEPS_LIBS} ${LIBRARY_NAME})

set(LIBRARY_NAME dxguid)
add_library(${LIBRARY_NAME} STATIC IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_LOCATION_DEBUG $ENV{DXSDK_DIR}Lib/x86/dxguid.lib
	IMPORTED_LOCATION_RELEASE $ENV{DXSDK_DIR}Lib/x86/dxguid.lib
)
set(DEPS_LIBS ${DEPS_LIBS} ${LIBRARY_NAME})

#link CEGUI
set(LIBRARY_NAME cegui)
add_library(${LIBRARY_NAME} SHARED IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES	
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/cegui/include
)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/cegui/lib/CEGUIBase-0_d.lib
	IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/cegui/lib/CEGUIBase-0.lib
)
set(DEPS_LIBS ${DEPS_LIBS} ${LIBRARY_NAME})

#link CEGUI Renderer
set(LIBRARY_NAME cegui_renderer)
add_library(${LIBRARY_NAME} SHARED IMPORTED GLOBAL)
set_target_properties(${LIBRARY_NAME} PROPERTIES	
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/cegui/include
)
set_target_properties(${LIBRARY_NAME} PROPERTIES
	IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/cegui/lib/CEGUIDirect3D11Renderer-0_d.lib
	IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/cegui/lib/CEGUIDirect3D11Renderer-0.lib
)
set(DEPS_LIBS ${DEPS_LIBS} ${LIBRARY_NAME})

# Copyable binaries CEGUI
set (CEGUI_COPIABLE_BINARIES_DIR_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/cegui/bin CACHE INTERNAL "")
set(CEGUI_COPIABLE_BINARIES_DEBUG CEGUIBase-0_d.dll 
								  CEGUICoreWindowRendererSet_d.dll 
								  CEGUIExpatParser_d.dll 
								  CEGUIFreeImageCodec_d.dll 
								  CEGUIDirect3D11Renderer-0_d.dll
								  expat_d.dll 
								  FreeImage_d.dll 
								  freetype_d.dll 
								  glew_d.dll
								  pcre_d.dll
								  minizip_d.dll
								  zlib_d.dll
CACHE INTERNAL "")
set (CEGUI_COPIABLE_BINARIES_DIR_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/cegui/bin CACHE INTERNAL "")
set(CEGUI_COPIABLE_BINARIES_RELEASE CEGUIBase-0.dll 
								    CEGUICoreWindowRendererSet.dll 
								    CEGUIExpatParser.dll 
								    CEGUIFreeImageCodec.dll 
								    CEGUIDirect3D11Renderer-0.dll 
								    expat.dll 
								    FreeImage.dll 
								    freetype.dll 
								    glew.dll
									pcre.dll
									minizip.dll
									zlib.dll
CACHE INTERNAL "")

#link KTX
#set(LIBRARY_NAME ktx)
#add_library(${LIBRARY_NAME} STATIC IMPORTED GLOBAL)
#set_target_properties(${LIBRARY_NAME} PROPERTIES	
#	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/ktx/include
#)
#set_target_properties(${LIBRARY_NAME} PROPERTIES
#	IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/ktx/lib/debug/libktx.lib
#	IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/ktx/lib/release/libktx.lib
#)
#set(DEPS_LIBS ${DEPS_LIBS} ${LIBRARY_NAME})

#link libraries
target_link_libraries(framework ${DEPS_LIBS})