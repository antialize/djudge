cmake_minimum_required(VERSION 2.6)
add_definitions(-Wall -W)

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(common)
add_subdirectory(common bin)
add_subdirectory(drone bin)

