set(SOURCE_LIB  gl3w.h 
				glcorearb.h 
				glext.h 
				glxext.h 
				wglext.h
				gl3w.c
)

add_library(gl3w STATIC ${SOURCE_LIB})
