# Define the source files for the main executable
set(EPU_BASIC_TEST_SOURCES
    TestBasic.cpp
)

# Create the executable target
add_executable(test_epu_basic ${EPU_BASIC_TEST_SOURCES})

target_link_libraries(test_epu_basic 
    PRIVATE 
        utils
        parser
        simulator
        TargetEPU
)
