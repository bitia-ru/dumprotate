#include <string.h> // strlen
#include <math.h> // pow
#include <stdlib.h> //atoi
#include <ctype.h> // isdigit

#include "dumprotate.h"

int is_number(char* param);
int convert_to_bytes(off_t* numOfBytes, char* arg);
int check_argument(off_t* numOfBytesFinal, char* arg);

int parse_args(Dumprotate* drd, int argc, char** argv) {
    char currentFlag = '0';
    int res;
    off_t numOfBytesFinal;

    for (int i = 1; i < argc; i++) {
        if (currentFlag == '0') {
            if ((strlen(argv[i]) != 2) || (argv[i][0] != '-')) {
                return 1;
            }
            switch (argv[i][1]) {
                case 's':
                case 'n':
                case 'e':
                case 'c':
                case 'd':
                    currentFlag = argv[i][1];
                    break;
                case 'h':
                    drd->args.action = DUMPROTATE_HELP;
                    return 0;
                default:
                    return 1;
            }
        } else {
            switch (currentFlag) {
                case 's':
                    res = check_argument(&numOfBytesFinal, argv[i]);
                    if (res != 0) {
                        return 1;
                    } else {
                        drd->args.maxSize = numOfBytesFinal;
                    }
                    break;
                case 'n':
                    res = is_number(argv[i]);
                    if (res != 0) {
                        return 1;
                    } else {
                        drd->args.maxCount = atoi(argv[i]);
                    }
                    break;
                case 'e':
                    res = check_argument(&numOfBytesFinal, argv[i]);
                    if (res != 0) {
                        return 1;
                    } else {
                        drd->args.minEmptySpace = numOfBytesFinal;
                    }
                    break;
                case 'c':
                    drd->args.configPath = argv[i];
                    break;
                case 'd':
                    drd->args.dumpDir = argv[i];
                    break;
            }
            currentFlag = '0';
        }
    }
    return 0;
}

int is_number(char* param) {
    for (int i = 0; i < strlen(param); i++) {
        if (isdigit(param[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int convert_to_bytes(off_t* numOfBytes, char* arg) {
    int res;
    char c;
    int len;

    len = strlen(arg) - 1;
    c = arg[len];
    arg[len] = 0;
    res = is_number(arg);
    if (res != 0) {
        return 1;
    }
    switch (c) {
        case 'k':
        case 'K':
            *numOfBytes = atoi(arg)*1024;
            return 0;
        case 'm':
        case 'M':
            *numOfBytes = atoi(arg) * pow(1024.0, 2);
            return 0;
        case 'g':
        case 'G':
            *numOfBytes = atoi(arg) * pow(1024.0, 3);
            return 0;
        default:
            return 1;
    }
}

int check_argument(off_t* numOfBytesFinal, char* arg) {
    int res;
    off_t numOfBytes;
    if (isdigit(arg[strlen(arg) - 1]) == 0) {
        res = convert_to_bytes(&numOfBytes, arg);
        if (res != 0) {
            return 1;
        }
        *numOfBytesFinal = numOfBytes;
    } else {
        res = is_number(arg);
        if (res != 0) {
            return 1;
        } else {
            *numOfBytesFinal = atoi(arg);
        }
    }
    return 0;
}