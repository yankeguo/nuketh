#include <stdio.h>
#include <signal.h>

#include "keccak256/keccak256.h"
#include "uECC/uECC.h"

#include "bal.h"
#include "util.h"

#define PRIVATE_KEY_SIZE 32
#define PUBLIC_KEY_SIZE 64

int signal_caught = 0;

void handle_signal(int signal)
{
    printf("Caught signal %d\n", signal);
    signal_caught = 1;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    bal_table t;
    bal_table_init(&t);

    FILE *f_out;

    if ((f_out = fopen("result.txt", "a")) == NULL)
    {
        perror("fopen");
        return -1;
    }

    if (bal_table_open(&t, "nuketh.bal") != 0)
    {
        return -1;
    }

    uECC_Curve curve = uECC_secp256k1();
    uint8_t private_key[PRIVATE_KEY_SIZE];
    uint8_t public_key[PUBLIC_KEY_SIZE];
    SHA3_CTX public_key_hash_ctx;
    uint8_t public_key_hash[32];
    bal_entry *address = NULL;

    for (;;)
    {
        if (!uECC_make_key(public_key, private_key, curve))
        {
            printf("Failed to make key\n");
            return -1;
        }

        keccak_init(&public_key_hash_ctx);
        keccak_update(&public_key_hash_ctx, public_key, PUBLIC_KEY_SIZE);
        keccak_final(&public_key_hash_ctx, public_key_hash);

        address = (bal_entry *)&public_key_hash[12];

        if (bal_table_search(&t, address) >= 0)
        {
            printf("Collision found\n");
            print_hex(private_key, PRIVATE_KEY_SIZE);
            print_hex(address, sizeof(bal_entry));

            fprintf(f_out, "Private key: ");
            fprint_hex(f_out, private_key, PRIVATE_KEY_SIZE);
            fprintf(f_out, "Address: ");
            fprint_hex(f_out, address, sizeof(bal_entry));
            fflush(f_out);
            break;
        }

        if (signal_caught)
        {
            break;
        }
    }

    bal_table_close(&t);

    fclose(f_out);

    return 0;
}