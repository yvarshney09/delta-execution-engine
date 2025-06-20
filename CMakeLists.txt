cmake_minimum_required(VERSION 3.14)
project(delta_execution)

set(CMAKE_CXX_STANDARD 17)

# Add subdirectories
add_subdirectory(strategies)
add_subdirectory(lib)

# Main executable
add_executable(delta_exec main.cpp)

# Link strategy + lib
target_link_libraries(delta_exec PRIVATE strategy optiongreeks)
# Include directories
target_include_directories(delta_exec PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/strategies
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)