cmake_minimum_required(VERSION 3.21)
project(ca_hw4)

set(CMAKE_CXX_STANDARD 14)

add_executable(ca_hw4 main.c core_api.cpp core_api.h sim_api.c sim_api.h)
