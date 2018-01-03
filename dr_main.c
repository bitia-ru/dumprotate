#include <stdio.h> // freopen, fopen, fwrite, sprintf, snprintf
#include <time.h> // strftime
#include <string.h> // strcpy, strlen, strstr
#include <unistd.h> // access, read, STDIN_FILENO
#include <sys/stat.h> // stat
#include <errno.h> // ENOENT, EACCES
#include <error.h> // error
#include <stdlib.h> // malloc, realloc, free
#include <dirent.h> // opendir
#include <sys/statvfs.h> // statvfs
#include <stddef.h> // NULL
#include <regex.h>

#include "dumprotate.h"

#define START_PATH_LENGTH (64U)
#define START_BUFFER_SIZE (4096U)
#define START_NUM_OF_FILES (32U)

typedef struct FileData {
    off_t fileSize;
    char * fileName;
    off_t orderNum;
} FileData;

FileData * get_list_of_dump_files(Dumprotate* drd, int * currentNumOfDumps, int * filesInFolder, struct tm *currentDateTime, off_t *maxOrderNum);
FileData * find_latest(FileData * fDataOld, int length);
void free_fdata(FileData * fData, int length);
char * load_input(off_t * inputSize);
void remove_file(const char* dumpDir, char * fileName);
FileData * find_oldest(FileData * fData, int length);
char *replace_str(char *str, char *originStr, char *replaceStr);
int get_num_by_pattern(char *nameFormat, char *fileName);
char * create_file_name_pattern(struct tm *currentDateTime, const char* nameFormat, bool woTimestamps);

