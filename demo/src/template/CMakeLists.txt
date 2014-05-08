set(DEMO_NAME Template)

# OpenGL
set(SHADERS_DIR_OGL ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/gl/win32/template)
set(SOURCE_OGL main_ogl.cpp)
set(SHADERS_OGL ${SHADERS_DIR_OGL}/shader.vsh.glsl
				${SHADERS_DIR_OGL}/shader.fsh.glsl)

# Direct3D 11
set(SHADERS_DIR_DX11 ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/dx11/template)				
set(SOURCE_DX11 main_dx11.cpp)
set(SHADERS_DX11 ${SHADERS_DIR_DX11}/shader.vsh.hlsl
				 ${SHADERS_DIR_DX11}/shader.psh.hlsl)
				 
source_group(shaders FILES ${SHADERS_DX11})
source_group(shaders FILES ${SHADERS_OGL})
set(SOURCE_DX11 ${SOURCE_DX11} ${SHADERS_DX11})
set(SOURCE_OGL ${SOURCE_OGL} ${SHADERS_OGL})
INIT_DEMO(${DEMO_NAME} "${SOURCE_OGL}" "${SOURCE_DX11}")