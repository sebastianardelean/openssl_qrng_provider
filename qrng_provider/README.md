# Build

1. cmake .
2. make
3. mv libqrng.so qrng.so
4. openssl rand -provider-path . -provider qrng 100 
