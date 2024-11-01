#ifndef QRNGD_H
#define QRNGD_H

extern int qrng_open(const char *device);

extern void qrng_close(void);

extern int qrng_random_int32(int32_t min, int32_t max, size_t samples, int32_t *buffer);

#endif /* QRNGD_H */
