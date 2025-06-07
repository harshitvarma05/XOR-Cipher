cmake_minimum_required(VERSION 3.21)
project(SecureFileEncryptionQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt’s auto-features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(SecureFileEncryptionQt
        main.cpp
        mainwindow.cpp
        mainwindow.ui
        Tree.cpp
        Encryptor.cpp
)

target_include_directories(SecureFileEncryptionQt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SecureFileEncryptionQt PRIVATE Qt6::Widgets)
