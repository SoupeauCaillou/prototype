include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)

link_directories (${PROJECT_BINARY_DIR}/sac)

file(
        GLOB
        source_files
        *.cpp *.h states/*.cpp
)

add_library(fond_ecrancommon ${source_files})
