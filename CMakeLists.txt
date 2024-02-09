# cmake_minimum_required(VERSION <specify CMake version here>)
project(Memory_Manager)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(Memory_Manager
        src/background.m3if.asset
        src/blaster0.m3if.asset
        src/blaster1.m3if.asset
        src/blaster2.m3if.asset
        src/blaster3.m3if.asset
        src/blaster4.m3if.asset
        src/error.m3if.asset
        src/flash.m3if.asset
        src/flipped.m3if.asset
        src/graphics.h
        src/main.c
        src/player.m3if.asset
        src/unflipped.m3if.asset
        src/warning.m3if.asset)
