# Define the source files for the main executable
set(EPU_CODEGEN_TEST_SOURCES
    TestMatmulCodegen.cpp
)

# Create the executable target
add_executable(test_epu_mm_codegen ${EPU_CODEGEN_TEST_SOURCES})

target_link_libraries(test_epu_mm_codegen 
    PRIVATE 
        utils
        parser
        simulator
        TargetEPU
)
