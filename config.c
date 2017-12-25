#include <stdio.h> // sscanf
#include <unistd.h> // access
#include <iniparser.h> // iniparser_*
#include <errno.h> // ENOENT, EACCESS, EINVAL

#include "dumprotate.h"

int convert_val_to_bytes(const char* str, off_t* numOfBytes);

int load_config(Dumprotate* drd) {
    dictionary * ini;
    const char * configPath;
    const char* str;
    off_t numOfBytes;
    int res;
    int val;

    if (!drd->args.configPath) {
        configPath = "/etc/dumprotate.conf";
    } else {
        configPath = drd->args.configPath;
    }
    if (access(configPath, F_OK) == -1) {
        return ENOENT;
    }
    if (access(configPath, R_OK) == -1) {
        return EACCES;
    }
    ini = iniparser_load(configPath);
    if (!drd->args.maxSize) {
        str = iniparser_getstring(ini, "main:maxSize", "0");
        res = convert_val_to_bytes(str, &numOfBytes);
        if (res != 0) {
            return EINVAL;
        } else {
            drd->args.maxSize = numOfBytes;
        }
    }
    if (!drd->args.maxCount) {
        res = iniparser_getint(ini, "main:maxCount", 0);
        drd->args.maxCount = res;
    }
    if (!drd->args.minEmptySpace) {
        str = iniparser_getstring(ini, "main:minEmptySpace", "0");
        res = convert_val_to_bytes(str, &numOfBytes);
        if (res != 0) {
            return EINVAL;
        } else {
            drd->args.minEmptySpace = numOfBytes;
        }
    }
    if (!drd->args.dumpDir) {
        str = iniparser_getstring(ini, "main:dumpDir", "");
        if (str != "") {
            drd->args.dumpDir = str;
        }
    }

    return 0;
}

int convert_val_to_bytes(const char* str, off_t* numOfBytes) {
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
