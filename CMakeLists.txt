cmake_minimum_required(VERSION 2.6)

project(Prototype)

#set flags
if (EMSCRIPTEN_BUILD)
	SET(CMAKE_CXX_FLAGS "-O0 -W -Wall -DDEBUG")
else ()
	SET(CMAKE_CXX_FLAGS "-O0 -g -W -Wall -DDEBUG -DENABLE_LOG -DENABLE_PROFILING -DDATADIR=\\\"${CMAKE_SOURCE_DIR}/assets/\\\"")
endif()

#get the differents CMakeLists.txt
	#everyone
	add_subdirectory(sources)

	if (ANDROID_BUILD)
		add_subdirectory(platforms/android)
	elif (EMSCRIPTEN_BUILD)
		add_subdirectory(platforms/emscripten)
	else ()
		#default version (pc - windows)
		add_subdirectory(platforms/default)
	endif()

	#sac
	add_subdirectory(sac/build/cmake)
