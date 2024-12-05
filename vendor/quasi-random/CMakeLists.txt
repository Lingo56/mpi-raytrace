cmake_minimum_required(VERSION 3.22.1)
project(quasi-random CXX)

add_library(quasirand INTERFACE)
target_compile_features(quasirand INTERFACE cxx_std_20)
target_include_directories(quasirand INTERFACE include)