# Root/CMakeLists.txt

# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Define the project name
project(ProcessorSim CXX)

# Set the C++ standard for the entire project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add the subdirectories where targets (libraries and executables) are defined
add_subdirectory(lib)
add_subdirectory(test)
