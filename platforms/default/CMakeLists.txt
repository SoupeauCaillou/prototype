include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/glm)
link_directories (${PROJECT_BINARY_DIR}/sac)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/sac/build/cmake/Modules")

file(
	GLOB
	source_files
	*.cpp *.h
)

add_executable(${PROJECT_NAME} ${source_files})
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}common sac)


#the necessary libs
set(LIBS_NEEDED GLEW GLFW OpenAL Sqlite3 VorbisFile Vorbis ZLIB AntTweakBar Ogg)

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
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries (${PROJECT_NAME} opengl32 Winmm)
elseif (APPLE)
   #SET(GUI_TYPE MACOSX_BUNDLE)
   INCLUDE_DIRECTORIES ( /Developer/Headers/FlatCarbon )
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(OPENGL_LIBRARY OpenGL )
   FIND_LIBRARY(IOKIT_LIBRARY IOKit )
   MARK_AS_ADVANCED (COCOA_LIBRARY
                     OPENGL_LIBRARY
                     IOKIT_LIBRARY)
   SET(EXTRA_LIBS ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY} c)
   target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS})
endif ()

#copy the libs .dll near the .exe
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(SAC_DLL_POSSIBLE_DIRS $ENV{SAC_DLLS_DIR} "${PROJECT_SOURCE_DIR}/../sac_dlls_dep")
	foreach (SAC_DLLS_DIR ${SAC_DLL_POSSIBLE_DIRS})
		if (EXISTS ${SAC_DLLS_DIR})
			set(BIN_DIR "${PROJECT_BINARY_DIR}/platforms/default/")
			foreach (BUILD_TYPE "Debug" "Release")
				add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_directory "${SAC_DLLS_DIR}"
			    "${BIN_DIR}/${BUILD_TYPE}")
			endforeach()
		endif()
	endforeach()
endif()

