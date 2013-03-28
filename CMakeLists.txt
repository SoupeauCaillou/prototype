cmake_minimum_required(VERSION 2.6)

project(Prototype)

#list of the supported OS
SET(SUPPORTED_TARGETS linux android emscripten windows mac)

#if target = NULL then target = current OS
if (NOT TARGET)
    message("No target defined! Targets available: ${SUPPORTED_TARGETS}.
    Example: cmake . -DTARGET=linux")

    string (TOLOWER "${CMAKE_SYSTEM_NAME}" varplatform)
    string (TOUPPER "${CMAKE_SYSTEM_NAME}" VARPLATFORM)
    message("Assuming your OS platform: ${varplatform}")
else()
    string (TOLOWER "${TARGET}" varplatform)
    string (TOUPPER "${TARGET}" VARPLATFORM)
endif()

LIST(FIND SUPPORTED_TARGETS ${varplatform} target_in_list)
#check it's supported yet
if (target_in_list EQUAL -1)
    message(SEND_ERROR "${varplatform} not in supported targets: ${SUPPORTED_TARGETS}")

    return()
endif()

message ("Building for ${varplatform}")
add_definitions(-DSAC_${VARPLATFORM})


include(sac/build/cmake/toolchains/${varplatform}.cmake)
include(sac/build/cmake/CMakeLists.txt)


