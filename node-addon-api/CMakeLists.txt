cmake_minimum_required(VERSION 3.15)
project(example)

find_package(unofficial-node-addon-api CONFIG REQUIRED)

add_library(example SHARED example.cpp)
target_link_libraries(example PRIVATE unofficial::node-addon-api::node-addon-api)

# Node addons need this extension to be loaded with `require("...")`
set_target_properties(example PROPERTIES PREFIX "" SUFFIX ".node")

target_compile_definitions(
  example
  PRIVATE -DNODE_ADDON_API_CPP_EXCEPTION=1
          -DNODE_ADDON_API_CPP_EXCEPTIONS_ALL=1)
