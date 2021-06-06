cmake_minimum_required(VERSION 3.1)
project(simplify_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)

# Detecting appropriate compiler
if (APPLE)
  set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
  set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
elseif(UNIX) # implicit AND NOT APPLE
  set(CMAKE_CXX_COMPILER "g++-10")
  set(CMAKE_C_COMPILER "gcc-10")
endif()

# Finding appropriate packages
find_package(CGAL)

# Adding executables needed
add_executable(simplify_test ./simplify_test.cpp)

# Linking appropriate libraries required
target_link_libraries(
  simplify_test
  CGAL::CGAL
)
