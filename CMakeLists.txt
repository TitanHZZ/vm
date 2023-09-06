cmake_minimum_required(VERSION 3.5.0)
project(vm VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

# get all source (cpp) files and remove specific executable source files
file(GLOB src "*.cpp")
list(REMOVE_ITEM src
    "${CMAKE_CURRENT_SOURCE_DIR}/vasma.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vme.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dvasm.cpp"
)

set(CMAKE_CXX_STANDARD 20)
# taken from: https://stackoverflow.com/q/45955272
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
    # default debug flags are OK 
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wswitch-enum")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# main target (the vm executable that actually executes programs in '.vm' files)
add_executable(vme ${src} "vme.cpp")

# 'vasma' target (its the vasm assembler, converts 'vasm' files to 'vm' files)
add_executable(vasma ${src} "vasma.cpp")

# 'dvasm' target (vasm disassembler)
add_executable(dvasm ${src} "dvasm.cpp")

########### tests section ###########
file(GLOB examples_src "examples/*.vasm")
# the expected output of each example are in order with the '*.vasm' files in the examples directory
set(examples_outputs
    "4\n-5\n-1\n0\n5\n5\n13\n0" # bitwise.vasm
    "2.71828" # e.vasm
    "0\n1\n1\n2\n3\n5\n8\n13\n21\n34\n55\n89\n144\n233\n377\n610\n987\n1597\n2584\n4181" # fibonacci.vasm
    "0.226565" # funcs.vasm
    "^0x" # native.vasm
    "2.2\n440000\n4\n-7562.31\n3.14159" # nums.vasm
    "3.15149" # pi.vasm
    "10" # preprocessor.vasm
)

list(LENGTH examples_src examples_len)
math(EXPR examples_range "${examples_len} - 1")

# translate/compile all the examples
foreach(idx RANGE ${examples_range})
    # get elements for the lists
    list(GET examples_src ${idx} example_src)
    list(GET examples_outputs ${idx} example_out)

    # get the name of the example (without the path and extension)
    get_filename_component(example_name ${example_src} NAME_WE)
    # add the corresponding test
    add_test(${example_name} vasma -i ${example_src} -o ${example_name}.vm)
    # do not pass test if there is a translation/compilation error
    set_property(TEST ${example_name} PROPERTY FAIL_REGULAR_EXPRESSION "ERROR")

    # check the output of the example
    add_test(${example_name}_run vme -i ${example_name}.vm)
    set_property(TEST ${example_name}_run PROPERTY PASS_REGULAR_EXPRESSION ${example_out})

    # remove the generated '.vm' file
    add_test(${example_name}_clean rm -f ${example_name}.vm)
endforeach()
# remove the generated executables
add_test(executable_clean rm -f vasma vme dvasm)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
