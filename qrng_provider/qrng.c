
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <openssl/core_names.h>
#include <openssl/rand.h>
#include <openssl/provider.h>
#include <openssl/evp.h>
#include <openssl/types.h>
#include <malloc.h>
#include <stdint.h>

#define FILE_PATH "/tmp/datafile.bin"

#define DEBUG 1
#ifdef DEBUG
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt "%s", __FILE__, \
                            __LINE__, __func__, ##__VA_ARGS__, ""); } while (0)


#else
#define debug_print(fmt, ...)
#endif


static void request_numbers(size_t numbers, unsigned char *values) {
    memset(values, 0, numbers);
    int fd = open(FILE_PATH,O_RDWR);

    if (fd == -1) {
        return;
    }
    struct stat st;
    
    do {
        flock(fd, LOCK_EX);
        if (stat(FILE_PATH,&st) == -1) {
            break;
        }
        if (st.st_size > 0 && st.st_size > (numbers*sizeof(unsigned char))) {
            read(fd, values, numbers * sizeof(unsigned char));

            off_t new_size = st.st_size - (numbers * sizeof(unsigned char));
            if (new_size < 0 ) {
                new_size = 0;
            }else {
                ftruncate(fd,new_size);
            }
        }
        flock(fd, LOCK_UN);
        usleep(1000);
    }while(st.st_size < numbers * sizeof(unsigned char));
}


static const OSSL_ITEM reason_strings[] = {
    {0,NULL}
};

/*
 * Provider context
 */

struct provider_ctx_st {
    const OSSL_CORE_HANDLE *core_handle;
 
};


static void provider_ctx_free(struct provider_ctx_st *ctx)
{
    if (ctx != NULL) {
        free(ctx);
        ctx = NULL;
    }
}

static struct provider_ctx_st *provider_ctx_new (const OSSL_CORE_HANDLE *core,
                                                 const OSSL_DISPATCH *in)
    
{
    struct provider_ctx_st *ctx = NULL;
    if ((ctx = malloc(sizeof(*ctx))) !=  NULL) {
        ctx->core_handle = core;
    } else {
        provider_ctx_free(ctx);
    }

    return ctx;
}
    
/*
 * And the cute implementation
 */

static OSSL_FUNC_rand_newctx_fn qrng_newctx;
static OSSL_FUNC_rand_freectx_fn qrng_freectx;
static OSSL_FUNC_rand_instantiate_fn qrng_instantiate;
static OSSL_FUNC_rand_uninstantiate_fn qrng_uninstantiate;

static OSSL_FUNC_rand_generate_fn qrng_generate;
static OSSL_FUNC_rand_reseed_fn qrng_reseed;
static OSSL_FUNC_rand_nonce_fn qrng_nonce;
static OSSL_FUNC_rand_get_seed_fn qrng_get_seed;
static OSSL_FUNC_rand_clear_seed_fn qrng_clear_seed;
static OSSL_FUNC_rand_verify_zeroization_fn qrng_verify_zeroization;

static OSSL_FUNC_rand_enable_locking_fn qrng_enable_locking;
static OSSL_FUNC_rand_lock_fn qrng_lock;
static OSSL_FUNC_rand_unlock_fn qrng_unlock;

static OSSL_FUNC_rand_gettable_params_fn qrng_gettable_params;
static OSSL_FUNC_rand_gettable_ctx_params_fn qrng_gettable_ctx_params;
static OSSL_FUNC_rand_settable_ctx_params_fn qrng_settable_ctx_params;

static OSSL_FUNC_rand_get_params_fn qrng_get_params;
static OSSL_FUNC_rand_get_ctx_params_fn qrng_get_ctx_params;
static OSSL_FUNC_rand_set_ctx_params_fn qrng_set_ctx_params;






struct qrng_ctx_st {
    int state;
    const char *name;
};


static void *qrng_newctx(void *provctx, void *parent,
                         const OSSL_DISPATCH *parent_dispatch)
{
    debug_print("entered\n");
    struct qrng_ctx_st *ctx = malloc(sizeof(*ctx));
    if (ctx != NULL) {
        memset(ctx, 0, sizeof(*ctx));
        ctx->state = EVP_RAND_STATE_UNINITIALISED;
    }
    return ctx;
}

static void qrng_freectx(void *vrng)
{
    debug_print("entered\n");
    struct qrng_ctx_st *ctx = (struct qrng_ctx_st *)vrng;
    if (ctx != NULL) {
        free(ctx);
    }
}

