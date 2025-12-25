# Define the source files for the main executable
set(EPU_ACC_CODEGEN_TEST_SOURCES
    TestMatmulAccCodegen.cpp
)

# Create the executable target
add_executable(test_epu_mm_acc_codegen ${EPU_ACC_CODEGEN_TEST_SOURCES})

target_link_libraries(test_epu_mm_acc_codegen
    PRIVATE
        utils
        parser
        simulator
        TargetEPU
)
