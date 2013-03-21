include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/squirrel/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/AntTweakBar/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/glog-gflags)
link_directories (${PROJECT_BINARY_DIR}/sac)
link_directories (${PROJECT_BINARY_DIR}/sac/libs/squirrel/lib)
link_directories (${PROJECT_SOURCE_DIR}/sac/libs/AntTweakBar/lib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/sac/build/cmake/Modules")


file(
	GLOB
	source_files
	*.cpp *.h
)

add_executable(${PROJECT_NAME} ${source_files})




find_package (GLEW)
	if (GLEW_FOUND)
		  include_directories(${GLEW_INCLUDE_DIR})
      target_link_libraries (${PROJECT_NAME} ${GLEW_LIBRARY})
	endif ()

	find_package (GLFW)
	if (GLFW_FOUND)
	  include_directories(${GLFW_INCLUDE_DIR})
      target_link_libraries (${PROJECT_NAME} ${GLFW_LIBRARY})
	endif ()

	find_package (OpenAL)
	if (OPENAL_FOUND)
		include_directories(${OPENAL_INCLUDE_DIR})
        target_link_libraries (${PROJECT_NAME} ${OPENAL_LIBRARY})
	endif ()

	find_package (Sqlite3)
	if (SQLITE3_FOUND)
	  include_directories(${SQLITE3_INCLUDE_DIR})
	  target_link_libraries (${PROJECT_NAME} ${SQLITE3_LIBRARY})
	endif ()

	find_package (VorbisFile)
	if (VORBISFILE_FOUND)
	  include_directories(${VORBISFILE_INCLUDE_DIR})
	  target_link_libraries (${PROJECT_NAME} ${VORBISFILE_LIBRARY})
    endif ()

	find_package (Vorbis)
	if (VORBIS_FOUND)
	  include_directories(${VORBIS_INCLUDE_DIR})
	  target_link_libraries (${PROJECT_NAME} ${VORBIS_LIBRARY})
	endif ()
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package (VPX)
	if (VPX_FOUND)
	  include_directories(${VPX_INCLUDE_DIR})
	  target_link_libraries (${PROJECT_NAME} ${VPX_LIBRARY})
	endif ()
endif()
	find_package (ZLIB)
	if (ZLIB_FOUND)
	  include_directories(${ZLIB_INCLUDE_DIR})
      target_link_libraries (${PROJECT_NAME} ${ZLIB_LIBRARY})
	endif ()

	find_package (AntTweakBar)
	if (ANTTWEAKBAR_FOUND)
	  include_directories(${ANTTWEAKBAR_INCLUDE_DIR})
      target_link_libraries (${PROJECT_NAME} ${ANTTWEAKBAR_LIBRARY})
	endif ()
	
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}common sac)
