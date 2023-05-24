cmake_minimum_required(VERSION 3.15)

project(rds)

add_subdirectory(src)

add_subdirectory(test)

add_executable(main main.cc)

target_link_libraries(main PRIVATE redis)
