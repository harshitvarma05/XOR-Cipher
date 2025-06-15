cmake_minimum_required(VERSION 3.21)
project(SecureFileEncryptionQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC     ON)
set(CMAKE_AUTOUIC     ON)
set(CMAKE_AUTORCC     ON)

find_package(OpenSSL REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(SecureFileEncryptionQt
        backend/main.cpp
        backend/mainwindow.cpp
        backend/Tree.cpp
        backend/Encryptor.cpp
        backend/Crypto.cpp
        backend/mainwindow.ui
)

target_include_directories(SecureFileEncryptionQt
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/backend
)

target_link_libraries(SecureFileEncryptionQt
        PRIVATE
        Qt6::Widgets
        OpenSSL::Crypto
        OpenSSL::SSL
)