static int qrng_instantiate(void *vrng,
                            ossl_unused unsigned int stength,
                            ossl_unused int prediction_resistance,
                            ossl_unused const unsigned char *pstr,
                            size_t pstr_len,
                            ossl_unused const OSSL_PARAM params[])
{
    debug_print("entered\n");
    struct qrng_ctx_st *ctx = (struct qrng_ctx_st *)vrng;
    ctx->state = EVP_RAND_STATE_READY;
    return 1;
}

static int qrng_uninstantiate(void *vrng)
{
    debug_print("entered\n");
    struct qrng_ctx_st *ctx = (struct qrng_ctx_st *)vrng;
    ctx->state = EVP_RAND_STATE_UNINITIALISED;
    return 1;
}

static int qrng_generate(void *vrng,
                         unsigned char *out,
                         size_t outlen,
                         unsigned int strength,
                         int prediction_resistance,
                         const unsigned char *addin,
                         size_t addinlen)
{
    debug_print("entered\n");
    struct qrng_ctx_st *ctx = (struct qrng_ctx_st *)vrng;
   
    request_numbers(outlen, out);
    /*
    size_t i = 0;
    for (i = 0; i < outlen; i++) {
        out[i] = (unsigned char)(rand()%256);
        printf("%d ", out[i]);
    }
    */
    return 1;
}

static int qrng_reseed(void *vrng,
                       int prediction_resistance,
                       const unsigned char *ent,
                       size_t ent_len,
                       const unsigned char *addin,
                       size_t addin_len)
{
    debug_print("entered\n");
    return 1;
}


static size_t qrng_nonce(void *ctx,
                         unsigned char *out,
                         unsigned int outlen,
                         long unsigned int min_noncelen,
                         long unsigned int max_noncelen
                         )
{
    debug_print("entered\n");
    return 1;
}

static size_t qrng_get_seed(void *ctx, unsigned char **buffer,
                               int entropy, size_t min_len, size_t max_len,
                               int prediction_resistance,
                               const unsigned char *adin, size_t adin_len)
{
    debug_print("entered\n");
    return 1;
}

static void qrng_clear_seed(void *ctx, unsigned char *buffer, size_t b_len)
{
    debug_print("entered\n");
    
}

static int qrng_verify_zeroization(void *ctx)
{
    debug_print("entered\n");
    return 1;
}

 

static int qrng_enable_locking(void *ctx)
{
    debug_print("entered\n");
    return 1;
}


static int qrng_lock(void *ctx)
{
    debug_print("entered\n");
    return 1;
}
static void qrng_unlock(void *ctx)
{
    debug_print("entered\n");
}

static const OSSL_PARAM *qrng_gettable_params(void *provctx)
{
    static const OSSL_PARAM table[] = {
        {NULL, 0, NULL, 0, 0}
    };
    return table;
}

static const OSSL_PARAM *qrng_gettable_ctx_params(void *ctx, void *provctx)
{
    debug_print("entered\n");
    static const OSSL_PARAM known_gettable_ctx_params[] = {
        OSSL_PARAM_int(OSSL_RAND_PARAM_STATE, NULL),
        OSSL_PARAM_uint(OSSL_RAND_PARAM_STRENGTH, NULL),
        OSSL_PARAM_size_t(OSSL_RAND_PARAM_MAX_REQUEST, NULL),
        OSSL_PARAM_END
    };
    return known_gettable_ctx_params;
}

static const OSSL_PARAM *qrng_settable_ctx_params(void *ctx, void *provctx){
    static const OSSL_PARAM table[] = {
        {NULL, 0, NULL, 0, 0}
    };
    return table;
}


static int qrng_get_params(OSSL_PARAM params[])
{
    debug_print("entered\n");
    return 1;
}

static int qrng_get_ctx_params(void *ctx, OSSL_PARAM params[])
{
    debug_print("entered\n");
    struct qrng_ctx_st *pctx = (struct qrng_ctx_st *)ctx;
    OSSL_PARAM *p;
    

    p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_STATE);
    if (p != NULL && !OSSL_PARAM_set_int(p, pctx->state))
        return 0;

    p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_STRENGTH);
    if (p != NULL && !OSSL_PARAM_set_int(p, 256))
        return 0;

    p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_MAX_REQUEST);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, INT_MAX))
        return 0;
    return 1;
}

