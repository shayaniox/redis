#include "avl.h"
#include "client.h"
#include "hashtable.h"
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
        return server_run();
    }
    else if (strcmp(argv[1], "client") == 0 && argc >= 3) {
        return client_run(argc - 2, &argv[2]);
    }
    else if (strcmp((argv[1]), "avl") == 0) {
        __auto_type tree = (AvlTree){};

        AvlNode **nodes = calloc(7, sizeof(AvlNode *));
        int randnums[7] = {2, 22, 49, 15, 99, 27, 40}; // , 95, 35, 9, 24, 5, 66, 71, 32, 80, 97, 63, 81, 59};
        for (int i = 0; i < 7; i++) {
            nodes[i] = avlnew(randnums[i]);
            avlins(&tree, nodes[i]);
        }
        tree.root = avldel(tree.root->left);

        avlverify(tree.root);
        avlprint(&tree);
        avlfree(tree.root);
        free(nodes);
    }
    else {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
