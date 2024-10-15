/*
TODO: Use example from https://github.com/provider-corner/vigenere/tree/main

and https://github.com/openssl/openssl/blob/master/test/testutil/fake_random.c

AND documentation https://docs.openssl.org/3.0/man7/provider/#operations
https://github.com/openssl/openssl/issues/16784

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
#include <openssl/provider.h>
#include <openssl/core.h>
#include <openssl/core_names.h>

static int generate_key(EVP_PKEY **pkey);

static int generate_x509(EVP_PKEY *pkey, X509 **x509_cert);

typedef int qrng_cb_t(unsigned char *out, size_t outlen,
                                    const char *name, EVP_RAND_CTX *ctx);

typedef struct {
    qrng_cb_t *cb;
    int state;
    const char *name;
    EVP_RAND_CTX *ctx;
} qrng_t;

static OSSL_FUNC_rand_newctx_fn qrng_rand_newctx;
static OSSL_FUNC_rand_freectx_fn qrng_rand_freectx;
static OSSL_FUNC_rand_instantiate_fn qrng_rand_instantiate;
static OSSL_FUNC_rand_uninstantiate_fn qrng_rand_uninstantiate;
static OSSL_FUNC_rand_generate_fn qrng_rand_generate;
static OSSL_FUNC_rand_enable_locking_fn qrng_rand_enable_locking;
static OSSL_FUNC_rand_get_ctx_params_fn qrng_rand_get_ctx_params;
static OSSL_FUNC_rand_gettable_ctx_params_fn qrng_rand_gettable_ctx_params;


static const OSSL_DISPATCH q_rand_functions[] = {
    { OSSL_FUNC_RAND_NEWCTX, (void (*)(void))qrng_rand_newctx },
    { OSSL_FUNC_RAND_FREECTX, (void (*)(void))qrng_rand_freectx },
    { OSSL_FUNC_RAND_INSTANTIATE, (void (*)(void))qrng_rand_instantiate },
    { OSSL_FUNC_RAND_UNINSTANTIATE, (void (*)(void))qrng_rand_uninstantiate },
    { OSSL_FUNC_RAND_GENERATE, (void (*)(void))qrng_rand_generate },
    { OSSL_FUNC_RAND_ENABLE_LOCKING, (void (*)(void))qrng_rand_enable_locking },
    { OSSL_FUNC_RAND_GETTABLE_CTX_PARAMS,
      (void(*)(void))qrng_rand_gettable_ctx_params },
    { OSSL_FUNC_RAND_GET_CTX_PARAMS, (void(*)(void))qrng_rand_get_ctx_params },
    {0, NULL}
};

static const OSSL_ALGORITHM q_rand_rand[] = {
    { "QRNG", "provider=qrng", q_rand_functions },
    { NULL, NULL, NULL }
};




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

void *qrng_rand_newctx(void *provctx, void *parent,
                       const OSSL_DISPATCH *parent_dispatch)
{
    qrng_t *rand = OPENSSL_zalloc(sizeof(*rand));
    if (rand != NULL) {
        rand->state = EVP_RAND_STATE_UNINITIALISED;
    }
    return rand;
}
                       
void qrng_rand_freectx(void *vrng)
{
    OPENSSL_free(vrng);
}

int qrng_rand_instantiate(void *vrng, ossl_unused unsigned int strength,
                                 ossl_unused  int prediction_resistance,
                                 ossl_unused const unsigned char *pstr,
                                 size_t pstr_len,
                                 ossl_unused const OSSL_PARAM params[])
{
    qrng_t *rng = (qrng_t *)vrng;
    rng->state = EVP_RAND_STATE_READY;
    return 1;
}

int qrng_rand_uninstantiate(void *vrng)
{
    qrng_t *rng = (qrng_t *)vrng;
    rng->state = EVP_RAND_STATE_UNINITIALISED;
    return 1;
}

int qrng_rand_generate(void *vrng, unsigned char *out, size_t outlen,
                              unsigned int strength, int prediction_resistance,
                              const unsigned char *adin, size_t adinlen)
{
    qrng_t *rng = (qrng_t*)vrng;
    size_t l;
    uint32_t r;
    if (rng->cb !=NULL) {
        return (*rng->cb)(out, outlen,rng->name, rng->ctx);
    }

    while (outlen > 0) {

        r = rand()%256; //HERE, get the bytes
        l = outlen < sizeof(r) ? outlen : sizeof(r);
        memcpy(out, &r, l);
        out += l;
        outlen -= l;
    }
    return 1;
}

int qrng_rand_enable_locking(void *vrng)
{
    return 1;
}

int qrng_rand_get_ctx_params(ossl_unused void *vrng, OSSL_PARAM params[])
{
    qrng_t * rng = (qrng_t *)vrng;
    OSSL_PARAM *p;

    p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_STATE);
    if (p != NULL && !OSSL_PARAM_set_int(p, rng->state))
        return 0;

    p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_STRENGTH);
    if (p != NULL && !OSSL_PARAM_set_int(p, 256))
        return 0;

    p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_MAX_REQUEST);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, INT_MAX))
        return 0;
    return 1;
}

const OSSL_PARAM *qrng_rand_gettable_ctx_params(ossl_unused void *vrng,
                                                       ossl_unused void *provctx)
{
    static const OSSL_PARAM known_gettable_ctx_params[] = {
        OSSL_PARAM_int(OSSL_RAND_PARAM_STATE, NULL),
        OSSL_PARAM_uint(OSSL_RAND_PARAM_STRENGTH, NULL),
        OSSL_PARAM_size_t(OSSL_RAND_PARAM_MAX_REQUEST, NULL),
        OSSL_PARAM_END
    };
    return known_gettable_ctx_params;
}
