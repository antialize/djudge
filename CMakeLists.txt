cmake_minimum_required(VERSION 2.6)
add_definitions(-Wall -W)
include_directories(common)
add_subdirectory(common bin)
add_subdirectory(drone bin)

