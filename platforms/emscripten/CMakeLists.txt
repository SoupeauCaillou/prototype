include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
link_directories (${PROJECT_BINARY_DIR}/sac)

file(
        GLOB
        source_files
        *.cpp *.h
)

add_executable(${PROJECT_NAME}.bc ${source_files})
target_link_libraries(${PROJECT_NAME}.bc ${PROJECT_NAME}common sac GLEW glfw openal sqlite3  vorbisfile vorbis z png)
add_custom_command(TARGET ${PROJECT_NAME}.bc
	POST_BUILD
	COMMAND EMCC_DEBUG=1 emcc -s WARN_ON_UNDEFINED_SYMBOLS=1 -O2 --llvm-lto 1 ${CMAKE_BINARY_DIR}/linux/${PROJECT_NAME}.bc -o /tmp/${PROJECT_NAME}.html --preload-file assets
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	COMMENT "Building emscripten HTML")
