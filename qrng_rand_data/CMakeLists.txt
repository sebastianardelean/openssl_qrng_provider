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





# Locate Bison and Flex
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# Define paths to Flex and Bison input files in the config directory
set(BISON_INPUT_FILE "${CMAKE_SOURCE_DIR}/config/config.y")
set(FLEX_INPUT_FILE "${CMAKE_SOURCE_DIR}/config/config.l")

set(BISON_OUTPUT_FILE "${CMAKE_BINARY_DIR}/config.tab.c")
set(BISON_OUTPUT_HEADER "${CMAKE_BINARY_DIR}/config.tab.h")
set(FLEX_OUTPUT_FILE "${CMAKE_BINARY_DIR}/lex.yy.c")

# Run Bison and Flex to generate C source and header files
BISON_TARGET(Parser ${BISON_INPUT_FILE} ${BISON_OUTPUT_FILE} COMPILE_FLAGS "--defines=${BISON_OUTPUT_HEADER}")
FLEX_TARGET(Scanner ${FLEX_INPUT_FILE} ${FLEX_OUTPUT_FILE})
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)




# Add the producer executable
add_executable(qrng_daemon main.c cfg_read.c ${BISON_OUTPUT_FILE} ${FLEX_OUTPUT_FILE})



target_compile_options(qrng_daemon PRIVATE -Wall -Wextra -O2)


include_directories(
        /usr/local/include
${CMAKE_BINARY_DIR}  # Include generated headers
        )

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
