# test/CMakeLists.txt

# Define the source files for the main executable
set(TOOLS_SOURCES
    TestEpuSimulator.cpp
)

# Create the executable target
add_executable(test_epu ${TOOLS_SOURCES})

# Link the executable against the 'utils' library
# CMake automatically finds the 'utils' target defined in lib/utils/CMakeLists.txt
target_link_libraries(test_epu 
    PRIVATE 
        utils
        parser
        simulator
        TargetEPU
)
