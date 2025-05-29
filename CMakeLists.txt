# Top-level CMake project configuration file for EthernetClientExamples

# Specify the minimum required CMake version for compatibility
cmake_minimum_required(VERSION 3.28)

# Include the vcpkg toolchain file
include(${CMAKE_TOOLCHAIN_FILE})

# Define the project name and specify it uses C++ language
project(EthernetClientExamples LANGUAGES CXX)

# Set the C++ standard to C++20 and require it to be used
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost with asio component
find_package(Boost REQUIRED COMPONENTS asio)

# Find nlohmann_json from vcpkg (it uses CONFIG mode)
find_package(nlohmann_json CONFIG REQUIRED)

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

# Link the ethernet_client library based on the platform
if(WIN32)
  target_link_libraries(main PRIVATE ${CMAKE_SOURCE_DIR}/lib/ethernet_client.lib)
else()
  target_link_libraries(main PRIVATE ${CMAKE_SOURCE_DIR}/lib/ethernet_client.a)
endif()

# Link against common libraries (Boost.Asio and nlohmann_json)
target_link_libraries(main PRIVATE
  Boost::asio
  nlohmann_json::nlohmann_json
)
