#sources
set(TOOL_NAME geomconv)
set(SOURCE_TOOL geomconv.cpp)
add_executable(${TOOL_NAME} ${SOURCE_TOOL})

#preprocessor
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#link libraries
target_link_libraries(${TOOL_NAME} mathlib geomlib utils)

#headers search
include_directories(../mathlib ../geomlib ../utils)