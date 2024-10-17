# openssl qrng provider

## Getting started

### Build and Install

1. Build and install the libqrng library. Detailed instructions are presented in the [README](https://github.com/sebastianardelean/libqrng)
2. In qrng_rand_data directory run the following commands
   + `mkdir build`
   + `cd build`
   + `cmake ..`
   + `make`
   + Run the executable in background: `./qrng_daemon &`
4. In qrng_provider directory run the following commands:
   + `mkdir build`
   + `cd build`
   + `cmake ..`
   + `make`
   + `mv libqrngprov.so qrngprov.so`
5. Run `openssl version -d` and copy the `openssl.cnf` into the location shown by the `openssl version -d` command. Example: `cp openssl.cnf /usr/lib/ssl/openssl.cnf`. Modify the module path from `[qrngprov_sec]` to point to the location of the qrngprov.so file. **BE AWARE: IT IS RECOMMENDED TO CREATE A BACKUP COPY OF THE ORIGINAL CONFIGURATION FILE FROM THE LOCATION POINTED BY THE OPENSSL COMMAND!**

### Use the qrng provider with openssl 3.0

1. List the available providers: `openssl list providers`
2. Create a RSA private key: `openssl genpkey -algorithm RSA -out private_key.pem -pkeyopt rsa_keygen_bits:2048 -provider qrngprov`
3. Create RSA key: `openssl genpkey -algorithm RSA -out example.com.key -pkeyopt rsa_keygen_bits:4096 -provider qrngprov`
4. Create CSR: `openssl req -new -sha256 -key example.com.key -out example.com.csr -provider qrngprov`
5. Create X509 certificate: `openssl req -x509 -sha256 -nodes -newkey rsa:4096 -keyout example.com.key -days 730 -out example.com.pem -provider qrngprov`

## Software Architecture