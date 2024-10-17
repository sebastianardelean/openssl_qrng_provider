# Minimum CMake version required
cmake_minimum_required(VERSION 3.20)

# Project name and version
project(qrng_daemon VERSION 1.0)

# Set the C standard to C99 (or C11 if preferred)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

if (NOT OPENSSL_FOUND)
  find_package(OpenSSL 3.0 REQUIRED)
endif() 


# Add the producer executable
add_executable(qrng_daemon main.c)



target_compile_options(qrng_daemon PRIVATE -Wall -Wextra -O2)


include_directories(/usr/local/include)

# Find the library in a known path
find_library(QRNG_LIBRARY NAMES qrng PATHS /usr/local/lib)

# Check if the library was found and link it
if(QRNG_LIBRARY)
  target_link_libraries(qrng_daemon PRIVATE ${QRNG_LIBRARY})
else()
  message(FATAL_ERROR "libqrng not found!")
endif()

# Find libcurl
find_package(CURL REQUIRED)
target_link_libraries(qrng_daemon PRIVATE CURL::libcurl)
