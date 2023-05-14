cmake_minimum_required(VERSION 3.15)

project(rds)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
    add_compile_options(--coverage -pg)
    add_link_options(--coverage -pg)
else()
    set(CMAKE_BUILD_TYPE RELEASE)
endif()

add_compile_options(-Wall -Werror)

include_directories(includes)

add_subdirectory(third)

add_subdirectory(src)

add_subdirectory(test)
