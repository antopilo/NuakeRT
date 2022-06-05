cmake_minimum_required(VERSION 3.6)

project(NuakeRT)
set (CMAKE_CXX_STANDARD 17)
set (CXX_STANDARD_REQUIRED ON)

add_subdirectory(NuakeRT/)

add_subdirectory(./Dependencies/NuakeRenderer)
include_directories(./Dependencies/NuakeRenderer)




