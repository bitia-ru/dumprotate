#include <stdio.h> // freopen, fopen, fwrite, sprintf, snprintf
#include <time.h> // strftime
#include <string.h> // strcpy, strlen
#include <unistd.h> // access, read, STDIN_FILENO
#include <sys/stat.h> // stat
#include <errno.h> // ENOENT, EACCES
#include <stdlib.h> // malloc, realloc, free
#include <dirent.h> // opendir

#include "dumprotate.h"

#define STARTPATHLENGTH (64U)
#define STARTBUFFERSIZE (4096U)
#define STARTFDATASIZE (32U)

typedef struct FileData {
    time_t createFileTime;
    char * fileName;
} FileData;

int find_insert_position(FileData* fData, time_t createTime, int length);

int dr_main(Dumprotate* drd) {
    const char* dumpDir = opt_dump_dir(drd);

    struct stat st = {0};
    if (stat(dumpDir, &st) == -1) {
        return ENOENT;
    }

    int maxCount = opt_max_count(drd);
    if (maxCount != 0) {
        DIR *dir;

        if ((dir = opendir(dumpDir)) != NULL) {
            struct dirent *ent;
            struct stat sb;
            int currentNumOfDumps = 0;
            int currentFDataSize = STARTFDATASIZE;
            FileData *fData = (FileData *) malloc(currentFDataSize * sizeof (struct FileData));
            while ((ent = readdir(dir)) != NULL) {
                if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0)) {
                    continue;
                }
                if (currentFDataSize <= currentNumOfDumps) {
                    currentFDataSize = currentFDataSize << 1;
                    fData = (FileData *) realloc(fData, currentFDataSize * sizeof (struct FileData));
                }
                size_t currentPathLength = snprintf(NULL, 0, "%s/%s", dumpDir, ent->d_name);
                char *fileFullPath = (char *) malloc(currentPathLength);
                sprintf(fileFullPath, "%s/%s", dumpDir, ent->d_name);
                stat(fileFullPath, &sb);
                free(fileFullPath);
                int pos = find_insert_position(fData, sb.st_ctime, currentNumOfDumps);
                for (int i = currentNumOfDumps; i > pos; i--) {
                    fData[i].createFileTime = fData[i - 1].createFileTime;
                    fData[i].fileName = fData[i - 1].fileName;
                }
                fData[pos].createFileTime = sb.st_ctime;
                fData[pos].fileName = ent->d_name;
                currentNumOfDumps++;
            }
            closedir(dir);
            while (currentNumOfDumps + 1 > maxCount) {
                size_t currentPathLength = snprintf(NULL, 0, "%s/%s", dumpDir, fData[currentNumOfDumps - 1].fileName);
                char *fileFullPath = (char *) malloc(currentPathLength);
                sprintf(fileFullPath, "%s/%s", dumpDir, fData[currentNumOfDumps - 1].fileName);
                int res = remove(fileFullPath);
                free(fileFullPath);
                currentNumOfDumps--;
            }
            free(fData);
        }
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

int find_insert_position(FileData* fData, time_t createTime, int length) {
    if (length == 0) {
        return 0;
    }
    int i_current;
    int i_start = 0;
    int i_finish = length - 1;
    while (i_start + 1 < i_finish) {
        i_current = (i_start + i_finish) / 2 + (i_start + i_finish) % 2;
        if (fData[i_current].createFileTime > createTime) {
            i_start = i_current;

        } else {
            i_finish = i_current;
        }
    }
    if (fData[i_finish].createFileTime > createTime) {
        return i_finish + 1;
    }
    if (fData[i_start].createFileTime > createTime) {
        return i_finish;
    }
    return i_start;
}