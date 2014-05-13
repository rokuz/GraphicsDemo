#sources
set(PRECOMPILED stdafx.cpp stdafx.h)
set(SOURCE_LIB  logger.h 
				utils.h 
				logger.cpp 
				utils.cpp
				timer.h
				timer.cpp
				profiler.h
				profiler.cpp
				inputkeys.h
				fpscounter.h
				fpscounter.cpp
)
source_group(core FILES ${SOURCE_LIB})
source_group(precompiled FILES ${PRECOMPILED})
add_library(utils STATIC ${PRECOMPILED} ${SOURCE_LIB})

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#headers search
include_directories(../mathlib)

#dependences
set(DEPS_LIBS mathlib)

#link libraries
target_link_libraries(utils ${DEPS_LIBS})

#precompiled headers
set_target_properties(utils PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties(utils PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
cotire(utils)