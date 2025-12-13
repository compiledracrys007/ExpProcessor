# Define the source files for the main executable
set(EPU_ACC_TEST_SOURCES
    TestAcc.cpp
)

# Create the executable target
add_executable(test_epu_acc ${EPU_ACC_TEST_SOURCES})

target_link_libraries(test_epu_acc 
    PRIVATE 
        utils
        parser
        simulator
        TargetEPU
)
