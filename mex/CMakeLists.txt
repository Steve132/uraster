include_directories("/usr/include/eigen3")
set(CMAKE_CXX_FLAGS "-std=c++11 -fopenmp")

add_library(uraster uraster_mex.cpp)

