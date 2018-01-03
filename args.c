#include <stdio.h> // sscanf
#include <string.h> // strlen, strcmp
#include <errno.h> // EINVAL
#include <error.h> // error

#include "ssize2bytes.h" // ssize2bytes
#include "dumprotate.h"

int parse_args(Dumprotate* drd, int argc, char** argv) {
    char currentFlag = 0;
    int res;
    off_t numOfBytes;
    int val;
    char c;

    for (int i = 1; i < argc; i++) {
        if (currentFlag == 0) {
            if ((strlen(argv[i]) != 2) || (argv[i][0] != '-')) {
                if (strcmp(argv[i], "--help") == 0) {
                    drd->args.action = DUMPROTATE_HELP;
                    return 0;
                }
                error(0, EINVAL, "Unknown option %s", argv[i]);
                return EINVAL;
            }
            switch (argv[i][1]) {
                case 's':
                case 'n':
                case 'e':
                case 'c':
                case 'd':
                case 'f':
                case 'T':
                    currentFlag = argv[i][1];
                    break;
                case 'h':
                    drd->args.action = DUMPROTATE_HELP;
                    return 0;
                default:
                    error(0, EINVAL, "Unknown option %s", argv[i]);
                    return EINVAL;
            }

            continue;
        }
        switch (currentFlag) {
            case 's':
                res = ssize2bytes(argv[i], &numOfBytes);
                if (res != 0) {
                    return res;
                }
                drd->args.maxSize = numOfBytes;
                break;
            case 'n':
                res = sscanf(argv[i], "%d%c", &val, &c);
                if (res != 1) {
                    error(0, EINVAL, "Wrong value %s for number of dumps", argv[i]);
                    return EINVAL;
                }
                drd->args.maxCount = val;
                break;
            case 'e':
                res = ssize2bytes(argv[i], &numOfBytes);
                if (res != 0) {
                    return res;
                }
                drd->args.minEmptySpace = numOfBytes;
                break;
            case 'c':
                drd->args.configPath = argv[i];
                break;
            case 'd':
                drd->args.dumpDir = argv[i];
                break;
            case 'f':
                drd->args.nameFormat = argv[i];
                break;
            case 'T':
                if (((strcmp(argv[i],"0") == 0) || (strcmp(argv[i],"f") == 0)) || (strcmp(argv[i],"false") == 0)) {
                    drd->args.woTimestamps = false;
                    break;
                }
                if (((strcmp(argv[i],"1") == 0) || (strcmp(argv[i],"t") == 0)) || (strcmp(argv[i],"true") == 0)) {
                    drd->args.woTimestamps = true;
                    break;
                }
                error(0, EINVAL, "Wrong value %s for without timestamp parameter", argv[i]);
                return EINVAL;
        }
        currentFlag = 0;
    }
    return 0;
}