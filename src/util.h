#ifndef __NUKETH_UTIL_H__
#define __NUKETH_UTIL_H__

#include <stdio.h>

static inline void fprint_hex(FILE *out, const void *data, size_t size)
{
    const unsigned char *p = (const unsigned char *)data;
    for (size_t i = 0; i < size; i++)
    {
        fprintf(out, "%02x", p[i]);
    }
    fprintf(out, "\n");
}

static inline void print_hex(const void *data, size_t size)
{
    const unsigned char *p = (const unsigned char *)data;
    for (size_t i = 0; i < size; i++)
    {
        printf("%02x", p[i]);
    }
    printf("\n");
}

static inline void decode_hex(const char *hex, void *data, size_t size)
{
    unsigned char *p = (unsigned char *)data;
    for (size_t i = 0; i < size; i++)
    {
        sscanf(hex + i * 2, "%2hhx", &p[i]);
    }
}

#endif
