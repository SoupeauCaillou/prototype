# - Find VPX libraries by Gautier :-).
# This module defines the following variables:
#  VPX_INCLUDE_DIRS - include directories for VPX
#  VPX_LIBRARIES - libraries to link against VPX
#  VPX_FOUND - true if VPX has been found and can be used

find_path(VPX_INCLUDE_DIR vpx/vp8.h
  HINTS
    ENV VPXDIR
  PATH_SUFFIXES include/GL include
  PATHS 
  "$ENV{PROGRAMFILES}/../Program\ Files/VPX"
  ~/Library/Frameworks
  /Library/Frameworks
  /opt
  /usr/include/
)

find_library(VPX_LIBRARY 
	NAMES vpx vpxdll
  HINTS
    ENV VPXDIR
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64 lib-msvc110
  PATHS
  "$ENV{PROGRAMFILES}/../Program\ Files/VPX"
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/lib/
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VPX
                                  REQUIRED_VARS VPX_INCLUDE_DIR VPX_LIBRARY)

mark_as_advanced(VPX_LIBRARY VPX_INCLUDE_DIR)
