include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
link_directories (${PROJECT_BINARY_DIR}/sac)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/sac/build/cmake/Modules")

#the necessary libs
set(LIBS_NEEDED GLEW GLFW AntTweakBar)

#you shouldn't touch that
foreach(lib ${LIBS_NEEDED})
	string(TOUPPER ${lib} LIB) #in uppercase
	
	find_package (${lib})
	if (${LIB}_FOUND)
	  include_directories(${${LIB}_INCLUDE_DIR})
	endif ()
endforeach()

file(
        GLOB
        source_files
        *.cpp *.h states/*.cpp
)

add_library(${PROJECT_NAME}common ${source_files})
