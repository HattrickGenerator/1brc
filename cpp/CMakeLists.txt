project(1brc)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} analyze.cpp )

# Define the source directory of your script
set(SCRIPT_SOURCE "${CMAKE_SOURCE_DIR}/test.sh")

# Define the destination directory where you want to copy the script
set(SCRIPT_DEST "${CMAKE_CURRENT_BINARY_DIR}/test.sh")

# Copy the script to the build directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SCRIPT_SOURCE}"
            "${SCRIPT_DEST}"
    COMMENT "Copying script to build directory"
)