static int qrng_set_ctx_params(void *ctx, const OSSL_PARAM params[])
{
    debug_print("entered\n");
    return 1;
}

/*
  Setup
 */
typedef void (*funcptr_t)(void);
/* Dispatch table */

static const OSSL_DISPATCH qrng_functions[] = {
    {OSSL_FUNC_RAND_NEWCTX, (funcptr_t)qrng_newctx},
    {OSSL_FUNC_RAND_FREECTX, (funcptr_t) qrng_freectx},
    {OSSL_FUNC_RAND_INSTANTIATE, (funcptr_t) qrng_instantiate},
    {OSSL_FUNC_RAND_UNINSTANTIATE, (funcptr_t) qrng_uninstantiate},
    {OSSL_FUNC_RAND_GENERATE, (funcptr_t) qrng_generate},
    {OSSL_FUNC_RAND_RESEED, (funcptr_t) qrng_reseed},
    {OSSL_FUNC_RAND_NONCE, (funcptr_t) qrng_nonce},
    {OSSL_FUNC_RAND_GET_SEED, (funcptr_t) qrng_get_seed},
    {OSSL_FUNC_RAND_CLEAR_SEED, (funcptr_t) qrng_clear_seed},
    {OSSL_FUNC_RAND_VERIFY_ZEROIZATION, (funcptr_t) qrng_verify_zeroization},
    {OSSL_FUNC_RAND_ENABLE_LOCKING, (funcptr_t) qrng_enable_locking},
    {OSSL_FUNC_RAND_LOCK, (funcptr_t) qrng_lock},
    {OSSL_FUNC_RAND_UNLOCK, (funcptr_t) qrng_unlock},
    {OSSL_FUNC_RAND_GETTABLE_PARAMS, (funcptr_t) qrng_gettable_params},
    {OSSL_FUNC_RAND_GETTABLE_CTX_PARAMS, (funcptr_t) qrng_gettable_ctx_params},
    {OSSL_FUNC_RAND_SETTABLE_CTX_PARAMS, (funcptr_t) qrng_settable_ctx_params},
    {OSSL_FUNC_RAND_GET_PARAMS, (funcptr_t) qrng_get_params},
    {OSSL_FUNC_RAND_GET_CTX_PARAMS, (funcptr_t) qrng_get_ctx_params},
    {0, NULL}

};

static const OSSL_ALGORITHM qrng_rand[] = {
    {"CTR-DRBG", "provider='qrngprov'", qrng_functions},
    {NULL, NULL, NULL}
};

static const OSSL_ALGORITHM *qrng_prov_query(void *vprovctx,
                                        int operation_id,
                                        int *no_cache)
{
    debug_print("entered with operation id %d\n", operation_id);
    switch(operation_id)
    {
    case OSSL_OP_CIPHER:
    {
        debug_print("OSSL_OP_CIPHER\n");   
        return qrng_rand;
    }
        
    case OSSL_OP_RAND:
    {
        debug_print("OSSL_OP_RAND\n");
        return qrng_rand;
    }
    }
    return NULL;
}

static const OSSL_ITEM *qrng_prov_get_reasons(void *provctx)
{
    debug_print("entered\n");
    return reason_strings;
}

static int qrng_prov_get_params(void *provctx, OSSL_PARAM *params)
{
    debug_print("entered\n");
    return 1;
}

static void qrng_prov_teardown(void *provctx)
{
    debug_print("entered\n");
    provider_ctx_free(provctx);
}

static const OSSL_DISPATCH provider_functions[] = {
    {OSSL_FUNC_PROVIDER_TEARDOWN, (funcptr_t)qrng_prov_teardown},
    {OSSL_FUNC_PROVIDER_QUERY_OPERATION, (funcptr_t)qrng_prov_query },
    {OSSL_FUNC_PROVIDER_GET_REASON_STRINGS,(funcptr_t)qrng_prov_get_reasons},
    {OSSL_FUNC_PROVIDER_GET_PARAMS, (funcptr_t)qrng_prov_get_params},
    {0,NULL}
};

int OSSL_provider_init(const OSSL_CORE_HANDLE *core,
                       const OSSL_DISPATCH *in,
                       const OSSL_DISPATCH **out,
                       void **vprovctx)
{
    debug_print("entered\n");
    if ((*vprovctx = provider_ctx_new(core,in)) == NULL) {
        return 0;
    }
    *out = provider_functions;
    return 1;
}
