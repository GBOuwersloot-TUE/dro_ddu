cmake_minimum_required(VERSION 3.16)
project(DRO_DDU LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SCIP REQUIRED)
include_directories(${SCIP_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp")

add_executable(program ${SOURCES})

target_link_libraries(program ${SCIP_LIBRARIES})
target_include_directories(program PRIVATE ${CMAKE_SOURCE_DIR}/include)