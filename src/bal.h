#ifndef __BAL_H__
#define __BAL_H__

#include <sys/stat.h>
#include <stdlib.h>

typedef struct
{
    uint64_t v1;
    uint64_t v2;
    uint32_t v3;
} __attribute__((packed)) bal_entry;

void bal_entry_print(const bal_entry *e);

int bal_entry_cmp(const bal_entry *a, const bal_entry *b);

typedef struct
{
    int fd;
    struct stat st;
    off_t size;
    bal_entry *buf;
} bal_table;

void bal_table_init(bal_table *t);

int bal_table_open(bal_table *t, const char *path);

void bal_table_sort(bal_table *t);

void bal_table_close(bal_table *t);

#endif