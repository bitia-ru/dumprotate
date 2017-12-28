#include <stdio.h> // freopen, fopen, fwrite, sprintf, snprintf
#include <time.h> // strftime
#include <string.h> // strcpy, strlen
#include <unistd.h> // access, read, STDIN_FILENO
#include <sys/stat.h> // stat
#include <errno.h> // ENOENT, EACCES
#include <error.h> // error
#include <stdlib.h> // malloc, realloc, free
#include <dirent.h> // opendir
#include <sys/statvfs.h> // statvfs
#include <stddef.h> // NULL

#include "dumprotate.h"

#define START_PATH_LENGTH (64U)
#define START_BUFFER_SIZE (4096U)
#define START_NUM_OF_FILES (32U)

typedef struct FileData {
    time_t createFileTime;
    off_t fileSize;
    char * fileName;
} FileData;

FileData * get_list_of_dump_files(const char* dumpDir, int maxCount, int * currentNumOfDumps);
FileData * find_latest(FileData * fDataOld, int length);
void free_fdata(FileData * fData, int length);
char * load_input(off_t * inputSize);
void remove_file(const char* dumpDir, char * fileName);
FileData * find_oldest(FileData * fData, int length);

int dr_main(Dumprotate* drd) {
    const char* dumpDir = opt_dump_dir(drd);

    struct stat st = {0};
    if (stat(dumpDir, &st) == -1) {
        error(0, ENOENT, "%s", dumpDir);
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
                remove_file(dumpDir, fDataOld[i].fileName);
            }
            free_fdata(fDataOld, numOfFilesToDel);
        }
        free_fdata(fData, currentNumOfDumps);
    }
    off_t maxSize = opt_max_size(drd);
    off_t inputSize = 0;
    char *inputData = NULL;
    freopen(NULL, "rb", stdin);
    if (maxSize != 0) {
        int currentNumOfDumps = 0;
        FileData *fData = get_list_of_dump_files(dumpDir, maxCount, &currentNumOfDumps);
        
        inputData = load_input(&inputSize);
        off_t sumFileSize = inputSize;
        for (int i = 0; i < currentNumOfDumps; i++) {
            sumFileSize += fData[i].fileSize;
        }
        int i = currentNumOfDumps;
        while ((i > 0) && (sumFileSize > maxSize)) {
            FileData *currentOldest;
            currentOldest = find_oldest(fData, currentNumOfDumps);
            remove_file(dumpDir, currentOldest->fileName);
            currentOldest->createFileTime = time(NULL);
            sumFileSize -= currentOldest->fileSize;
            i--;
        }
        free_fdata(fData, currentNumOfDumps);
        if (sumFileSize > maxSize) {
            error(0, ENOMEM, "%s", dumpDir);
            return ENOMEM;
        }
    }

    off_t minEmptySpace = opt_min_empty_space(drd);
    if (minEmptySpace != 0) {
        struct statvfs stat;
        statvfs(dumpDir, &stat);
        off_t currentAvailable = stat.f_bsize * stat.f_bavail;
        if (inputData == NULL) {
            inputData = load_input(&inputSize);
        }
        if (inputSize > currentAvailable - minEmptySpace) {
            int currentNumOfDumps = 0;
            FileData *fData = get_list_of_dump_files(dumpDir, maxCount, &currentNumOfDumps);
            int i = currentNumOfDumps;
            while ((i > 0) && (minEmptySpace > currentAvailable - inputSize)) {
                FileData *currentOldest;
                currentOldest = find_oldest(fData, currentNumOfDumps);
                remove_file(dumpDir, currentOldest->fileName);
                currentOldest->createFileTime = time(NULL);
                currentAvailable += currentOldest->fileSize;
                i--;
            }
            free_fdata(fData, currentNumOfDumps);
            if (minEmptySpace > currentAvailable - inputSize) {
                return ENOMEM;
            }
        }
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
    if (outputFile == NULL) {
        error(0, EACCES, "%s", fileFullPathFinal);
        return EACCES;
    }
    free(fileFullPathFinal);
    if (inputData != NULL) {
        fwrite(inputData, inputSize, 1, outputFile);
        free(inputData);
    } else {
        char buffer[START_BUFFER_SIZE];
        ssize_t readedBytes;
        readedBytes = read(STDIN_FILENO, buffer, START_BUFFER_SIZE);
        while (readedBytes != 0) {
            fwrite(buffer, readedBytes, 1, outputFile);
            readedBytes = read(STDIN_FILENO, buffer, START_BUFFER_SIZE);
        }
    }
    fclose(outputFile);
    return 0;
}

FileData * get_list_of_dump_files(const char* dumpDir, int maxCount, int * currentNumOfDumps) {
    DIR *dir;
    dir = opendir(dumpDir);
    struct dirent *ent;
    struct stat sb;
    int currentFDataSize;
    if (maxCount == 0) {
        currentFDataSize = START_NUM_OF_FILES;
    } else {
        currentFDataSize = maxCount;
    }
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
        fData[*currentNumOfDumps].fileSize = sb.st_size;
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

char * load_input(off_t * inputSize) {
    char buffer[START_BUFFER_SIZE];
    ssize_t readedBytes;
    readedBytes = read(STDIN_FILENO, buffer, START_BUFFER_SIZE);
    off_t currentInputBufferSize = START_BUFFER_SIZE;
    char *inputData = (char *) malloc(currentInputBufferSize);
    strcpy(inputData, "");

    while (readedBytes != 0) {
        strcat(inputData, buffer);
        *inputSize += readedBytes;
        readedBytes = read(STDIN_FILENO, buffer, START_BUFFER_SIZE);
        if (*inputSize + readedBytes > currentInputBufferSize) {
            currentInputBufferSize = currentInputBufferSize << 1;
            inputData = (char *) realloc(inputData, currentInputBufferSize);
        }

    }
    return inputData;
}

void remove_file(const char* dumpDir, char * fileName) {
    size_t currentPathLength = snprintf(NULL, 0, "%s/%s", dumpDir, fileName);
    char *fileFullPath = (char *) malloc(currentPathLength + 1);
    sprintf(fileFullPath, "%s/%s", dumpDir, fileName);
    int res = remove(fileFullPath);
    free(fileFullPath);
}

FileData * find_oldest(FileData * fData, int length) {
    FileData * currentOldest;
    currentOldest = &(fData[0]);
    for (int i = 0; i < length; i++) {
        if (currentOldest->createFileTime > fData[i].createFileTime) {
            currentOldest = &(fData[i]);
        }
    }
    return currentOldest;
}