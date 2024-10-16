# Build

1. cmake .
2. make
3. mv libqrng.so qrng.so
4. openssl rand -provider-path . -provider qrng 100 


Check
https://github.com/qursa-uc3m/quantis-qrng-openssl-integration/blob/main/qrng_openssl_provider/quantis_qrng_provider.c
