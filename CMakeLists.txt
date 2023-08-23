cmake_minimum_required(VERSION 3.5.0)
project(vm VERSION 0.1.0 LANGUAGES C CXX)

# get all source (cpp) files
file(GLOB vm_src "*.cpp")

set(CMAKE_CXX_STANDARD 23)
add_executable(vm ${vm_src})
target_compile_options(vm PUBLIC -Wall -Wextra -Wswitch-enum)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
