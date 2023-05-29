cmake_minimum_required(VERSION 3.15)

project(rds)

add_subdirectory(src)

add_executable(redis-server redis-server.cc)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
    target_compile_options(redis-server PUBLIC --coverage -pg)
    target_link_options(redis-server PUBLIC --coverage -pg)
    add_subdirectory(test)
    message("redis: building debug...")
else()
    set(CMAKE_BUILD_TYPE RELEASE)
    target_compile_options(redis-server PUBLIC -DNDEBUG)
    message("redis: building release...")
endif()

target_link_libraries(redis-server PRIVATE redis)

add_executable(redis-cli redis-cli.cc)

target_link_libraries(redis-cli PRIVATE json11.a)

target_link_directories(redis-cli PRIVATE /home/jean/mypieces/rds/third/install/usr/lib/x86_64-linux-gnu)

target_include_directories(redis-cli PRIVATE /home/jean/mypieces/rds/third/install/usr/include/x86_64-linux-gnu)


