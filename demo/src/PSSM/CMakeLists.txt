set(DEMO_NAME Demo_PSSM)

# OpenGL
set(SHADERS_DIR_OGL ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/gl/win32/pssm)
set(SOURCE_OGL main_ogl.cpp)
set(SHADERS_OGL ${SHADERS_DIR_OGL}/scene.vsh.glsl
				${SHADERS_DIR_OGL}/scene.fsh.glsl
				${SHADERS_DIR_OGL}/shadowmap.vsh.glsl
				${SHADERS_DIR_OGL}/shadowmap.gsh.glsl)

# Direct3D 11
set(SHADERS_DIR_DX11 ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/dx11/pssm)				
set(SOURCE_DX11 main_dx11.cpp)
set(SHADERS_DX11 ${SHADERS_DIR_DX11}/common.h.hlsl
				 ${SHADERS_DIR_DX11}/scene.vsh.hlsl
				 ${SHADERS_DIR_DX11}/scene.psh.hlsl
				 ${SHADERS_DIR_DX11}/shadowmap.vsh.hlsl
				 ${SHADERS_DIR_DX11}/shadowmap.gsh.hlsl
				 ${SHADERS_DIR_DX11}/shadowmap.psh.hlsl)
				 
source_group(shaders FILES ${SHADERS_DX11})
source_group(shaders FILES ${SHADERS_OGL})
set(SOURCE_DX11 ${SOURCE_DX11} ${SHADERS_DX11})
set(SOURCE_OGL ${SOURCE_OGL} ${SHADERS_OGL})
INIT_DEMO(${DEMO_NAME} "${SOURCE_OGL}" "${SOURCE_DX11}")