cmake_minimum_required(VERSION 3.22)
project(aerocorex LANGUAGES C ASM)

# The workspace root is a wrapper for the STM32 project in the nested folder.
# CMake Tools expects a real target to exist in the root project, so we create a
# minimal placeholder source here while the actual firmware target remains below.
add_executable(aerocorex dummy.c)

# Add the real STM32CubeMX project from the nested I2C_comms folder.
add_subdirectory(I2C_comms)
