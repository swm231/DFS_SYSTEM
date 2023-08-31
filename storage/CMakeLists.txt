cmake_minimum_required(VERSION 2.8)

project(ftp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

file(GLOB_RECURSE native_srcs src/*.cpp)

add_executable(main ${native_srcs})

# Find Threads package
find_package(Threads)

# Link with MySQL client library
target_link_libraries(main mysqlclient)

# Link with OpenSSL libraries
find_package(OpenSSL REQUIRED)
target_link_libraries(main OpenSSL::SSL OpenSSL::Crypto)

# Link with pthread library
target_link_libraries(main Threads::Threads)

# Link with libconfig.h++ library
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCONFIGXX REQUIRED libconfig++)
include_directories(${LIBCONFIGXX_INCLUDE_DIRS})
link_directories(${LIBCONFIGXX_LIBRARY_DIRS})
target_link_libraries(main ${LIBCONFIGXX_LIBRARIES})

# Create a custom target for running Valgrind
find_program(VALGRIND "valgrind")
if(VALGRIND)
    add_custom_target(valgrind
        COMMAND ${VALGRIND} --leak-check=full --show-leak-kinds=all ./bin/main
        DEPENDS main
        COMMENT "Running Valgrind"
    )
endif()