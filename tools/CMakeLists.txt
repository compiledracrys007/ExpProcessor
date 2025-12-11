# Define the source files for the main executable
set(EPU_CODEGEN_TOOL_SOURCES
    EPUISAGenerator.cpp
)

# Create the executable target
add_executable(codegen_epu ${EPU_CODEGEN_TOOL_SOURCES})

target_link_libraries(codegen_epu 
    PRIVATE 
        utils
        parser
        simulator
        TargetEPU
)

