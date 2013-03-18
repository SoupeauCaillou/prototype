include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/squirrel/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/sqrat/include)
link_directories (${PROJECT_BINARY_DIR}/sac)
link_directories (${PROJECT_BINARY_DIR}/sac/libs/squirrel/lib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/sac/build/cmake/Modules")

file(
	GLOB
	source_files
	*.cpp *.h
)

add_executable(${PROJECT_NAME} ${source_files})

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}common sac)

	find_package (OpenAL)
	if (OPENAL_FOUND)
		include_directories(${OPENAL_INCLUDE_DIR})
        target_link_libraries (${PROJECT_NAME} ${OPENAL_LIBRARY})
	endif ()

	find_package (GLEW)
	if (GLEW_FOUND)
		  include_directories(${GLEW_INCLUDE_DIR})
	endif ()

	find_package (GLFW)
	if (GLFW_FOUND)
	  include_directories(${GLFW_INCLUDE_DIR})
	endif ()

	find_package (ZLIB)
	if (ZLIB_FOUND)
	  include_directories(${ZLIB_INCLUDE_DIRS})
      target_link_libraries (${PROJECT_NAME} ${ZLIB_LIBRARIES})
	endif ()

