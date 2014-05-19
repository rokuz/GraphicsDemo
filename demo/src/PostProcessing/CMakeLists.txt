set(DEMO_NAME Demo_PostProcessing)

# OpenGL
set(SHADERS_DIR_OGL ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/gl/win32/postprocessing)
set(SOURCE_OGL main_ogl.cpp)
set(SHADERS_OGL ${SHADERS_DIR_OGL}/screenquad.vsh.glsl
				${SHADERS_DIR_OGL}/screenquad.gsh.glsl
				${SHADERS_DIR_OGL}/screenquad.fsh.glsl
				${SHADERS_DIR_OGL}/screenquad_msaa.fsh.glsl
				${SHADERS_DIR_OGL}/scene.vsh.glsl
				${SHADERS_DIR_OGL}/scene.fsh.glsl)
				 
# Direct3D 11			
set(SOURCE_DX11 "skipped")

source_group(shaders FILES ${SHADERS_DX11})
source_group(shaders FILES ${SHADERS_OGL})
set(SOURCE_DX11 ${SOURCE_DX11} ${SHADERS_DX11})
set(SOURCE_OGL ${SOURCE_OGL} ${SHADERS_OGL})
INIT_DEMO(${DEMO_NAME} "${SOURCE_OGL}" "${SOURCE_DX11}")