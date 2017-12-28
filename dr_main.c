#include <stdio.h> // freopen, fopen, fwrite, sprintf, snprintf
#include <time.h> // strftime
#include <string.h> // strcpy, strlen
#include <unistd.h> // access, read, STDIN_FILENO
#include <sys/stat.h> // stat
#include <errno.h> // ENOENT, EACCES
#include <stdlib.h> // malloc, realloc, free
#include <dirent.h> // opendir

#include "dumprotate.h"

#define START_PATH_LENGTH (64U)
#define START_BUFFER_SIZE (4096U)

typedef struct FileData {
    time_t createFileTime;
    char * fileName;
} FileData;

FileData * get_list_of_dump_files(const char* dumpDir, int maxCount, int * currentNumOfDumps);
FileData * find_latest(FileData * fDataOld, int length);
void free_fdata(FileData * fData, int length);

int dr_main(Dumprotate* drd) {
    const char* dumpDir = opt_dump_dir(drd);

    struct stat st = {0};
    if (stat(dumpDir, &st) == -1) {
        return ENOENT;
    }

    int maxCount = opt_max_count(drd);
    if (maxCount != 0) {
        int currentNumOfDumps = 0;
        FileData *fData = get_list_of_dump_files(dumpDir, maxCount, &currentNumOfDumps);
        int numOfFilesToDel = currentNumOfDumps + 1 - maxCount;
        if (numOfFilesToDel > 0) {
            FileData *fDataOld = (FileData *) malloc(numOfFilesToDel * sizeof (struct FileData));
            for (int i = 0; i < numOfFilesToDel; i++) {
                fDataOld[i].fileName = (char *) malloc(strlen(fData[0].fileName));
            }
            FileData *currentLatest;
            currentLatest = &(fDataOld[0]);
            for (int i = 0; i < numOfFilesToDel; i++) {
                fDataOld[i].createFileTime = fData[i].createFileTime;
                fDataOld[i].fileName = (char *) realloc(fDataOld[i].fileName, strlen(fData[i].fileName) + 1);
                strcpy(fDataOld[i].fileName, fData[i].fileName);
                if (currentLatest->createFileTime < fDataOld[i].createFileTime) {
                    currentLatest = &(fDataOld[i]);
                }
            }
            for (int i = numOfFilesToDel; i < currentNumOfDumps; i++) {
                if (currentLatest->createFileTime > fData[i].createFileTime) {
                    currentLatest->createFileTime = fData[i].createFileTime;
                    currentLatest->fileName = (char *) realloc(currentLatest->fileName, strlen(fData[i].fileName) + 1);
                    strcpy(currentLatest->fileName, fData[i].fileName);
                    currentLatest = find_latest(fDataOld, numOfFilesToDel);
                }
            }
            for (int i = 0; i < numOfFilesToDel; i++) {
                size_t currentPathLength = snprintf(NULL, 0, "%s/%s", dumpDir, fDataOld[i].fileName);
                char *fileFullPath = (char *) malloc(currentPathLength + 1);
                sprintf(fileFullPath, "%s/%s", dumpDir, fDataOld[i].fileName);
                int res = remove(fileFullPath);
                free(fileFullPath);
            }
            free_fdata(fDataOld, numOfFilesToDel);
        }
        free_fdata(fData, currentNumOfDumps);
    }

    time_t rawtime;
    time(&rawtime);
    struct tm *currentDateTime;
    currentDateTime = localtime(&rawtime);
    size_t currentPathLength = START_PATH_LENGTH;
    char *fileName = (char *) malloc(currentPathLength);
    size_t res = strftime(fileName, currentPathLength, "%c.dump", currentDateTime);
    while (res == 0) {
        currentPathLength = currentPathLength << 1;
        fileName = (char *) realloc(fileName, currentPathLength);
        res = strftime(fileName, currentPathLength, "%c.dump", currentDateTime);
    }
    currentPathLength = snprintf(NULL, 0, "%s/%s", dumpDir, fileName);
    char *fileFullPathBase = (char *) malloc(currentPathLength + 1);
    sprintf(fileFullPathBase, "%s/%s", dumpDir, fileName);
    free(fileName);
    char *fileFullPathFinal = (char *) malloc(currentPathLength);
    strcpy(fileFullPathFinal, fileFullPathBase);
    int i = 1;
    while (access(fileFullPathFinal, F_OK) != -1) {
        currentPathLength = snprintf(NULL, 0, "%s.%d", fileFullPathBase, i);
        if (currentPathLength > strlen(fileFullPathFinal)) {
            fileFullPathFinal = (char *) realloc(fileFullPathFinal, currentPathLength + 1);
        }
        sprintf(fileFullPathFinal, "%s.%d", fileFullPathBase, i);
        i++;
    }
    free(fileFullPathBase);
    FILE * outputFile = fopen(fileFullPathFinal, "wb");
    free(fileFullPathFinal);
    if (outputFile == NULL) {
        return EACCES;
    }
    FILE * inputFile = freopen(NULL, "rb", stdin);
    char buffer[START_BUFFER_SIZE];
    ssize_t readedBytes;
    readedBytes = read(STDIN_FILENO, buffer, START_BUFFER_SIZE);
    while (readedBytes != 0) {
        fwrite(buffer, readedBytes, 1, outputFile);
        readedBytes = read(STDIN_FILENO, buffer, START_BUFFER_SIZE);
    }
    fclose(outputFile);
    return 0;
}

FileData * get_list_of_dump_files(const char* dumpDir, int maxCount, int * currentNumOfDumps) {
    DIR *dir;
    dir = opendir(dumpDir);
    struct dirent *ent;
    struct stat sb;
    int currentFDataSize = maxCount;
    FileData *fData = (FileData *) malloc(currentFDataSize * sizeof (struct FileData));
    while ((ent = readdir(dir)) != NULL) {
        if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0)) {
            continue;
        }
        if (currentFDataSize <= *currentNumOfDumps) {
            currentFDataSize = currentFDataSize << 1;
            fData = (FileData *) realloc(fData, currentFDataSize * sizeof (struct FileData));
        }
        size_t currentPathLength = snprintf(NULL, 0, "%s/%s", dumpDir, ent->d_name);
        char *fileFullPath = (char *) malloc(currentPathLength + 1);
        sprintf(fileFullPath, "%s/%s", dumpDir, ent->d_name);
        stat(fileFullPath, &sb);
        free(fileFullPath);
        fData[*currentNumOfDumps].createFileTime = sb.st_ctime;
        fData[*currentNumOfDumps].fileName = (char *) malloc(strlen(ent->d_name) + 1);
        strcpy(fData[*currentNumOfDumps].fileName, ent->d_name);
        (*currentNumOfDumps)++;
    }
    closedir(dir);
    return fData;
}

FileData * find_latest(FileData * fDataOld, int length) {
    FileData * currentLatest;
    currentLatest = &(fDataOld[0]);
    for (int i = 0; i < length; i++) {
        if (currentLatest->createFileTime < fDataOld[i].createFileTime) {
            currentLatest = &(fDataOld[i]);
        }
    }
    return currentLatest;
}

void free_fdata(FileData * fData, int length) {
    for (int i = 0; i < length; i++) {
        free(fData[i].fileName);
    }
    free(fData);
}