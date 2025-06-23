cmake_minimum_required(VERSION 3.14)
project(delta_execution)

set(CMAKE_CXX_STANDARD 17)


add_subdirectory(strategies)
add_subdirectory(lib)


add_executable(delta_exec main.cpp)


target_link_libraries(delta_exec PRIVATE strategy optiongreeks)

target_include_directories(delta_exec PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/strategies
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)


add_executable(test_latency test_latency.cpp)
target_link_libraries(test_latency PRIVATE strategy optiongreeks)
target_include_directories(test_latency PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/strategies
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)