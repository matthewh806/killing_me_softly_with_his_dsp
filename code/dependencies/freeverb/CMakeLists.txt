cmake_minimum_required(VERSION 3.19)
project(freeverb VERSION 1.0.0)


file(GLOB freeverb_src
    delayline.hpp
    delayline.cpp
    allpass.hpp
    allpass.cpp
    comb.hpp
    comb.cpp
    denormals.h
    revmodel.hpp
    revmodel.cpp
    tuning.h
)

add_library(freeverb ${freeverb_src})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
