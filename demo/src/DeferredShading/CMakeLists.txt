set(DEMO_NAME DeferredShading)

# OpenGL
set(SHADERS_DIR_OGL ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/gl/win32/deferredshading)
set(SOURCE_OGL main_ogl.cpp)
set(SHADERS_OGL ${SHADERS_DIR_OGL}/gbuffer.fsh.glsl
				${SHADERS_DIR_OGL}/gbuffer.vsh.glsl
				${SHADERS_DIR_OGL}/deferredshading.vsh.glsl
				${SHADERS_DIR_OGL}/deferredshading.gsh.glsl
				${SHADERS_DIR_OGL}/deferredshading.fsh.glsl
				${SHADERS_DIR_OGL}/deferredshading_msaa.fsh.glsl)
				 
# Direct3D 11
set(SHADERS_DIR_DX11 ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/dx11/deferredshading)				
set(SOURCE_DX11 main_dx11.cpp)
set(SHADERS_DX11 ${SHADERS_DIR_DX11}/common.h.hlsl
				 ${SHADERS_DIR_DX11}/deferredshading.gsh.hlsl
				 ${SHADERS_DIR_DX11}/deferredshading.psh.hlsl
				 ${SHADERS_DIR_DX11}/deferredshading_msaa.psh.hlsl
				 ${SHADERS_DIR_DX11}/dscommon.h.hlsl
				 ${SHADERS_DIR_DX11}/gbuffer.psh.hlsl
				 ${SHADERS_DIR_DX11}/gbuffer.vsh.hlsl
				 ${SHADERS_DIR_DX11}/screenquad.vsh.hlsl
				 ${SHADERS_DIR_DX11}/skybox.gsh.hlsl
				 ${SHADERS_DIR_DX11}/skybox.psh.hlsl)

source_group(shaders FILES ${SHADERS_DX11})
source_group(shaders FILES ${SHADERS_OGL})
set(SOURCE_DX11 ${SOURCE_DX11} ${SHADERS_DX11})
set(SOURCE_OGL ${SOURCE_OGL} ${SHADERS_OGL})
INIT_DEMO(${DEMO_NAME} "${SOURCE_OGL}" "${SOURCE_DX11}")