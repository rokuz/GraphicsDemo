set(DEMO_NAME Demo_OIT)

# OpenGL
set(SOURCE_OGL "skipped")

# Direct3D 11
set(SHADERS_DIR_DX11 ${CMAKE_CURRENT_SOURCE_DIR}/../../../data/shaders/dx11/oit)				
set(SOURCE_DX11 main_dx11.cpp)
set(SHADERS_DX11 ${SHADERS_DIR_DX11}/common.h.hlsl
				 ${SHADERS_DIR_DX11}/fragmentslist.psh.hlsl
				 ${SHADERS_DIR_DX11}/opaque.psh.hlsl
				 ${SHADERS_DIR_DX11}/opaque.vsh.hlsl
				 ${SHADERS_DIR_DX11}/pscommon.h.hlsl
				 ${SHADERS_DIR_DX11}/screenquad.gsh.hlsl
				 ${SHADERS_DIR_DX11}/screenquad.vsh.hlsl
				 ${SHADERS_DIR_DX11}/transparent.psh.hlsl)

source_group(shaders FILES ${SHADERS_DX11})
source_group(shaders FILES ${SHADERS_OGL})
set(SOURCE_DX11 ${SOURCE_DX11} ${SHADERS_DX11})
set(SOURCE_OGL ${SOURCE_OGL} ${SHADERS_OGL})
INIT_DEMO(${DEMO_NAME} "${SOURCE_OGL}" "${SOURCE_DX11}")