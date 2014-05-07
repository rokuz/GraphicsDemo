set(SOURCE_LIB  pch.h 
				PlatformHelpers.h
				DDSTextureLoader.h
				dds.h
				DDSTextureLoader.cpp
				ScreenGrab.h
				ScreenGrab.cpp
)

add_library(ddslib STATIC ${SOURCE_LIB})
