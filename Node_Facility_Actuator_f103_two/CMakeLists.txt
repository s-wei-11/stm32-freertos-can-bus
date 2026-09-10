cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Node_Facility_Actuator_f103_two)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

file(GLOB CODE_APP CONFIGURE_DEPENDS "User/Application/*.c")
file(GLOB CODE_BSP CONFIGURE_DEPENDS "User/Board_Support_Package/*.c")
# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CODE_APP}
    ${CODE_BSP}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    User/Application
    User/Board_Support_Package
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -u _printf_float)

# 生成 .hex、.bin 文件并在编译结束时打印 Flash / RAM 占用大小
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    COMMENT "Building ${CMAKE_PROJECT_NAME}.hex and ${CMAKE_PROJECT_NAME}.bin"
)