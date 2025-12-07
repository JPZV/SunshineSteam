cmake_minimum_required(VERSION 3.10.0)
project(sunshinesteam VERSION 0.1.0 LANGUAGES C CXX)

# App

add_executable(SunshineSteam main.cpp
                             process_monitor.cpp)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/version.hpp
        @ONLY
    )
target_include_directories(SunshineSteam PUBLIC ${CMAKE_CURRENT_BINARY_DIR})