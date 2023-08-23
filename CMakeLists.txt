cmake_minimum_required(VERSION 3.5.0)
project(vm VERSION 0.1.0 LANGUAGES C CXX)

# get all source (cpp) files and remove specific executable source files
file(GLOB src "*.cpp")
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/vasma.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/vme.cpp")

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall -Wextra -Wswitch-enum)

# main target (the vm executable that actually executes programs in '.vm' files)
add_executable(vme ${src} "vme.cpp")

# 'vasma' target (its the vasm assembler, converts 'vasm' files to 'vm' files)
add_executable(vasma ${src} "vasma.cpp")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
