cmake_minimum_required(VERSION 3.14)
project(s5)
set(CMAKE_C_STANDARD 17)

add_executable(s5 repl.c lisp.c obarray.c gc.c primitives.c printer.c reader.c eval.c)
target_link_libraries(s5 m)
