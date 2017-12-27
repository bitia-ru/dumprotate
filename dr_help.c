#include <stdio.h> // puts

#include "dumprotate.h"

int dr_help() {
    puts("Usage: dumprot [OPTION...]");
    puts("");
    puts("\t-h, --help\tdisplay this help and exit");
    puts("\t-s SIZE\t\tlimit on the maximum storage size");
    puts("\t\t\t\tSIZE — number of bytes (kKmMgG supported)");
    puts("\t-n NUM\t\tlimit on the number of dumps");
    puts("\t-e SIZE\t\tlimit on the minimum amount of free space in the mount point");
    puts("\t\t\t\tSIZE — number of bytes (kKmMgG supported)");
    puts("\t-c PATH\t\tconfig file path");
    puts("\t-d DIR\t\tdump directory path");
    puts("\t-f FORMAT\tformat of dump file name");
    puts("");
    return 0;
}
