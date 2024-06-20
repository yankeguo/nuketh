cmake_minimum_required(VERSION 3.13)

project(nuketh)

set(CMAKE_C_STANDARD 11)

add_executable(nuketh-balsort src/bal.c src/balsort.c)