set(DEMO_NAME Demo_Fur)

# OpenGL
set(SHADERS_DIR_OGL ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/gl/win32/fur)
set(SOURCE_OGL main_ogl.cpp)
set(SHADERS_OGL ${SHADERS_DIR_OGL}/furshells.vsh.glsl
				${SHADERS_DIR_OGL}/furshells.fsh.glsl
				${SHADERS_DIR_OGL}/furfins.vsh.glsl
				${SHADERS_DIR_OGL}/furfins.gsh.glsl
				${SHADERS_DIR_OGL}/furfins.fsh.glsl
				${SHADERS_DIR_OGL}/solid.vsh.glsl
				${SHADERS_DIR_OGL}/solid.fsh.glsl)

# Direct3D 11
set(SHADERS_DIR_DX11 ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/dx11/fur)				
set(SOURCE_DX11 main_dx11.cpp)
set(SHADERS_DX11 ${SHADERS_DIR_DX11}/common.h.hlsl
				 ${SHADERS_DIR_DX11}/furshells.vsh.hlsl
				 ${SHADERS_DIR_DX11}/furshells.psh.hlsl
				 ${SHADERS_DIR_DX11}/furfins.vsh.hlsl
				 ${SHADERS_DIR_DX11}/furfins.gsh.hlsl
				 ${SHADERS_DIR_DX11}/furfins.psh.hlsl
				 ${SHADERS_DIR_DX11}/solid.vsh.hlsl
				 ${SHADERS_DIR_DX11}/solid.psh.hlsl)
				 
source_group(shaders FILES ${SHADERS_DX11})
source_group(shaders FILES ${SHADERS_OGL})
set(SOURCE_DX11 ${SOURCE_DX11} ${SHADERS_DX11})
set(SOURCE_OGL ${SOURCE_OGL} ${SHADERS_OGL})
INIT_DEMO(${DEMO_NAME} "${SOURCE_OGL}" "${SOURCE_DX11}")