# define all target executables
set(TARGETS "vasma" "vme" "dvasm" "vasm2x86_64")

# get all source (cpp) files and remove executable specific source files
file(GLOB src "*.cpp")
foreach(target ${TARGETS})
    list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/${target}.cpp")
endforeach()

# get all header (h) files and remove executable specific header files
file(GLOB include "${CMAKE_CURRENT_SOURCE_DIR}/../include")
foreach(target ${TARGETS})
    list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/../include/${target}.h")
endforeach()

# add the target executables
foreach(target ${TARGETS})
    add_executable(${target} ${src} "${target}.cpp")
endforeach()

# add the header files for each target executable
foreach(target ${TARGETS})
    target_include_directories(${target} PUBLIC ${include} "${CMAKE_CURRENT_SOURCE_DIR}/../include/${target}.h")
endforeach()
