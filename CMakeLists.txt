cmake_minimum_required(VERSION 3.10)
project(secure_file_encryption)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add all source files
add_executable(secure_file_encryption
        main.cpp
        Tree.cpp
        Encryptor.cpp
)

# Include current directory for headers
target_include_directories(secure_file_encryption PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
