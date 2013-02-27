include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/squirrel/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/sqrat/include)
link_directories (${PROJECT_BINARY_DIR}/sac)
link_directories (${PROJECT_BINARY_DIR}/sac/libs/squirrel/lib)

file(
	GLOB
	source_files
	*.cpp *.h
)

add_executable(${PROJECT_NAME} ${source_files})




find_package (GLEW)
if (GLEW_FOUND)
	  include_directories(${GLEW_INCLUDE_DIRS})
	  target_link_libraries (${PROJECT_NAME} ${GLEW_LIBRARIES})
endif ()

find_package (GLFW)
if (GLFW_FOUND)
  include_directories(${GLFW_INCLUDE_DIRS})
  target_link_libraries (${PROJECT_NAME} ${GLFW_LIBRARIES})
endif ()

find_package (OpenAL)
if (OPENAL_FOUND)
	include_directories(${OPENAL_INCLUDE_DIRS})
	target_link_libraries (${PROJECT_NAME} ${OPENAL_LIBRARIES})
endif ()

# find_package (Sqlite3)
# if (SQLITE3_FOUND)
  # include_directories(${SQLITE3_INCLUDE_DIRS})
  # target_link_libraries (${PROJECT_NAME} ${SQLITE3_LIBRARIES})
# endif ()

# find_package (VorbisFile)
# if (VORBISFILE_FOUND)
  # include_directories(${VORBISFILE_INCLUDE_DIRS})
  # target_link_libraries (${PROJECT_NAME} ${VORBISFILE_LIBRARIES})
# endif ()

# find_package (Vorbis)
# if (VORBIS_FOUND)
  # include_directories(${VORBIS_INCLUDE_DIRS})
  # target_link_libraries (${PROJECT_NAME} ${VORBIS_LIBRARIES})
# endif ()

# find_package (Vpx)
# if (VPX_FOUND)
  # include_directories(${VPX_INCLUDE_DIRS})
  # target_link_libraries (${PROJECT_NAME} ${VPX_LIBRARIES})
# endif ()

find_package (ZLIB)
if (ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
  target_link_libraries (${PROJECT_NAME} ${ZLIB_LIBRARIES})
endif ()



target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}common sac)

