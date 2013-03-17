cmake_minimum_required(VERSION 2.6)

project(Prototype)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules_cmake/")

#set flags
#ADD_DEFINITIONS(-DDEBUG)
if (NOT EMSCRIPTEN_BUILD)
	ADD_DEFINITIONS(-DENABLE_LOG -DENABLE_PROFILING -DDATADIR="${CMAKE_SOURCE_DIR}/assets/")
	
	#Enable warnings
	if (MSVC) 
		# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	elseif(LINUX_BUILD)
	# else if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
		# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W")
		ADD_DEFINITIONS(-DLINUX=1)
	endif()
	
endif()

include_directories(${PROJECT_SOURCE_DIR}/sources)


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
