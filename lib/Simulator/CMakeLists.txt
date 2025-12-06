# lib/Simulator/CMakeLists.txt

# Define the source files for the utility library
set(SIMULATOR_LIB_SOURCES 
    Simulator.cpp
)

# Create a static library named 'simulator'
# This name is crucial as the executable will link against it later.
add_library(simulator STATIC ${SIMULATOR_LIB_SOURCES})
