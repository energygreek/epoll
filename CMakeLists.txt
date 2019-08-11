cmake_minimum_required(VERSION 3.0)
project(main)
add_compile_options(-std=c++11)

add_subdirectory(thirdparty)
add_subdirectory(src)

#add_executable(server server.cpp)
#add_executable(client client.cpp)
#target_link_libraries(server fmt-header-only)
#target_link_libraries(client fmt-header-only)

