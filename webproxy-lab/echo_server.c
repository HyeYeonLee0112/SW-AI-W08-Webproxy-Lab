#include <stdio.h>
#include <stdlib.h>
#include "csapp.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }

    fprintf(stdout, "echo_server placeholder: listening on port %s\n", argv[1]);
    return 0;
}
