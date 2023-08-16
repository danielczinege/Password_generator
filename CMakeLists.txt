cmake_minimum_required(VERSION 3.22)
project(Password_generator C)

set(CMAKE_C_STANDARD 11)

add_executable(Password_generator
        main.c password_tools.c password_tools.h)

target_link_libraries(Password_generator PRIVATE m)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(Password_generator PRIVATE OpenSSL::SSL OpenSSL::Crypto)
