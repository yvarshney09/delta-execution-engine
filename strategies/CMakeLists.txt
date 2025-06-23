add_library(strategy STATIC StrategyExecution.cpp)


target_include_directories(strategy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
)
