file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*/*.cc ${PROJECT_SOURCE_DIR}/src/*.cc)

add_library(redis STATIC ${SOURCES})

target_include_directories(redis PUBLIC ${PROJECT_SOURCE_DIR}/includes)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
    set(ADD_TEST ON)
    target_compile_options(redis PUBLIC --coverage -pg)
    target_link_options(redis PUBLIC --coverage -pg)
else()
    set(CMAKE_BUILD_TYPE RELEASE)
    target_compile_options(redis PUBLIC -DNDEBUG)
endif()

target_compile_options(redis PUBLIC -Wall -Werror)

target_link_libraries(redis PUBLIC lzfse.so snmallocshim.so json11.a)

set(CMAKE_CXX_COMPILER_LAUNCHER env LD_PRELOAD=/home/jean/mypieces/rds/third/install/usr/local/lib/snmallocshim.so)

target_link_directories(redis PUBLIC /home/jean/mypieces/rds/third/install/usr/local/lib)
target_link_directories(redis PUBLIC /home/jean/mypieces/rds/third/install/usr/lib/x86_64-linux-gnu)

target_include_directories(redis PUBLIC /home/jean/mypieces/rds/third/install/usr/local/include)
target_include_directories(redis PUBLIC /home/jean/mypieces/rds/third/install/usr/include/x86_64-linux-gnu)