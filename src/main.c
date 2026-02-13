#include "client.h"
#include "hashtable.h"
#include "log.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(char *progname)
{
    fprintf(stderr, "Usage: %s\n", progname);
    fprintf(stderr, "   %s [client|server]\n", progname);
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "   %s client set key value\n", progname);
    fprintf(stderr, "   %s server\n", progname);
}

__auto_type gdata = (struct HashTable){};

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "server") == 0) {
        if (server_run() == -1) {
            return EXIT_FAILURE;
        }
    }
    else if (strcmp(argv[1], "client") == 0) {
        if (argc < 3) {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }

        if (client_run(argc - 2, &argv[2]) == -1) {
            return EXIT_FAILURE;
        }
    }
    else {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
