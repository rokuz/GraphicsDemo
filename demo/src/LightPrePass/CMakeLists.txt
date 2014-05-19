set(DEMO_NAME LightPrePass)

# OpenGL
set(SOURCE_OGL "skipped")
				 
# Direct3D 11
set(SHADERS_DIR_DX11 ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/dx11/lpp)				
set(SOURCE_DX11 main_dx11.cpp)
set(SHADERS_DX11 ${SHADERS_DIR_DX11}/common.h.hlsl
				 ${SHADERS_DIR_DX11}/lightprepass.vsh.hlsl
				 ${SHADERS_DIR_DX11}/lightprepass.gsh.hlsl
				 ${SHADERS_DIR_DX11}/lightprepass.psh.hlsl
				 ${SHADERS_DIR_DX11}/lightprepass_msaa.psh.hlsl
				 ${SHADERS_DIR_DX11}/lppcommon.h.hlsl
				 ${SHADERS_DIR_DX11}/gbuffer.psh.hlsl
				 ${SHADERS_DIR_DX11}/gbuffer.vsh.hlsl
				 ${SHADERS_DIR_DX11}/scene.psh.hlsl
				 ${SHADERS_DIR_DX11}/scene_msaa.psh.hlsl
				 ${SHADERS_DIR_DX11}/scene.vsh.hlsl)

source_group(shaders FILES ${SHADERS_DX11})
source_group(shaders FILES ${SHADERS_OGL})
set(SOURCE_DX11 ${SOURCE_DX11} ${SHADERS_DX11})
set(SOURCE_OGL ${SOURCE_OGL} ${SHADERS_OGL})
INIT_DEMO(${DEMO_NAME} "${SOURCE_OGL}" "${SOURCE_DX11}")