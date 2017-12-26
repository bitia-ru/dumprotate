#include <stdio.h> // puts

#include "dumprotate.h"

int dr_help() {
    puts("Usage: dumprot [OPTION...]");
    puts("");
    puts("\t-h, --help\tdisplay this help and exit");
    puts("\t-s\t\tlimit on the maximum storage size");
    puts("\t\t\t\tSIZE — number of bytes,");
    puts("\t\t\t\tSIZEk/SIZEK — number of KB,");
    puts("\t\t\t\tSIZEm/SIZEM — number of MB,");
    puts("\t\t\t\tSIZEg/SIZEG — number of GB");
    puts("\t-n\t\tlimit on the number of dumps");
    puts("\t-e\t\tlimit on the minimum amount of free space in the mount point");
    puts("\t\t\t\tSIZE — number of bytes,");
    puts("\t\t\t\tSIZEk/SIZEK — number of KB,");
    puts("\t\t\t\tSIZEm/SIZEM — number of MB,");
    puts("\t\t\t\tSIZEg/SIZEG — number of GB");
    puts("\t-c\t\tconfig file path");
    puts("\t-d\t\tdump directory path");
    puts("");
    return 0;
}
