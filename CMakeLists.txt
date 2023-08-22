cmake_minimum_required(VERSION 3.22)
project(Password_generator C)

set(CMAKE_C_STANDARD 11)

add_executable(Password_generator
        main.c password_tools.c password_tools.h data_saving.c data_saving.h)

target_link_libraries(Password_generator PRIVATE m)

# Find OpenSSL and set its variables
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# Link against OpenSSL libraries (static)
target_link_libraries(Password_generator PRIVATE ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
