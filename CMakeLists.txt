# Top-level CMake project configuration file for EthernetClientExamples

# Specify the minimum required CMake version for compatibility
cmake_minimum_required(VERSION 3.28)

# Define the project name and specify it uses C++ language
project(EthernetClientExamples LANGUAGES CXX)

# Set the C++ standard to C++20 and require it to be used
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the include directory for the ethernet_client headers
# This allows the compiler to find header files under include/ethernet_client
include_directories(${CMAKE_SOURCE_DIR}/include)

# List all source files for the executable target
set(SOURCES
  src/main.cpp
  # Add additional source files here as needed
)

# Create an executable target from the source files
add_executable(main ${SOURCES})

# Link the appropriate static ethernet_client library based on the platform
if(WIN32)
  # On Windows, link the .lib static library file
  target_link_libraries(main PRIVATE ${CMAKE_SOURCE_DIR}/lib/ethernet_client.lib)
else()
  # On Unix/Linux/macOS, link the .a static library file
  target_link_libraries(main PRIVATE ${CMAKE_SOURCE_DIR}/lib/ethernet_client.a)
endif()
