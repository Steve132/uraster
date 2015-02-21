include_directories("/usr/include/eigen3")

set(CMAKE_CXX_FLAGS "-std=c++11 -fopenmp")
add_executable(triangle main.cpp)
add_executable(bunny bunnystatic.cpp bunny.c)
add_executable(bunnyanim bunnyanim.cpp bunny.c)
target_link_libraries(bunnyanim X11 Xext)
