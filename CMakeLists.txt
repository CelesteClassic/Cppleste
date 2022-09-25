cmake_minimum_required(VERSION 3.6)
project(Cppleste)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(SOURCE_FILES Carts/Celeste.cpp Carts/Celeste.h PICO8.h CelesteUtils.h Searcheline.h ThreadedSearcheline.h)

add_library(Cppleste STATIC ${SOURCE_FILES})
