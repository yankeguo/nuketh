#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "bal.h"
#include "util.h"

void bal_entry_print(const bal_entry *e)
{
    print_hex(e, sizeof(bal_entry));
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

int bal_table_check(const bal_table *t)
{
    if (t->buf == NULL)
    {
        fprintf(stderr, "Table not open\n");
        return -1;
    }

    if (t->size == 0)
    {
        return 0;
    }

    for (off_t i = 1; i < t->size; i++)
    {
        if (bal_entry_cmp(&t->buf[i - 1], &t->buf[i]) > 0)
        {
            fprintf(stderr, "Table not sorted\n");
            return 1;
        }
    }

    return 0;
}

off_t bal_table_search(const bal_table *t, const bal_entry *entry)
{
    if (t->buf == NULL)
    {
        fprintf(stderr, "Table not open\n");
        return -1;
    }

    bal_entry *ret = bsearch(entry, t->buf, t->size, sizeof(bal_entry), (int (*)(const void *, const void *))bal_entry_cmp);

    if (ret == NULL)
    {
        return -1;
    }
    else
    {
        return ret - t->buf;
    }
}