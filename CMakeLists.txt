cmake_minimum_required(VERSION 3.10)

project(hypervector CXX)

if(${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT})
  # install to the build directory by default
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "" FORCE)
endif()

option(HYPERVECTOR_BUILD_TESTS "Build test executable(s)" FALSE)

# header-only library
set(HYPERVECTOR_PUBLIC_HEADER
  hypervector.h
  hypervector_container.h
  hypervector_detail.h
  hypervector_print.h
  hypervector_view.h
)
add_library(hypervector INTERFACE ${HYPERVECTOR_PUBLIC_HEADER})
target_compile_features(hypervector INTERFACE cxx_std_11)
target_include_directories(hypervector INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
set_target_properties(hypervector PROPERTIES
  PUBLIC_HEADER "${HYPERVECTOR_PUBLIC_HEADER}"
)

export(TARGETS hypervector FILE hypervector-config.cmake)
install(EXPORT hypervector-config DESTINATION lib/cmake/hypervector)
install(TARGETS hypervector
  EXPORT hypervector-config
  PUBLIC_HEADER DESTINATION include
)

if(HYPERVECTOR_BUILD_TESTS)
  add_executable(hypervector_test hypervector_test.cpp)
  target_link_libraries(hypervector_test hypervector)
endif()
