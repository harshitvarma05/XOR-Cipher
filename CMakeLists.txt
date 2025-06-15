cmake_minimum_required(VERSION 3.21)
project(SecureFileEncryptionQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt’s auto-moc/uic/rcc
set(CMAKE_AUTOMOC     ON)
set(CMAKE_AUTOUIC     ON)
set(CMAKE_AUTORCC     ON)

# Tell CMake to find OpenSSL via vcpkg
find_package(OpenSSL REQUIRED)

# Find Qt6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Pull in all your backend sources (including new Crypto.cpp)
add_executable(SecureFileEncryptionQt
        backend/main.cpp
        backend/mainwindow.cpp
        backend/Tree.cpp
        backend/Encryptor.cpp
        backend/Crypto.cpp        # ← your new AES+RSA module
        backend/mainwindow.ui     # AUTOUIC will process this
)

# Wherever your headers live
target_include_directories(SecureFileEncryptionQt
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/backend
)

# Link Qt and OpenSSL
target_link_libraries(SecureFileEncryptionQt
        PRIVATE
        Qt6::Widgets
        OpenSSL::Crypto
        OpenSSL::SSL      # optional if you only ever call the Crypto API
)
