cmake_minimum_required(VERSION 3.22)
project(AeroCoreXRoot NONE)

# Root workspace CMake project.
# Add the real STM32CubeMX project from the nested I2C_comms folder.
add_subdirectory(I2C_comms)
