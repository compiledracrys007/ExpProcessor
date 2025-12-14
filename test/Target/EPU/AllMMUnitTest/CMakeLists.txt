# Define the source files for the main executable
set(EPU_ALL_MM_UNIT_TEST_SOURCES
    TestAllMMUnit.cpp
)

# Create the executable target
add_executable(test_epu_allmmunit ${EPU_ALL_MM_UNIT_TEST_SOURCES})

target_link_libraries(test_epu_allmmunit 
    PRIVATE 
        utils
        parser
        simulator
        TargetEPU
)
