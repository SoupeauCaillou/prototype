# - Find GLFW libraries by Gautier :-).
# This module defines the following variables:
#  GLFW_INCLUDE_DIRS - include directories for GLFW
#  GLFW_LIBRARIES - libraries to link against GLFW
#  GLFW_FOUND - true if GLFW has been found and can be used

#=============================================================================
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(GLFW_INCLUDE_DIR GL/glfw.h 
  HINTS
    ENV GLFWDIR
  PATH_SUFFIXES include/GL include
  PATHS 
  "$ENV{PROGRAMFILES}/../Program\ Files/glfw"
  ~/Library/Frameworks
  /Library/Frameworks
  /opt
)


find_library(GLEW_LIBRARY 
	NAMES GLEW glew32 glew glew32s 
  HINTS
    ENV GLEWDIR
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
  PATHS
  "$ENV{PROGRAMFILES}/../Program\ Files/glew"
  ~/Library/Frameworks
  /Library/Frameworks
)

find_library(GLFW_LIBRARY 
	NAMES GLFW GLFWDLL
  HINTS
    ENV GLFWDIR
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64 lib-msvc110
  PATHS
  "$ENV{PROGRAMFILES}/../Program\ Files/glfw"
  ~/Library/Frameworks
  /Library/Frameworks
)

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(GLFW
                                  REQUIRED_VARS GLFW_INCLUDE_DIR GLFW_LIBRARY)

mark_as_advanced(GLFW_LIBRARY GLFW_INCLUDE_DIR)