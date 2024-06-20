#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "util.h"
#include "bal.h"

const char *CSV_HEADER_PREFIX = "address,";

int handle_csv(char *filename, FILE *out)
{
    FILE *in = NULL;
    if ((in = fopen(filename, "r")) == NULL)
    {
        perror("Error: Cannot open input file");
        return 0;
    }

    int ret = 1;

    char line[1024];

    uint8_t data[20];

    // check csv header
    if (!fgets(line, sizeof(line), in))
    {
        perror("Error: Cannot read input file");
        goto handle_csv_close;
    }

    if (strlen(line) < strlen(CSV_HEADER_PREFIX))
    {
        fprintf(stderr, "Error: Invalid csv header\n");
        goto handle_csv_close;
    }

    if (strncmp(line, CSV_HEADER_PREFIX, strlen(CSV_HEADER_PREFIX)) != 0)
    {
        fprintf(stderr, "Error: Invalid csv header\n");
        goto handle_csv_close;
    }

    // check line
    while (fgets(line, sizeof(line), in))
    {
        // 0x[a-zA-Z0-9]{40},
        // 2 + 40 + 1
        if (strlen(line) < 43)
        {
            fprintf(stderr, "Error: Invalid line length\n");
            goto handle_csv_close;
        }

        if (line[0] != '0' || line[1] != 'x' || line[42] != ',')
        {
            fprintf(stderr, "Error: Invalid line format\n");
            goto handle_csv_close;
        }

        decode_hex(line + 2, data, 20);

        fwrite(data, 1, 20, out);
    }

    if (ferror(in))
    {
        perror("Error: Cannot read input file");
        goto handle_csv_close;
    }

    ret = 0;

handle_csv_close:
    fclose(in);
    return ret;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <dir> <output.bal>\n", argv[0]);
        return 1;
    }

    char *dir_s = argv[1];
    char *out_s = argv[2];

    DIR *dir = NULL;

    if ((dir = opendir(dir_s)) == NULL)
    {
        fprintf(stderr, "Error: Cannot open directory %s\n", dir_s);
        return 1;
    }

    FILE *out = NULL;
    if ((out = fopen(out_s, "w")) == NULL)
    {
        perror("Error: Cannot open output file");
        return 1;
    };

    char filename[1024];

    for (;;)
    {
        struct dirent *entry = readdir(dir);

        if (entry == NULL)
        {
            if (errno != 0)
            {
                perror("Error: Cannot read directory");
                return 1;
            }
            break;
        }

        if (strlen(entry->d_name) == 0)
        {
            continue;
        }

        if (entry->d_name[0] == '.')
        {
            continue;
        }

        memset(filename, 0, sizeof(filename));

        strcat(filename, dir_s);
        strcat(filename, "/");
        strcat(filename, entry->d_name);

        printf("Processing %s\n", filename);

        if (handle_csv(filename, out) != 0)
        {
            return 1;
        }

        if (fflush(out) != 0)
        {
            perror("Error: Cannot write output file");
            return 1;
        }
    }

    if (fclose(out) != 0)
    {
        perror("Error: Cannot close output file");
        return 1;
    }

    bal_table t;
    bal_table_init(&t);

    if (bal_table_open(&t, out_s) != 0)
    {
        perror("Error: Cannot open output file");
        return 1;
    }

    int sample_base = t.size / 3;
    int sample_count = 5;

    printf("Sample:\n");

    for (int i = 0; i < sample_count; i++)
    {
        bal_entry_print(&t.buf[sample_base + i]);
    }

    printf("Sorting...\n");

    bal_table_sort(&t);

    printf("Sample after sorting:\n");

    for (int i = 0; i < sample_count; i++)
    {
        bal_entry_print(&t.buf[sample_base + i]);
    }

    bal_table_close(&t);

    return 0;
}