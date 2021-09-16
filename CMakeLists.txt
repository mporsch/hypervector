cmake_minimum_required(VERSION 3.0.0)

project(hypervector CXX)
set(CMAKE_CXX_STANDARD 11)

# header-only library
add_library(hypervector INTERFACE)
target_include_directories(hypervector INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
export(TARGETS hypervector FILE hypervector-config.cmake)

# test executable
add_executable(hypervector_test hypervector_test.cpp)
target_link_libraries(hypervector_test hypervector)
