cmake_minimum_required (VERSION 2.8)
project (libir)
include_directories("include")

SET(CMAKE_C_COMPILER avr-gcc)
#SET(CMAKE_CXX_COMPILER avr-gcc)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3")

add_library(ir STATIC src/ir.c)

add_executable(main test/main.c)
target_link_libraries(main ir)

