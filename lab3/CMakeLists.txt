find_package(Threads REQUIRED)

add_executable(lab3_main src/main.c)
target_link_libraries(lab3_main Threads::Threads)

add_executable(generator src/generator.c)
