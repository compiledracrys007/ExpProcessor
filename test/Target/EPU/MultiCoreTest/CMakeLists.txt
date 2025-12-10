# Define the source files for the main executable
set(EPU_MULTI_CORE_TEST_SOURCES
    TestMultiCore.cpp
)

# Create the executable target
add_executable(test_epu_multicore ${EPU_MULTI_CORE_TEST_SOURCES})

target_link_libraries(test_epu_multicore 
    PRIVATE 
        utils
        parser
        simulator
        TargetEPU
)
