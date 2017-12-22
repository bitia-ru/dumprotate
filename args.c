#include <stdio.h> // sscanf
#include <string.h> // strlen

#include "dumprotate.h"

int convert_to_bytes(char* str, off_t* numOfBytes);

int parse_args(Dumprotate* drd, int argc, char** argv) {
    char currentFlag = '0';
    int res;
    off_t numOfBytes;
    int val;

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
                    res = convert_to_bytes(argv[i], &numOfBytes);
                    if (res != 0) {
                        return 1;
                    } else {
                        drd->args.maxSize = numOfBytes;
                    }
                    break;
                case 'n':
                    res = sscanf(argv[i], "%d", &val);
                    if (res != 1) {
                        return 1;
                    } else {
                        drd->args.maxCount = val;
                    }
                    break;
                case 'e':
                    res = convert_to_bytes(argv[i], &numOfBytes);
                    if (res != 0) {
                        return 1;
                    } else {
                        drd->args.minEmptySpace = numOfBytes;
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

int convert_to_bytes(char* str, off_t* numOfBytes) {
    off_t i;
    char a;
    int res;
    res = sscanf(str, "%ld%c", &i, &a);
    switch (res) {
        case 1:
            *numOfBytes = i;
            return 0;
        case 2:
            switch (a) {
                case 'k':
                case 'K':
                    *numOfBytes = i * 1024;
                    return 0;
                case 'm':
                case 'M':
                    *numOfBytes = i * (1024 << 1);
                    return 0;
                case 'g':
                case 'G':
                    *numOfBytes = i * (1024 << 2);
                    return 0;
                default:
                    return 1;
            }
        default:
            return 1;
    }
}