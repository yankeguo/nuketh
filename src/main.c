#include <stdio.h>
#include <signal.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>

#include "keccak256/keccak256.h"
#include "uECC/uECC.h"

#include "bal.h"
#include "util.h"

#define PRIVATE_KEY_SIZE 32
#define PUBLIC_KEY_SIZE 64
#define COUNTER_INTERVAL 5

atomic_long *_counter = NULL;

int _signal_caught = 0;

void handle_signal(int signal)
{
    printf("Caught signal %d\n", signal);
    _signal_caught = 1;
}

void routine_print_counter()
{
    while (!_signal_caught)
    {
        sleep(COUNTER_INTERVAL);
        printf("Working: %ld/s\n", atomic_exchange(_counter, 0) / COUNTER_INTERVAL);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <nuketh.bal> <result.txt>\n", argv[0]);
        return 1;
    }

    char *bal_s = argv[1];
    char *ret_s = argv[2];

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    bal_table t;
    bal_table_init(&t);

    FILE *f_out;

    if ((f_out = fopen(ret_s, "a")) == NULL)
    {
        perror("fopen");
        return -1;
    }

    if (bal_table_open(&t, bal_s) != 0)
    {
        return -1;
    }

    printf("Table opened\n");

    if (bal_table_check(&t) != 0)
    {
        return -1;
    }

    printf("Table checked\n");

    uECC_Curve curve = uECC_secp256k1();
    uint8_t private_key[PRIVATE_KEY_SIZE];
    uint8_t public_key[PUBLIC_KEY_SIZE];
    SHA3_CTX public_key_hash_ctx;
    uint8_t public_key_hash[32];
    bal_entry *address = NULL;

    _counter = malloc(sizeof(atomic_long));
    atomic_init(_counter, 0);

    pthread_t thread_counter;
    if (pthread_create(&thread_counter, NULL, (void *)routine_print_counter, NULL) != 0)
    {
        perror("pthread_create");
        return -1;
    }

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

        atomic_fetch_add(_counter, 1);

        if (_signal_caught)
        {
            break;
        }
    }

    bal_table_close(&t);

    fclose(f_out);

    return 0;
}