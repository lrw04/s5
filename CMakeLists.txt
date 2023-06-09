cmake_minimum_required(VERSION 3.14)
project(s5)
set(CMAKE_C_STANDARD 17)

add_executable(s5 main.c lisp.c obarray.c gc.c primitives.c printer.c)
