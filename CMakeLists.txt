cmake_minimum_required(VERSION 3.13)

project(nuketh)

find_package (Threads)

set(CMAKE_C_STANDARD 11)

# nuketh-csv2bal
add_executable(nuketh-csv2bal src/csv2bal.c src/bal.c)

# nuketh
add_executable(nuketh src/main.c src/bal.c src/uECC/uECC.c src/keccak256/keccak256.c)
target_link_libraries (nuketh ${CMAKE_THREAD_LIBS_INIT})
