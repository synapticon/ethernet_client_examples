# Top-level CMake project configuration file for EthernetClientExamples

# Specify the minimum required CMake version for compatibility
cmake_minimum_required(VERSION 3.28)

# Define Windows API target version as Windows 10/11 to enable corresponding features
if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

# Define the project name and specify it uses C++ language
project(EthernetClientExamples LANGUAGES CXX)

# Set the C++ standard to C++20 and require it to be used
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the path to loguru as a subdirectory
add_subdirectory(loguru)

# Find Boost with asio component
find_package(Boost REQUIRED COMPONENTS asio)

# Find nlohmann_json from vcpkg (it uses CONFIG mode)
find_package(nlohmann_json CONFIG REQUIRED)

# List all source files for the executable target
set(SOURCES
  src/main.cpp
  # Add additional source files here as needed
)

# Create an executable target from the source files
add_executable(main ${SOURCES})

# Add the include directory for the ethernet_client headers
# This allows the compiler to find header files under include/ethernet_client
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link the ethernet_client library based on the platform
if(WIN32)
  target_link_libraries(main PRIVATE
    $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/lib/ethernet_client_debug.lib>
    $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/lib/ethernet_client.lib>
    $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/lib/common_debug.lib>
    $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/lib/common.lib>
  )
else()
  target_link_libraries(main PRIVATE
    $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/lib/ethernet_client_debug.a>
    $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/lib/ethernet_client.a>
    $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/lib/common_debug.a>
    $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/lib/common.a>
  )
endif()

# Link against common libraries (Boost.Asio and nlohmann_json)
target_link_libraries(main PRIVATE
  Boost::asio
  nlohmann_json::nlohmann_json
  loguru
)
