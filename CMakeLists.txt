cmake_minimum_required(VERSION 3.20)
project(RogueLike CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RogueLike
        main.cpp
        RogueLike.cpp
        RogueLike.h
)