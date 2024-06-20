#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "bal.h"
#include <unistd.h>

void bal_entry_print(const bal_entry *e)
{
    uint8_t *raw = (uint8_t *)e;
    for (int i = 0; i < sizeof(bal_entry); i++)
    {
        printf("%02x", raw[i]);
    }
    printf("\n");
}

int bal_entry_cmp(const bal_entry *a, const bal_entry *b)
{
    if (a->v1 < b->v1)
    {
        return -1;
    }
    else if (a->v1 > b->v1)
    {
        return 1;
    }
    else
    {
        if (a->v2 < b->v2)
        {
            return -1;
        }
        else if (a->v2 > b->v2)
        {
            return 1;
        }
        else
        {
            if (a->v3 < b->v3)
            {
                return -1;
            }
            else if (a->v3 > b->v3)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
}

void bal_table_init(bal_table *t)
{
    t->fd = -1;
    t->size = 0;
    t->buf = NULL;
}

int bal_table_open(bal_table *t, const char *path)
{
    if (t->buf != NULL)
    {
        fprintf(stderr, "Table already open\n");
        return -1;
    }

    if ((t->fd = open(path, O_RDWR)) < 0)
    {
        perror("open");
        return -1;
    }

    if (fstat(t->fd, &t->st) < 0)
    {
        perror("fstat");
        return -1;
    }

    if (t->st.st_size % sizeof(bal_entry) != 0)
    {
        fprintf(stderr, "Invalid file size\n");
        return -1;
    }

    t->size = t->st.st_size / sizeof(bal_entry);

    if ((t->buf = mmap(NULL, t->st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, t->fd, 0)) == MAP_FAILED)
    {
        perror("mmap");
        return -1;
    }

    return 0;
}

void bal_table_sort(bal_table *t)
{
    if (t->buf == NULL)
    {
        fprintf(stderr, "Table not open\n");
        return;
    }

    qsort(t->buf, t->size, sizeof(bal_entry), (int (*)(const void *, const void *))bal_entry_cmp);
}

void bal_table_close(bal_table *t)
{
    if (t->buf == NULL)
    {
        fprintf(stderr, "Table not open\n");
        return;
    }

    if (munmap(t->buf, t->st.st_size) < 0)
    {
        perror("munmap");
    }

    t->buf = NULL;
    t->size = 0;

    close(t->fd);
}