add_executable(lab4_parent src/parent.c)
add_executable(lab4_child src/child.c)

target_compile_definitions(lab4_parent PRIVATE CHILD_PATH="$<TARGET_FILE:lab4_child>")
add_dependencies(lab4_parent lab4_child)
