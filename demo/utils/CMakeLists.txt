#sources
set(SOURCE_LIB  logger.h 
				utils.h 
				logger.cpp 
				utils.cpp
				timer.h
				timer.cpp
				profiler.h
				profiler.cpp
)
source_group(utils FILES ${SOURCE_LIB})
add_library(utils STATIC ${SOURCE_LIB})

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#headers search
include_directories(../mathlib)

#dependences
set(DEPS_LIBS mathlib)

#link libraries
target_link_libraries(utils ${DEPS_LIBS})