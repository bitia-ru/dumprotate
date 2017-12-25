#include <stdio.h> // freopen, fopen, fread, fwrite, sprintf
#include <time.h> // strftime
#include <string.h> // strcpy
#include <unistd.h> // access
#include <sys/stat.h> // stat
#include <errno.h> // ENOENT, EACCES

#include "dumprotate.h"

int dr_main(Dumprotate* drd) {
    time_t rawtime;
    struct tm *currentDateTime;
    char fileName[80];
    char fileFullPath[256];
    char fileFullPathCopy[256];
    char buffer[2];
    FILE * outputFile;
    FILE * inputFile;
    int i;
    size_t sizeOfInput;
    const char* dumpDir;
    struct stat st = {0};

    time(&rawtime);

    currentDateTime = localtime(&rawtime);

    strftime(fileName, 80, "%c.dump", currentDateTime);
    dumpDir = opt_dump_dir(drd);

    if (stat(dumpDir, &st) == -1) {
        return ENOENT;
    }
    sprintf(fileFullPath, "%s/%s", dumpDir, fileName);
    strcpy(fileFullPathCopy, fileFullPath);
    i = 1;
    while (access(fileFullPathCopy, F_OK) != -1) {
        sprintf(fileFullPathCopy, "%s.%d", fileFullPath, i);
        i++;
    }
    outputFile = fopen(fileFullPathCopy, "wb");
    if (outputFile == 0) {
        return EACCES;
    }
    inputFile = freopen(NULL, "rb", stdin);
    sizeOfInput = fread(buffer, 1, sizeof buffer, inputFile);
    while (sizeOfInput != 0) {
        fwrite(buffer, 1, sizeOfInput, outputFile);
        sizeOfInput = fread(buffer, 1, sizeof buffer, inputFile);
    }
    fclose(outputFile);
    return 0;
}
