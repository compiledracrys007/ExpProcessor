# Define the source files for the main executable
set(EPU_PARALLEL_DISPATCH_TEST_SOURCES
    TestParallelDispatch.cpp
)

# Create the executable target
add_executable(test_epu_parallel_dispatch ${EPU_PARALLEL_DISPATCH_TEST_SOURCES})

target_link_libraries(test_epu_parallel_dispatch 
    PRIVATE 
        utils
        parser
        simulator
        TargetEPU
)