int dr_main(Dumprotate* drd) {
    const char* dumpDir = opt_dump_dir(drd);

    struct stat st = {0};
    if (stat(dumpDir, &st) == -1) {
        error(0, ENOENT, "Couldn't find dump dir %s", dumpDir);
        return ENOENT;
    }

    time_t rawtime;
    time(&rawtime);
    struct tm *currentDateTime;
    currentDateTime = localtime(&rawtime);
    bool woTimestamps = opt_wo_timestamps(drd);
    const char* nameFormat = opt_name_format(drd);
    int maxCount = opt_max_count(drd);
    if (maxCount != 0) {
        int currentNumOfDumps = 0;
        int filesInFolder = 0;
        off_t maxOrderNum = -1;
        FileData *fData = get_list_of_dump_files(drd, &currentNumOfDumps, &filesInFolder, currentDateTime, &maxOrderNum);
        int numOfFilesToDel = filesInFolder + 1 - maxCount;
        if (numOfFilesToDel > currentNumOfDumps) {
            error(0, ENOMEM, "Too many files that don't fit current name format. Limit on maximum number of dumps can't be satisfied");
            return ENOMEM;
        }
        if (numOfFilesToDel > 0) {
            FileData *fDataOld = (FileData *) malloc(numOfFilesToDel * sizeof (struct FileData));
            for (int i = 0; i < numOfFilesToDel; i++) {
                fDataOld[i].fileName = (char *) malloc(strlen(fData[0].fileName));
            }
            FileData *currentLatest;
            currentLatest = &(fDataOld[0]);
            for (int i = 0; i < numOfFilesToDel; i++) {
                fDataOld[i].orderNum = fData[i].orderNum;
                fDataOld[i].fileName = (char *) realloc(fDataOld[i].fileName, strlen(fData[i].fileName) + 1);
                strcpy(fDataOld[i].fileName, fData[i].fileName);
                if (currentLatest->orderNum < fDataOld[i].orderNum) {
                    currentLatest = &(fDataOld[i]);
                }
            }
            for (int i = numOfFilesToDel; i < currentNumOfDumps; i++) {
                if (currentLatest->orderNum > fData[i].orderNum) {
                    currentLatest->orderNum = fData[i].orderNum;
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
        int filesInFolder = 0;
        off_t maxOrderNum = -1;
        FileData *fData = get_list_of_dump_files(drd, &currentNumOfDumps, &filesInFolder, currentDateTime, &maxOrderNum);

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
            if (!woTimestamps) {
                currentOldest->orderNum = time(NULL);
            } else {
                currentOldest->orderNum = maxOrderNum + 1;
            }
            sumFileSize -= currentOldest->fileSize;
            i--;
        }
        free_fdata(fData, currentNumOfDumps);
        if (sumFileSize > maxSize) {
            error(0, ENOMEM, "Maximum storage size is less then dump file size or too many files that don't fit current name format.");
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
            int filesInFolder = 0;
            off_t maxOrderNum = -1;
            FileData *fData = get_list_of_dump_files(drd, &currentNumOfDumps, &filesInFolder, currentDateTime, &maxOrderNum);
            int i = currentNumOfDumps;
            while ((i > 0) && (minEmptySpace > currentAvailable - inputSize)) {
                FileData *currentOldest;
                currentOldest = find_oldest(fData, currentNumOfDumps);
                remove_file(dumpDir, currentOldest->fileName);
                if (!woTimestamps) {
                    currentOldest->orderNum = time(NULL);
                } else {
                    currentOldest->orderNum = maxOrderNum + 1;
                }
                currentAvailable += currentOldest->fileSize;
                i--;
            }
            free_fdata(fData, currentNumOfDumps);
            if (minEmptySpace > currentAvailable - inputSize) {
                error(0, ENOMEM, "Not enough space in %s or too many files that don't fit current name format.", dumpDir);
                return ENOMEM;
            }
        }
    }

    char *fileNamePattern = create_file_name_pattern(currentDateTime, nameFormat, woTimestamps);
    char * fileNameFinal;
    int currentNumOfDumps = 0;
    int filesInFolder = 0;
    off_t maxOrderNum = -1;
    FileData *fData = get_list_of_dump_files(drd, &currentNumOfDumps, &filesInFolder, currentDateTime, &maxOrderNum);
    free_fdata(fData, currentNumOfDumps);

    if (maxOrderNum == -1) {
        fileNameFinal = replace_str(fileNamePattern, "%i", "");
    } else {
        int iToASize = snprintf(NULL, 0, "%ld", maxOrderNum + 1);
        char * iToA = (char *) malloc(iToASize + 1);
        sprintf(iToA, "%ld", maxOrderNum + 1);
        fileNameFinal = replace_str(fileNamePattern, "%i", iToA);
        free(iToA);
    }

    size_t currentPathLength = snprintf(NULL, 0, "%s/%s", dumpDir, fileNameFinal);
    char *fileFullPath = (char *) malloc(currentPathLength + 1);
    sprintf(fileFullPath, "%s/%s", dumpDir, fileNameFinal);
    free(fileNameFinal);
    free(fileNamePattern);
    FILE * outputFile = fopen(fileFullPath, "wb");
    if (outputFile == NULL) {
        error(0, EACCES, "Couldn't open output file %s", fileFullPath);
        return EACCES;
    }
    free(fileFullPath);
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

FileData * get_list_of_dump_files(Dumprotate* drd, int * currentNumOfDumps, int * filesInFolder, struct tm *currentDateTime, off_t * maxOrderNum) {
    const char* dumpDir = opt_dump_dir(drd);
    bool woTimestamps = opt_wo_timestamps(drd);
    const char* nameFormat = opt_name_format(drd);
    int maxCount = opt_max_count(drd);
    
    DIR *dir;
    dir = opendir(dumpDir);
    struct dirent *ent;
    struct stat sb;
    int currentFDataSize;
    char * fileNamePattern = create_file_name_pattern(currentDateTime, nameFormat, woTimestamps);
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
        (*filesInFolder)++;
        if (currentFDataSize <= *currentNumOfDumps) {
            currentFDataSize = currentFDataSize << 1;
            fData = (FileData *) realloc(fData, currentFDataSize * sizeof (struct FileData));
        }
        size_t currentPathLength = snprintf(NULL, 0, "%s/%s", dumpDir, ent->d_name);
        char *fileFullPath = (char *) malloc(currentPathLength + 1);
        sprintf(fileFullPath, "%s/%s", dumpDir, ent->d_name);
        stat(fileFullPath, &sb);
        free(fileFullPath);
        int res;
        if (!woTimestamps) {
            fData[*currentNumOfDumps].orderNum = sb.st_ctime;
            fData[*currentNumOfDumps].fileName = (char *) malloc(strlen(ent->d_name) + 1);
            strcpy(fData[*currentNumOfDumps].fileName, ent->d_name);
            fData[*currentNumOfDumps].fileSize = sb.st_size;
            (*currentNumOfDumps)++;
            res = get_num_by_pattern(fileNamePattern, ent->d_name);
            if (res > (*maxOrderNum)) {
                (*maxOrderNum) = res;
            }
        } else {
            res = get_num_by_pattern(fileNamePattern, ent->d_name);
            if (res != -1) {
                fData[*currentNumOfDumps].orderNum = res;
                if (res > (*maxOrderNum)) {
                    (*maxOrderNum) = fData[*currentNumOfDumps].orderNum;
                }
                fData[*currentNumOfDumps].fileName = (char *) malloc(strlen(ent->d_name) + 1);
                strcpy(fData[*currentNumOfDumps].fileName, ent->d_name);
                fData[*currentNumOfDumps].fileSize = sb.st_size;
                (*currentNumOfDumps)++;
            }
        }
    }
    free(fileNamePattern);
    closedir(dir);
    return fData;
}

FileData * find_latest(FileData * fDataOld, int length) {
    FileData * currentLatest;
    currentLatest = &(fDataOld[0]);
    for (int i = 0; i < length; i++) {
        if (currentLatest->orderNum < fDataOld[i].orderNum) {
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
        if (currentOldest->orderNum > fData[i].orderNum) {
            currentOldest = &(fData[i]);
        }
    }
    return currentOldest;
}

char *replace_str(char *str, char *originStr, char *replaceStr) {
    char *p;

    p = strstr(str, originStr);
    char *buffer = (char *) malloc(strlen(str) + strlen(replaceStr) - strlen(originStr) + 1);
    strncpy(buffer, str, p - str);
    buffer[p - str] = '\0';

    sprintf(buffer + (p - str), "%s%s", replaceStr, p + strlen(originStr));
    return buffer;
}

int get_num_by_pattern(char *nameFormat, char *fileName) {
    char * nameFormatWithStartEnd = (char *) malloc(strlen(nameFormat) + 3);
    strcpy(nameFormatWithStartEnd, "^");
    strcat(nameFormatWithStartEnd, nameFormat);
    strcat(nameFormatWithStartEnd, "$");
    char * regexString = replace_str(nameFormatWithStartEnd, "%i", "([0-9]*)");
    free(nameFormatWithStartEnd);
    size_t maxGroups = 2;

    regex_t regexCompiled;
    regmatch_t groupArray[2];
    if (regcomp(&regexCompiled, regexString, REG_EXTENDED)) {
        free(regexString);
        return -1;
    }
    free(regexString);
    if (regexec(&regexCompiled, fileName, maxGroups, groupArray, 0) == 0) {
        if (groupArray[1].rm_so == (size_t) - 1) {
            regfree(&regexCompiled);
            return -1;
        }
        char *fileNameCopy = (char *) malloc(strlen(fileName) + 1);
        strcpy(fileNameCopy, fileName);
        fileNameCopy[groupArray[1].rm_eo] = 0;
        int num;
        sscanf(fileNameCopy + groupArray[1].rm_so, "%d", &num);
        free(fileNameCopy);
        regfree(&regexCompiled);
        return num;
    }

    regfree(&regexCompiled);

    return -1;
}

char * create_file_name_pattern(struct tm *currentDateTime, const char* nameFormat, bool woTimestamps) {
    size_t currentPathLength = START_PATH_LENGTH;
    char *fileNamePattern = (char *) malloc(currentPathLength + 1);
    char* nameFormatCurrent = (char *) malloc(strlen(nameFormat) + strlen(".dump") + strlen("%i") + 1);
    strcpy(nameFormatCurrent, nameFormat);
    if (woTimestamps) {
        strcat(nameFormatCurrent, ".dump");
        if (strstr(nameFormatCurrent, "%i") == NULL) {
            strcat(nameFormatCurrent, "%i");
        }
    }
    size_t res = strftime(fileNamePattern, currentPathLength, nameFormatCurrent, currentDateTime);
    while (res == 0) {
        currentPathLength = currentPathLength << 1;
        fileNamePattern = (char *) realloc(fileNamePattern, currentPathLength);
        res = strftime(fileNamePattern, currentPathLength, nameFormatCurrent, currentDateTime);
    }
    return fileNamePattern;
}