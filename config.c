#include <stdio.h> // sscanf
#include <unistd.h> // access
#include <iniparser.h> // iniparser_*
#include <errno.h> // ENOENT, EACCESS, EINVAL
#include <error.h> // error

#include "ssize2bytes.h" // ssize2bytes
#include "dumprotate.h"

int load_config(Dumprotate* drd, const char* configPath) {
    dictionary * ini;
    const char* str;
    off_t numOfBytes;
    int res;

    if (access(configPath, F_OK) == -1) {
        return 0;
    }
    if (access(configPath, R_OK) == -1) {
        error(0, EACCES, "Couldn't open config file %s", configPath);
        return EACCES;
    }
    ini = iniparser_load(configPath);
    str = iniparser_getstring(ini, "main:maxSize", "0");
    res = ssize2bytes(str, &numOfBytes);
    if (res != 0) {
        return res;
    }
    drd->configFile.maxSize = numOfBytes;
    drd->configFile.maxCount = iniparser_getint(ini, "main:maxCount", 0);
    str = iniparser_getstring(ini, "main:minEmptySpace", "0");
    res = ssize2bytes(str, &numOfBytes);
    if (res != 0) {
        return res;
    }
    drd->configFile.minEmptySpace = numOfBytes;
    drd->configFile.dumpDir = iniparser_getstring(ini, "main:dumpDir", NULL);
    drd->configFile.nameFormat = iniparser_getstring(ini, "main:nameFormat", NULL);
    drd->configFile.woTimestamps = iniparser_getboolean(ini, "main:woTimestamps", 0);

    return 0;
}
