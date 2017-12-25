#include <stdio.h> // sscanf
#include <unistd.h> // access
#include <iniparser.h> // iniparser_*
#include <errno.h> // ENOENT, EACCESS, EINVAL

#include "ssize2bytes.h" // ssize2bytes
#include "dumprotate.h"

int load_config(Dumprotate* drd) {
    dictionary * ini;
    const char * configPath;
    const char* str;
    off_t numOfBytes;
    int res;

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
    str = iniparser_getstring(ini, "main:maxSize", "0");
    res = ssize2bytes(str, &numOfBytes);
    if (res != 0) {
        return res;
    }
    drd->configFile.maxSize = numOfBytes;
    res = iniparser_getint(ini, "main:maxCount", 0);
    drd->configFile.maxCount = res;
    str = iniparser_getstring(ini, "main:minEmptySpace", "0");
    res = ssize2bytes(str, &numOfBytes);
    if (res != 0) {
        return res;
    }
    drd->configFile.minEmptySpace = numOfBytes;
    str = iniparser_getstring(ini, "main:dumpDir", NULL);
    drd->configFile.dumpDir = str;

    return 0;
}
