# get all 'vasm' source code files
file(GLOB examples_src "${CMAKE_SOURCE_DIR}/examples/*.vasm")

# the expected output of each example are in order with the '*.vasm' files in the examples directory
set(examples_outputs
    "4\n-5\n-1\n0\n5\n5\n13\n0\n140737488355327" # bitwise.vasm
    "2.71828" # e.vasm
    "0\n1\n1\n2\n3\n5\n8\n13\n21\n34\n55\n89\n144\n233\n377\n610\n987\n1597\n2584\n4181" # fibonacci.vasm
    "0.226565" # funcs.vasm
    "Hello, World!\n" # hello_world.vasm
    "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n60\n61\n62\n63\n64\n65\n66\n67\n68\n69\n70\n71\n72\n73\n74\n75\n76\n77\n78\n79\n80\n81\n82\n83\n84\n85\n86\n87\n88\n89\n90\n91\n92\n93\n94\n95\n96\n97\n98\n99\n100\n101\n102\n103\n104\n105\n106\n107\n108\n109\n110\n111\n112\n113\n114\n115\n116\n117\n118\n119\n120\n121\n122\n123\n124\n125\n126\n127\n-128\n-127\n-126\n-125\n-124\n-123\n-122\n-121\n-120\n-119\n-118\n-117\n-116\n-115\n-114\n-113\n-112\n-111\n-110\n-109\n-108\n-107\n-106\n-105\n-104\n-103\n-102\n-101\n-100\n-99\n-98\n-97\n-96\n-95\n-94\n-93\n-92\n-91\n-90\n-89\n-88\n-87\n-86\n-85\n-84\n-83\n-82\n-81\n-80\n-79\n-78\n-77\n-76\n-75\n-74\n-73\n-72\n-71\n-70\n-69\n-68\n-67\n-66\n-65\n-64\n-63\n-62\n-61\n-60\n-59\n-58\n-57\n-56\n-55\n-54\n-53\n-52\n-51\n-50\n-49\n-48\n-47\n-46\n-45\n-44\n-43\n-42\n-41\n-40\n-39\n-38\n-37\n-36\n-35\n-34\n-33\n-32\n-31\n-30\n-29\n-28\n-27\n-26\n-25\n-24\n-23\n-22\n-21\n-20\n-19\n-18\n-17\n-16\n-15\n-14\n-13\n-12\n-11\n-10\n-9\n-8\n-7\n-6\n-5\n-4\n-3\n-2\n-1\n0\n1" # memory.vasm
    "^0x" # native.vasm
    "2.2\n440000\n4\n-7562.31\n3.14159\n2" # nums.vasm
    "3.15149" # pi.vasm
    "10" # preprocessor.vasm
)

# get the length of the examples
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
    add_test(${example_name} ${CMAKE_BINARY_DIR}/src/vasma -i ${example_src} -o ${example_name}.vm)
    # do not pass test if there is a translation/compilation error
    set_property(TEST ${example_name} PROPERTY FAIL_REGULAR_EXPRESSION "ERROR")

    # check the output of the example
    add_test(${example_name}_run ${CMAKE_BINARY_DIR}/src/vme -i ${example_name}.vm)
    set_property(TEST ${example_name}_run PROPERTY PASS_REGULAR_EXPRESSION ${example_out})

    # remove the generated '.vm' file
    # taken from: https://stackoverflow.com/a/58136951
    add_test(NAME ${example_name}_clean COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/tests/${example_name}.vm)
endforeach()
# TODO: remove the generated target executables
