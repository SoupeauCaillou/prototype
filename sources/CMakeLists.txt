include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/squirrel/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/sqrat/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/AntTweakBar/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/glog-gflags)
link_directories (${PROJECT_BINARY_DIR}/sac)
link_directories (${PROJECT_BINARY_DIR}/sac/libs/squirrel/lib)


link_directories (${PROJECT_BINARY_DIR}/sac)

file(
        GLOB
        source_files
        *.cpp *.h states/*.cpp
)

add_library(${PROJECT_NAME}common ${source_files})
