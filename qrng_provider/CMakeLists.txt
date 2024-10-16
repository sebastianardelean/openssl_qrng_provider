cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(
  qrng-provider
  VERSION 1.2
  DESCRIPTION "QRNG provider in OpenSSL 3.0"
  LANGUAGES C)
set(CMAKE_C_STANDARD 99)
# I use my github handle here
add_compile_definitions(AUTHOR="@sebastianardelean")



if (NOT OPENSSL_FOUND)
  find_package(OpenSSL 3.0 REQUIRED)
endif() 




set(qrng_source qrng.c)
add_library(libqrng SHARED ${qrng_source})
set_target_properties(libqrng PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  OUTPUT_NAME "qrng")
target_include_directories(libqrng PUBLIC include)
target_include_directories(libqrng PRIVATE ${OPENSSL_INCLUDE_DIR}) 

# Testing
get_property(_is_multiconfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (_is_multiconfig)
  set(QRNG_MODULE_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
else()
  set(QRNG_MODULE_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
unset(_is_multiconfig)
message(STATUS "Determined the build time QRNG provider location to be '${QRNG_MODULE_DIRECTORY}'")

set(TEST_ENVIRONMENT
  "OPENSSL_MODULES=${QRNG_MODULE_DIRECTORY}"
  "OPENSSL_PROGRAM=${OPENSSL_PROGRAM}"
  "OPENSSL_RUNTIME_DIR=${OPENSSL_RUNTIME_DIR}"
  "OPENSSL_LIBRARY_DIR=$<IF:$<BOOL:${WIN32}>,${OPENSSL_RUNTIME_DIR},${OPENSSL_LIBRARY_DIR}>"
  "SOURCEDIR=${CMAKE_CURRENT_SOURCE_DIR}"
  "PERL5LIB=${CMAKE_CURRENT_SOURCE_DIR}/t"
  )


# No installation instruction, as this should never be used in production
# If you still want to do so, just copy vigenere.so / vigenere.dll to an
# appropriate location.
