cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    pybind11_example
    LANGUAGES CXX
    VERSION 0.1.0)

include(FetchContent)

set(PYBIND11_FINDPYTHON OFF)
unset(Python_EXECUTABLE CACHE)
find_package(Python COMPONENTS Interpreter Development REQUIRED)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v3.0.0)
FetchContent_MakeAvailable(pybind11)

pybind11_add_module(example example.cpp)
set_target_properties(example PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
