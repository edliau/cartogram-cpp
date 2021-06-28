if (MSVC)
  # warning level 4
  add_compile_options(/W4)
else()
  # lots of warnings
  add_compile_options(-Wall -Wextra -pedantic)
endif()

# code for findFFTW
configure_file(downloadFindFFTW.cmake.in findFFTW-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/findFFTW-download )
if(result)
  message(FATAL_ERROR "CMake step for findFFTW failed: ${result}")
else()
  message("CMake step for findFFTW completed (${result}).")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/findFFTW-download )
if(result)
  message(FATAL_ERROR "Build step for findFFTW failed: ${result}")
endif()

set(findFFTW_DIR ${CMAKE_CURRENT_BINARY_DIR}/findFFTW-src)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${findFFTW_DIR}")

# As per https://github.com/CGAL/cgal/wiki/How-to-use-CGAL-with-CMake-or-your-own-build-system
cmake_minimum_required(VERSION 3.1)
project(cartogram LANGUAGES CXX)

# setting appropriate flags
set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_BUILD_TYPE Release)

set(Boost_USE_STATIC_LIBS ON)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(FFTW_USE_STATIC_LIBS TRUE)

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
# find_package(Boost 1.70 COMPONENTS program_options REQUIRED)
find_package(FFTW)
find_package(Threads REQUIRED)
# find_package(OpenMP REQUIRED)

# Adding executables needed
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
  ../cartogram_generator/read_geojson.cpp
  ../cartogram_generator/fill_with_density.cpp
  ../cartogram_generator/blur_density.cpp
  ../cartogram_generator/flatten_density.cpp
  ../cartogram_generator/ft_real_2d.cpp
  ../cartogram_generator/interpolate_bilinearly.cpp
  ../cartogram_generator/project.cpp
  ../cartogram_generator/write_to_json.cpp
)

# testing out
message("\n\n\n")
# message(${CGAL::CGAL})
message("\n\n")

# FFTW
message(${FFTW_LIBRARIES})
message("\n\n")

string(REPLACE ".so"
       ".a;" FFTW_LIBRARIES
     ${FFTW_LIBRARIES})

message(${FFTW_LIBRARIES})
message("\n\n")

# BOOST
# message(${Boost_LIBRARIES})
# message("\n\n")
#
# string(REPLACE ".so"
#        ".a;" Boost_LIBRARIES
#      ${Boost_LIBRARIES})
#
# message(${Boost_LIBRARIES})
# message("\n\n")


# FFTW
message(${CMAKE_THREAD_LIBS_INIT})
message("\n\n")

# Linking appropriate libraries required
target_link_libraries(
  cartogram
  # -static
  # ${Boost_LIBRARIES}
  /usr/lib/x86_64-linux-gnu/libboost_program_options.a
  # OpenMP::OpenMP_CXX
  # CGAL::CGAL
  ${FFTW_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)
