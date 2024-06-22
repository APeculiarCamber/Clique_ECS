cmake_minimum_required(VERSION 3.28)
project(Clique_ECS)

set(CMAKE_CXX_STANDARD 23)

include_directories(ECS_src)

add_executable(Clique_ECS ECS_src/ECS_main.cpp)
