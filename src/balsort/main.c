/**
 * @file src/balsort/main.c
 * @author GUO YANKE
 * @date 2024-06-20
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    uint64_t v1;
    uint64_t v2;
    uint32_t v3;
} __attribute__((packed)) bal_entry;

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

int main(int argc, char **argv)
{
    int fd;
    if ((fd = open("nuketh.bal", O_RDWR)) < 0)
    {
        perror("open");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        perror("fstat");
        return -1;
    }

    if (st.st_size % sizeof(bal_entry) != 0)
    {
        fprintf(stderr, "Invalid file size\n");
        return -1;
    }

    off_t entry_count = st.st_size / sizeof(bal_entry);

    printf("entry_count: %lld\n", entry_count);

    bal_entry *data = NULL;

    if ((data = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
        perror("mmap");
        return -1;
    };

    qsort(data, entry_count, sizeof(bal_entry), (int (*)(const void *, const void *))bal_entry_cmp);

    if (munmap(data, st.st_size) < 0)
    {
        perror("munmap");
        return -1;
    }

    return 0;
}