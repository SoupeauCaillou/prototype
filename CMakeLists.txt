cmake_minimum_required(VERSION 2.6)

project(Prototype)

string (TOUPPER "${CMAKE_SYSTEM_NAME}" VarPlatform)

#set flags
if (NOT EMSCRIPTEN_BUILD)
	ADD_DEFINITIONS(-D${VarPlatform}=1 -DENABLE_LOG -DENABLE_PROFILING -DDATADIR="${CMAKE_SOURCE_DIR}/assets/")
	
	#Enable warnings - operating system dependent
	if (${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
		message ("Building for Windows [TODO add C++11 flag]")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		message ("Building for Linux")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -W -g -O0")
	else ()
		message (WARNING "Building for unknown platform: ${CMAKE_SYSTEM_NAME}")
		
	endif()
endif()

include_directories(${PROJECT_SOURCE_DIR}/sources)

#get the differents CMakeLists.txt
	#everyone
	add_subdirectory(sources)

	if (ANDROID_BUILD)
		add_subdirectory(platforms/android)
	elseif (EMSCRIPTEN_BUILD)
		add_subdirectory(platforms/emscripten)
	else ()
		#default version (linux - windows)
		add_subdirectory(platforms/default)
	endif()

	#sac
	add_subdirectory(sac/build/cmake)
