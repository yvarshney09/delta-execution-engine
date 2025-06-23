add_library(optiongreeks STATIC OptionGreeks.cpp )


target_include_directories(optiongreeks PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
