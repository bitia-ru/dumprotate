#include <stdio.h> // sscanf
#include <unistd.h> // access
#include <iniparser.h> // iniparser_load, iniparser_getstring

#include "dumprotate.h"

int convert_val_to_bytes(char* str, off_t* numOfBytes);

int load_config(Dumprotate* drd) {
    dictionary * ini;
    char * ini_name;
    char* str;
    off_t numOfBytes;
    int res;
    int val;

    if (!drd->args.configPath) {
        ini_name = "/etc/dumprotate.conf";
    } else {
        ini_name = drd->args.configPath;
    }
    if (access(ini_name, F_OK) == -1) {
        return 1;
    }
    if (access(ini_name, R_OK) == -1) {
        return 1;
    }
    ini = iniparser_load(ini_name);
    str = iniparser_getstring(ini, "main:maxSize", "");
    if (str != "") {
        res = convert_val_to_bytes(str, &numOfBytes);
        if (res != 0) {
            return 1;
        } else {
            drd->args.maxSize = numOfBytes;
        }
    }
    res = iniparser_getint(ini, "main:maxCount", -1);
    if (res != -1) {
        drd->args.maxCount = res;
    }
    str = iniparser_getstring(ini, "main:minEmptySpace", "");
    if (str != "") {
        res = convert_val_to_bytes(str, &numOfBytes);
        if (res != 0) {
            return 1;
        } else {
            drd->args.minEmptySpace = numOfBytes;
        }
    }
    str = iniparser_getstring(ini, "main:dumpDir", "");
    if (str != "") {
        drd->args.dumpDir = str;
    }
    iniparser_freedict(ini);

    return 0;
}

int convert_val_to_bytes(char* str, off_t* numOfBytes) {
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
