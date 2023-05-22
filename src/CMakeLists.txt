file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*/*.cc ${PROJECT_SOURCE_DIR}/src/*.cc)

add_library(redis STATIC ${SOURCES})

target_include_directories(redis PUBLIC ${PROJECT_SOURCE_DIR}/includes)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
    target_compile_options(redis PUBLIC --coverage -pg)
    target_link_options(redis PUBLIC --coverage -pg)
else()
    set(CMAKE_BUILD_TYPE RELEASE)
    target_compile_definitions(redis PUBLIC -NDEBUG)
endif()

target_compile_options(redis PUBLIC -Wall -Werror)

target_link_libraries(redis PUBLIC lzfse snmalloc json11)

add_library(rds::lib ALIAS redis)