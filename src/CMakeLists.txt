file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*/*.cc ${PROJECT_SOURCE_DIR}/src/*.cc)

add_library(redis STATIC ${SOURCES})

target_include_directories(redis PUBLIC ${PROJECT_SOURCE_DIR}/includes)

add_library(rds::lib ALIAS redis)