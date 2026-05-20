cmake_minimum_required(VERSION 3.20)
project(decimal_example CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    boost_decimal
    GIT_REPOSITORY https://github.com/cppalliance/decimal.git
    GIT_TAG v6.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(boost_decimal)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE Boost::decimal)
