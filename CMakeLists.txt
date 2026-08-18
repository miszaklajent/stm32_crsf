cmake_minimum_required(VERSION 3.22)

# CRSF STM32 Driver Library
add_library(crsf STATIC
    Src/crsf.c
)

# Export the include directory so any target linking `crsf` gets headers automatically
target_include_directories(crsf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
)

# Link to STM32 HAL / CMSIS dependencies provided by stm32cubemx target
target_link_libraries(crsf PUBLIC
    stm32cubemx
)
