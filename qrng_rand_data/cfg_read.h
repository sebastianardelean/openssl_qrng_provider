#ifndef CFG_READ_H
#define CFG_READ_H


void cfg_read_init(void);
void cfg_read_run(void);


void cfg_read_domain_address(char *domain_address);
void cfg_read_pool_size(size_t *pool_size);
void cfg_read_chunk_size(size_t *chunk_size);

void cfg_read_min_rng_value(int32_t *min_value);
void cfg_read_max_rng_value(int32_t *max_value);

#endif