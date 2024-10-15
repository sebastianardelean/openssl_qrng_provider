/*
TODO: Use example from https://github.com/provider-corner/vigenere/tree/main

and https://github.com/openssl/openssl/blob/master/test/testutil/fake_random.c

AND documentation https://docs.openssl.org/3.0/man7/provider/#operations
https://github.com/openssl/openssl/issues/16784

Example RNG provider: https://docs.openssl.org/3.0/man7/provider-base/#examples

 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#define OPENSSL_NO_DEPRECATED
/*openssl*/
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/rand.h>

static int generate_key(EVP_PKEY **pkey);
static int generate_x509(EVP_PKEY *pkey, X509 **x509_cert);


int main(int argc, char **argv)
{
    printf("Generating RSA key...\n");
    EVP_PKEY *pkey = NULL;
    X509 *x509_cert = NULL;
    FILE *fp = NULL;
    
    int retvalue = 0;
   
    retvalue = generate_key(&pkey);
    
    if (retvalue != 0) {
        printf("Failed to generate RSA PKEY\n");
        return retvalue;
    }

    printf("Successfully generated RSA");

    printf("Saving private key to file...\n");

    fp = fopen("private_key.pem","wb");
    if (fp == NULL) {
        printf("Could not open file to write private key.\n");
        EVP_PKEY_free(pkey);
        return -1;
    }

    PEM_write_PrivateKey(fp, pkey, NULL,NULL,0,NULL,NULL);

    fclose(fp);
    
    printf("Generating x509 certificate...\n");
    retvalue = generate_x509(pkey, &x509_cert);
    if (retvalue !=0) {
        printf("Failed to generate X509 certificate.\n");
        EVP_PKEY_free(pkey);
        return retvalue;
    }

    printf("Saving certificate to file...\n");
    fp = fopen("certificate.pem","wb");
    if (fp == NULL) {
        printf("Could not open file to write private key.\n");
        EVP_PKEY_free(pkey);
        X509_free(x509_cert);
        return -1;
    }
 
    PEM_write_X509(fp,x509_cert);

    fclose(fp);
  
    EVP_PKEY_free(pkey);
    X509_free(x509_cert); 
    
    return retvalue;
    
    
}



int generate_key(EVP_PKEY **pkey)
{
    EVP_PKEY *pk = NULL;

    pk = EVP_RSA_gen(2048);
    
    if (NULL == pk) {
        printf("Unable to create EVP_PKEY structure.\n");
        *pkey = NULL;
        return -1;
    }

    
    *pkey=pk;
    return 0;
}


int generate_x509(EVP_PKEY *pkey, X509 **x509_cert)
{
    X509 *x509 = NULL;
    x509 = X509_new();
    if (NULL == x509) {
        printf("Unable to create x509 structure.\n");
        return -1;
    }

    /*Set the serial number*/
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

     /* This certificate is valid from now until exactly one year from now. */
    X509_gmtime_adj(X509_getm_notBefore(x509), 0);
    X509_gmtime_adj(X509_getm_notAfter(x509), 31536000L);

    /* Set the public key for our certificate. */
    X509_set_pubkey(x509, pkey);
    
    /* We want to copy the subject name to the issuer name. */
    X509_NAME * name = X509_get_subject_name(x509);

     /* Set the country code and common name. */
    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char *)"CA",        -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *)"MyCompany", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"localhost", -1, -1, 0);

     /* Now set the issuer name. */
    X509_set_issuer_name(x509, name);

    /* Actually sign the certificate with our key. */
    if(!X509_sign(x509, pkey, EVP_sha1()))
    {
        printf("Error signing certificate.\n");
        *x509_cert = NULL;
        X509_free(x509);
        return -1;
    }
    *x509_cert = x509;
    return 0;
}

