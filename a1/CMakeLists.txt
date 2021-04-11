cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(cmpe279_assignment)

add_executable(client client.c)
target_compile_options(client PRIVATE 
    -Wall 
    -Werror
)

add_executable(server server.c)
target_compile_options(server PRIVATE 
    -Wall 
    -Werror
)
