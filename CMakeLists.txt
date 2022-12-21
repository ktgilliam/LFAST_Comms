# Have to suppress this warning for sanity
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)
project(LFAST_DEVICE LANGUAGES C CXX VERSION 0.0.1)
cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
include(CTest)
enable_testing()
# add_subdirectory(src)
include_directories(include)
add_subdirectory(src)
add_subdirectory(test)