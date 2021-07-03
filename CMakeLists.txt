cmake_minimum_required(VERSION 3.10)
project(remake_risicv)

set(CMAKE_CXX_STANDARD 14)

add_executable(remake_risicv main.cpp my_tools.h program.h)