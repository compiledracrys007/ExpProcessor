# tools/CMakeLists.txt

# Define the source files for the main executable
set(TEST_UTILS_SOURCES
    TestUtils.cpp
)

# Create the executable target
add_executable(TestUtils ${TEST_UTILS_SOURCES})

# Link the executable against the 'utils' library
# CMake automatically finds the 'utils' target defined in lib/utils/CMakeLists.txt
target_link_libraries(TestUtils 
    PRIVATE 
        utils
        parser
)
