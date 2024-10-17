# Build

1. cmake .
2. make
3. mv libqrng.so qrng.so
4. openssl rand -provider-path . -provider qrng 100 


Check
https://github.com/qursa-uc3m/quantis-qrng-openssl-integration/blob/main/qrng_openssl_provider/quantis_qrng_provider.c


TODO: Use libqrng to stream bytes to a file descriptor/ shared memory while being a daemon. and in the provider, just read from the memory/fd.
Otherwise, by just using libqrng in the provider, curl will exit with out of memory.


Cmake for the provider to link libcurl and libqrng

```
# Include directories (if required)
include_directories(/usr/local/include)

# Find the library in a known path
find_library(QRNG_LIBRARY NAMES qrng PATHS /usr/local/lib)

# Check if the library was found and link it
if(QRNG_LIBRARY)
  target_link_libraries(qrngprov PRIVATE ${QRNG_LIBRARY})
else()
  message(FATAL_ERROR "libqrng not found!")
endif()

# Find libcurl
find_package(CURL REQUIRED)
target_link_libraries(qrngprov PRIVATE CURL::libcurl)

```


openssl list -providers
openssl genpkey -algorithm RSA -out private_key.pem -pkeyopt rsa_keygen_bits:2048 -provider qrngprov

https://adfinis.com/en/blog/openssl-x509-certificates/

Certificates:

openssl genpkey -algorithm RSA -out random.cs.upt.ro.key -pkeyopt rsa_keygen_bits:4096 -provider qrngprov

openssl req -new -sha256 -key random.cs.upt.ro.key -out random.cs.upt.ro.csr -provider qrngprov


openssl req -x509 -sha256 -nodes -newkey rsa:4096 -keyout random.cs.upt.ro.key -days 730 -out random.cs.upt.ro.pem -provider qrngprov
