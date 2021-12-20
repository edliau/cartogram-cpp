# Lots of warnings.
add_compile_options(-Wall -Wextra -pedantic)

# As per https://github.com/CGAL/cgal/wiki/How-to-use-CGAL-with-CMake-or-your-own-build-system
cmake_minimum_required(VERSION 3.1)
project(cartogram LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_BUILD_TYPE Release)

# Finding appropriate packages.
find_package(OpenMP REQUIRED)
find_package(CGAL 5.3 REQUIRED)
find_package(Boost 1.70 REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)

# Adding Cairo library
# find_package(Cairo)

include_directories(PkgConfig::FFTW ${Cairo_INCLUDE_DIRS})
link_libraries(PkgConfig::FFTW)

#target_link_libraries(cartogram ${Cairo_LIBRARIES})


# Adding executables needed.
add_executable(
  cartogram
  ../cartogram_generator/main.cpp
  ../cartogram_generator/geo_div.cpp
  ../cartogram_generator/cartogram_info.cpp
  ../cartogram_generator/inset_state.cpp
  ../cartogram_generator/read_csv.cpp
  ../cartogram_generator/rescale_map.cpp
  ../cartogram_generator/write_eps.cpp
  ../cartogram_generator/colors.cpp
  ../cartogram_generator/auto_color.cpp
  ../cartogram_generator/check_topology.cpp
  ../cartogram_generator/matrix.cpp
  ../cartogram_generator/densify.cpp
  ../cartogram_generator/densification_points.cpp
  ../cartogram_generator/read_geojson.cpp
  ../cartogram_generator/fill_with_density.cpp
  ../cartogram_generator/blur_density.cpp
  ../cartogram_generator/flatten_density.cpp
  ../cartogram_generator/ft_real_2d.cpp
  ../cartogram_generator/interpolate_bilinearly.cpp
  ../cartogram_generator/project.cpp
  ../cartogram_generator/write_geojson.cpp
  ../cartogram_generator/albers_projection.cpp
  ../cartogram_generator/parse_arguments.cpp)

# Linking appropriate libraries required.
if(APPLE)

  # Using LLVM clang, installed via homebrew, instaed of Apple clang.
  if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")

    # Homebrew install path on arm64 (apple silicon, M1 etc.) machines.
    set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")
    set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
  else()

    # Homebrew install path on x86 machines (Intel).
    set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
  endif()

elseif(UNIX)
  set(CMAKE_CXX_COMPILER "g++-11")
  set(CMAKE_C_COMPILER "gcc-11")

  # set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lcairo")

endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lcairo")

target_link_libraries(
  cartogram
  ${Boost_LIBRARIES}
  CGAL::CGAL
  ${CMAKE_THREAD_LIBS_INIT}
  PkgConfig::FFTW
  OpenMP::OpenMP_CXX
)
