# lib/Parser/CMakeLists.txt

# Define the source files for the utility library
set(PARSER_LIB_SOURCES 
    Parser.cpp
)

# Create a static library named 'parser'
# This name is crucial as the executable will link against it later.
add_library(parser STATIC ${PARSER_LIB_SOURCES})
