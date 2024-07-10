cmake_minimum_required(VERSION 3.28)
project(Dynamic_Containers_For_C C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Dynamic_Containers_For_C
        dynarray.h
        dynqueue.h
        dynstack.h
        test.c)
