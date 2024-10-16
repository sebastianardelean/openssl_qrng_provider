/* openssl list -provider-path . -provider qrng -providers

    mv libqrng.so qrng.so
    openssl rand -provider-path . -provider qrng 1
 */
#include <string.h>
#include <openssl/core_names.h>
#include <openssl/rand.h>
#include <openssl/provider.h>
#include <openssl/evp.h>
#include <openssl/types.h>
#include <stdio.h>
#include <malloc.h>

/*
 * Pseudo random number generator of low quality but having repeatability
 * across platforms.  The two calls are replacements for random(3) and
 * srandom(3).
 */

static const OSSL_ITEM reasons[] ={
    {0,NULL}
};

typedef struct {
    int state;
    const char *name;
} FAKE_RAND;

static OSSL_FUNC_rand_newctx_fn fake_rand_newctx;
static OSSL_FUNC_rand_freectx_fn fake_rand_freectx;
static OSSL_FUNC_rand_instantiate_fn fake_rand_instantiate;
static OSSL_FUNC_rand_uninstantiate_fn fake_rand_uninstantiate;
static OSSL_FUNC_rand_generate_fn fake_rand_generate;
static OSSL_FUNC_rand_get_ctx_params_fn fake_rand_get_ctx_params;
static OSSL_FUNC_rand_enable_locking_fn fake_rand_enable_locking;
static OSSL_FUNC_rand_gettable_ctx_params_fn fake_rand_gettable_ctx_params;


static void *fake_rand_newctx(void *provctx, void *parent,
                              const OSSL_DISPATCH *parent_dispatch)
{
    printf("fake_rand_newctx\n");
    FAKE_RAND *r = OPENSSL_zalloc(sizeof(*r));

    if (r != NULL)
        r->state = EVP_RAND_STATE_UNINITIALISED;
    return r;
}

static void fake_rand_freectx(void *vrng)
{
    printf("fake_rand_freectx\n");
    OPENSSL_free(vrng);
}

static int fake_rand_instantiate(void *vrng, ossl_unused unsigned int strength,
                                 ossl_unused  int prediction_resistance,
                                 ossl_unused const unsigned char *pstr,
                                 size_t pstr_len,
                                 ossl_unused const OSSL_PARAM params[])
{
    printf("fake_rand_instantiate\n");
    FAKE_RAND *frng = (FAKE_RAND *)vrng;

    frng->state = EVP_RAND_STATE_READY;
    return 1;
}

static int fake_rand_uninstantiate(void *vrng)
{
    FAKE_RAND *frng = (FAKE_RAND *)vrng;

    frng->state = EVP_RAND_STATE_UNINITIALISED;
    return 1;
}

static int fake_rand_generate(void *vrng, unsigned char *out, size_t outlen,
                              unsigned int strength, int prediction_resistance,
                              const unsigned char *adin, size_t adinlen)
{
    printf("fake_rand_generate\n");
    FAKE_RAND *frng = (FAKE_RAND *)vrng;
    (void)frng;
    printf("YES\n");
    size_t i =  0;
    for (i = 0; i < outlen; i++) {
        out[i] = (unsigned char)(rand() % 256); // Use a more secure method in production
    }
  
    return 1;
}

static int fake_rand_enable_locking(void *vrng)
{
    printf("fake_rand_enable_locking\n");
    return 1;
}

static int fake_rand_get_ctx_params(ossl_unused void *vrng, OSSL_PARAM params[])
{
    printf("fake_rand_get_ctx_params\n");
    FAKE_RAND *frng = (FAKE_RAND *)vrng;
    OSSL_PARAM *p;

    p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_STATE);
    if (p != NULL && !OSSL_PARAM_set_int(p, frng->state))
        return 0;

    p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_STRENGTH);
    if (p != NULL && !OSSL_PARAM_set_int(p, 256))
        return 0;

    p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_MAX_REQUEST);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, INT_MAX))
        return 0;
    return 1;
}

static const OSSL_PARAM *fake_rand_gettable_ctx_params(ossl_unused void *vrng,
                                                       ossl_unused void *provctx)
{
    printf("fake_rand_gettable_ctx_params\n");
    static const OSSL_PARAM known_gettable_ctx_params[] = {
        OSSL_PARAM_int(OSSL_RAND_PARAM_STATE, NULL),
        OSSL_PARAM_uint(OSSL_RAND_PARAM_STRENGTH, NULL),
        OSSL_PARAM_size_t(OSSL_RAND_PARAM_MAX_REQUEST, NULL),
        OSSL_PARAM_END
    };
    return known_gettable_ctx_params;
}


static const OSSL_DISPATCH fake_rand_functions[] = {
    { OSSL_FUNC_RAND_NEWCTX, (void (*)(void))fake_rand_newctx },
    { OSSL_FUNC_RAND_FREECTX, (void (*)(void))fake_rand_freectx },
    { OSSL_FUNC_RAND_INSTANTIATE, (void (*)(void))fake_rand_instantiate },
    { OSSL_FUNC_RAND_UNINSTANTIATE, (void (*)(void))fake_rand_uninstantiate },
    { OSSL_FUNC_RAND_GENERATE, (void (*)(void))fake_rand_generate },
      { OSSL_FUNC_RAND_ENABLE_LOCKING, (void (*)(void))fake_rand_enable_locking },
    { OSSL_FUNC_RAND_GETTABLE_CTX_PARAMS,
      (void(*)(void))fake_rand_gettable_ctx_params },
    { OSSL_FUNC_RAND_GET_CTX_PARAMS, (void(*)(void))fake_rand_get_ctx_params },
    { 0, NULL }

};

static const OSSL_ALGORITHM fake_rand_rand[] = {
    { "QRNG", "provider=qrng", fake_rand_functions },
    { NULL, NULL, NULL }
};

static const OSSL_ALGORITHM *fake_rand_query(void *provctx,
                                             int operation_id,
                                             int *no_cache)
{
    printf("fake_rand_query\n");
    *no_cache = 0;
    switch (operation_id) {
    case OSSL_OP_RAND: {
        printf("\treturn fake_rand_rand\n");
        return fake_rand_rand;
    }
    }
    return NULL;
}


static const OSSL_ITEM *p_reasons(void *provctx)
{
    printf("p_reasons\n");
    return reasons;
}

static void p_teardown(void *provctx)
{
    printf("P_Teardown\n");
    free(provctx);
}


/* Functions we provide to the core */
static const OSSL_DISPATCH fake_rand_method[] = {
    { OSSL_FUNC_PROVIDER_TEARDOWN, (void (*)(void))p_teardown },
    { OSSL_FUNC_PROVIDER_QUERY_OPERATION, (void (*)(void))fake_rand_query },
    {OSSL_FUNC_PROVIDER_GET_REASON_STRINGS, (void (*)(void))p_reasons},
    {0, NULL}
};

int OSSL_provider_init(const OSSL_CORE_HANDLE *handle,
                       const OSSL_DISPATCH *in,
                       const OSSL_DISPATCH **out,
                       void **provctx)
{
  
    printf("Init");
    *provctx = OSSL_LIB_CTX_new();
    
    RAND_set_DRBG_type(*provctx, "qrng", NULL,NULL,NULL);
    printf("Set DRBG\n");

    
    if (*provctx == NULL) {
        printf("Return NULL\n");
        return 0;
    }
    *out = fake_rand_method;
    printf("Return success\n");
    return 1;
}


