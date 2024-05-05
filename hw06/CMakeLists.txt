cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

project(main LANGUAGES CXX)

add_executable(main main.cpp)

#find_package(OpenMP REQUIRED)
#target_link_libraries(main PUBLIC OpenMP::OpenMP_CXX)
set(TBB_DIR F:/vcpkg/packages/tbb_x64-windows/share/tbb)

find_package(TBB REQUIRED)
target_link_libraries(main PUBLIC TBB::tbb)

#find_package(benchmark REQUIRED)
#target_link_libraries(main PUBLIC benchmark::benchmark)
