#include <string.h>

#include "bal.h"

int main(int argc, char **argv)
{
    bal_table t = bal_table_new();

    if (bal_table_open(&t, "nuketh.bal") < 0)
    {
        return -1;
    }

    bal_table_sort(&t);

    bal_table_close(&t);

    return 0;
}