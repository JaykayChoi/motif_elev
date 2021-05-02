cmake_minimum_required (VERSION 3.13)

project (Motif)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(/MP)

add_compile_definitions(UNICODE _UNICODE)

set(DEPENDENCIES elevator)
foreach(DEPENDENCY ${DEPENDENCIES})
	if (NOT EXISTS "${PROJECT_SOURCE_DIR}/lib/${DEPENDENCY}/CMakeLists.txt")
		message(FATAL_ERROR "lib/${DEPENDENCY} is not found.")
	endif()

    add_subdirectory("lib/${DEPENDENCY}" EXCLUDE_FROM_ALL)
endforeach()


add_executable(${CMAKE_PROJECT_NAME})
add_subdirectory(src)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME})

target_include_directories(
	${CMAKE_PROJECT_NAME} SYSTEM PRIVATE

	lib
)

include_directories(
	${CMAKE_PROJECT_NAME}

	elevator
)

target_link_libraries(
	${CMAKE_PROJECT_NAME} PRIVATE

	elevator
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(
	elevator

	PROPERTIES FOLDER Libraries
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

get_property(TARGET_SOURCE_FILES TARGET ${CMAKE_PROJECT_NAME} PROPERTY SOURCES)
source_group(TREE "${PROJECT_SOURCE_DIR}/src" FILES ${TARGET_SOURCE_FILES})




