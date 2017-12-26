#include <stdio.h> // freopen, fopen, fwrite, sprintf, snprintf
#include <time.h> // strftime
#include <string.h> // strcpy, strlen
#include <unistd.h> // access, read, STDIN_FILENO
#include <sys/stat.h> // stat
#include <errno.h> // ENOENT, EACCES
#include <stdlib.h> // malloc, realloc, free

#include "dumprotate.h"

#define STARTPATHLENGTH (64U)
#define STARTBUFFERSIZE (4096U)

int dr_main(Dumprotate* drd) {
    const char* dumpDir = opt_dump_dir(drd);

    struct stat st = {0};
    if (stat(dumpDir, &st) == -1) {
        return ENOENT;
    }
    time_t rawtime;
    time(&rawtime);
    struct tm *currentDateTime;
    currentDateTime = localtime(&rawtime);
    size_t currentPathLength = STARTPATHLENGTH;
    char *fileName = (char *) malloc(currentPathLength);
    size_t res = strftime(fileName, currentPathLength, "%c.dump", currentDateTime);
    while (res == 0) {
        currentPathLength = currentPathLength << 1;
        fileName = (char *) realloc(fileName, currentPathLength);
        res = strftime(fileName, currentPathLength, "%c.dump", currentDateTime);
    }
    currentPathLength = snprintf(NULL, 0, "%s/%s", dumpDir, fileName);
    char *fileFullPathBase = (char *) malloc(currentPathLength);
    sprintf(fileFullPathBase, "%s/%s", dumpDir, fileName);
    free(fileName);
    char *fileFullPathFinal = (char *) malloc(currentPathLength);
    strcpy(fileFullPathFinal, fileFullPathBase);
    int i = 1;
    while (access(fileFullPathFinal, F_OK) != -1) {
        currentPathLength = snprintf(NULL, 0, "%s.%d", fileFullPathBase, i);
        if (currentPathLength > strlen(fileFullPathFinal)) {
            fileFullPathFinal = (char *) realloc(fileFullPathFinal, currentPathLength);
        }
        sprintf(fileFullPathFinal, "%s.%d", fileFullPathBase, i);
        i++;
    }
    free(fileFullPathBase);
    FILE * outputFile = fopen(fileFullPathFinal, "wb");
    free(fileFullPathFinal);
    if (outputFile == 0) {
        return EACCES;
    }
    FILE * inputFile = freopen(NULL, "rb", stdin);
    char buffer[STARTBUFFERSIZE];
    ssize_t readedBytes;
    readedBytes = read(STDIN_FILENO, buffer, STARTBUFFERSIZE);
    while (readedBytes != 0) {
        fwrite(buffer, readedBytes, 1, outputFile);
        readedBytes = read(STDIN_FILENO, buffer, STARTBUFFERSIZE);
    }
    fclose(outputFile);
    return 0;
}
