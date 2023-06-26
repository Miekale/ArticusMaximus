cmake_minimum_required(VERSION 3.25)
project(PID C)

set(CMAKE_C_STANDARD 11)

add_executable(PID main.c PID.c PID.h)
