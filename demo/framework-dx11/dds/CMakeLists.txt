set(SOURCE_LIB  pch.h 
				PlatformHelpers.h
				DDSTextureLoader.h
				dds.h
				DDSTextureLoader.cpp
)

add_library(ddslib STATIC ${SOURCE_LIB})
