cmake_minimum_required(VERSION 3.13)

project(nuketh)

set(CMAKE_C_STANDARD 11)

# nuketh-balsort
add_executable(nuketh-balsort src/bal.c src/main-balsort.c)

# nuketh
add_executable(nuketh src/main.c src/uECC/uECC.c src/keccak256.c)