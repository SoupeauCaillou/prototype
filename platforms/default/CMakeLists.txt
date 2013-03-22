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


#the necessary libs
set(LIBS_NEEDED GLEW GLFW OpenAL Sqlite3 VorbisFile Vorbis Zlib AntTweakBar Gflags Glog)

#you shouldn't touch that
foreach(lib ${LIBS_NEEDED})
	string(TOUPPER ${lib} LIB) #in uppercase
	
	find_package (${lib})
	if (${LIB}_FOUND)
	  include_directories(${${LIB}_INCLUDE_DIR})
	  target_link_libraries (${PROJECT_NAME} ${${LIB}_LIBRARY})
	endif ()
endforeach()

#particular libs
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package (VPX)
	if (VPX_FOUND)
	  include_directories(${VPX_INCLUDE_DIR})
	  target_link_libraries (${PROJECT_NAME} ${VPX_LIBRARY})
	endif ()
endif()

	
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}common sac)
