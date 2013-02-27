include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/squirrel/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/sqrat/include)
link_directories (${PROJECT_BINARY_DIR}/sac)
link_directories (${PROJECT_BINARY_DIR}/sac/libs/squirrel/lib)

file(
        GLOB
        source_files
        *.cpp *.h
)

if (EMSCRIPTEN_BUILD)
	add_executable(fond_ecran.bc ${source_files})
	target_link_libraries(fond_ecran.bc fond_ecrancommon sac GLEW glfw openal sqlite3  vorbisfile vorbis z png)
	add_custom_command(TARGET fond_ecran.bc
		POST_BUILD
		COMMAND EMCC_DEBUG=1 emcc -s WARN_ON_UNDEFINED_SYMBOLS=1 -O2 --llvm-lto 1 ${CMAKE_BINARY_DIR}/linux/fond_ecran.bc -o /tmp/fond_ecran.html --preload-file assets
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR} 
		COMMENT "Building emscripten HTML")	
else ()
	add_executable(fond_ecran ${source_files})
	target_link_libraries(fond_ecran fond_ecrancommon sac GLEW glfw openal sqlite3  vorbisfile vorbis z vpx)
endif ()
