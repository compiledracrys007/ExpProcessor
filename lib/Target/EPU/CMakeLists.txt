# lib/Target/EPU/CMakeLists.txt

# Define the source files for the utility library
set(EPU_TARGET_SOURCES 
    Simulator/EPUSimulator.cpp
    Parser/EPUAsmParser.cpp
)

# Create a static library named 'TargetEPU'
# This name is crucial as the executable will link against it later.
add_library(TargetEPU STATIC ${EPU_TARGET_SOURCES})

