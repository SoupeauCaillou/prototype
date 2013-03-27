cmake_minimum_required(VERSION 2.6)

project(Prototype)

string (TOLOWER "${CMAKE_SYSTEM_NAME}" VarPlatform)
string (TOUPPER "${CMAKE_SYSTEM_NAME}" VARPLATFORM)

if (EMSCRIPTEN_BUILD) 
	set (VarPlatform "emscripten")
endif()

message ("Building for ${VarPlatform}")

include(sac/build/cmake/toolchains/${VarPlatform}.cmake)

ADD_DEFINITIONS(-DSAC_${VARPLATFORM})

add_subdirectory(sac/build/cmake)


#include_directories(sources)

#get the differents CMakeLists.txt
	#everyone
#	add_subdirectory(sources)

#	if (ANDROID_BUILD)
#		add_subdirectory(platforms/android)
#	elseif (EMSCRIPTEN_BUILD)
#		add_subdirectory(platforms/emscripten)
#	else ()
		#default version (linux - windows)
#		add_subdirectory(platforms/default)
#	endif()

	#sac
#	add_subdirectory(sac/build/cmake)
