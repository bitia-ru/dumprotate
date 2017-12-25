#include <stdio.h> // puts

#include "dumprotate.h"

int dr_help() {
    puts("Usage: dumprot [OPTION...]");
    puts("");
    printf("\t-h, --help\tdisplay this help and exit\n");
    printf("\t-s\t\tlimit on the maximum storage size\n");
    printf("\t\t\t\tSIZE — number of bytes,\n");
    printf("\t\t\t\tSIZEk/SIZEK — number of KB,\n");
    printf("\t\t\t\tSIZEm/SIZEM — number of MB,\n");
    printf("\t\t\t\tSIZEg/SIZEG — number of GB\n");
    printf("\t-n\t\tlimit on the number of dumps\n");
    printf("\t-e\t\tlimit on the minimum amount of free space in the mount point\n");
    printf("\t\t\t\tSIZE — number of bytes,\n");
    printf("\t\t\t\tSIZEk/SIZEK — number of KB,\n");
    printf("\t\t\t\tSIZEm/SIZEM — number of MB,\n");
    printf("\t\t\t\tSIZEg/SIZEG — number of GB\n");
    printf("\t-c\t\tconfig file path\n");
    printf("\t-d\t\tdump directory path\n");
    puts("");
    return 0;
}
