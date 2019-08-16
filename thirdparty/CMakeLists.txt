cmake_minimum_required(VERSION 3.0)

add_subdirectory(fmt EXCLUDE_FROM_ALL)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)
