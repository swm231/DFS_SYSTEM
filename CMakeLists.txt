cmake_minimum_required (VERSION 2.8)

project (ftp)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

file(GLOB_RECURSE native_srcs src/*.cpp)

# add_library(Server SHARED ${native_srcs})

target_link_libraries(main ${CMAKE_THREAD_LIBS_INIT})

add_executable(main ${native_srcs})

find_package(Threads)

target_link_libraries(main ${CMAKE_THREAD_LIBS_INIT})

# target_link_libraries(main mysqlclient)
