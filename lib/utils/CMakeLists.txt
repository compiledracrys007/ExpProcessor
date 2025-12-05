# lib/utils/CMakeLists.txt

# Define the source files for the utility library
set(UTILS_LIB_SOURCES 
    Utils.cpp
)

# Create a static library named 'utils'
# This name is crucial as the executable will link against it later.
add_library(utils STATIC ${UTILS_LIB_SOURCES})
