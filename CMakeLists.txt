cmake_minimum_required(VERSION 3.20)
project(c2_s3_labs_oci C)

set(CMAKE_C_STANDARD 11)

add_executable(parent parent.c)
add_executable(child child.c)
add_dependencies(parent child)
