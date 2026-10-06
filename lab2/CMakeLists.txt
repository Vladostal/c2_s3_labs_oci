add_executable(parent parent.c)
add_executable(child child.c)
add_dependencies(parent child)
