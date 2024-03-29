#include <stddef.h> // NULL

#include "dumprotate.h"

#define OPT_ARGS_CONFIG(drd, fieldName, default) \
    ( drd->args.fieldName != default ? drd->args.fieldName : drd->configFile.fieldName )

off_t opt_max_size(Dumprotate* drd) {
    return OPT_ARGS_CONFIG(drd, maxSize, 0);
}

int opt_max_count(Dumprotate* drd) {
    return OPT_ARGS_CONFIG(drd, maxCount, 0);
}

off_t opt_min_empty_space(Dumprotate* drd) {
    return OPT_ARGS_CONFIG(drd, minEmptySpace, 0);
}

const char* opt_dump_dir(Dumprotate* drd) {
    return OPT_ARGS_CONFIG(drd, dumpDir, NULL);
}

const char* opt_name_format(Dumprotate* drd) {
    const char* res = OPT_ARGS_CONFIG(drd, nameFormat, NULL);
    return res != NULL ? res : "%c.dump%i";
}

DumprotateAction opt_action(Dumprotate* drd) {
    return drd->args.action;
}

const char* opt_config_path(Dumprotate* drd) {
    return drd->args.configPath != NULL ? drd->args.configPath : "/etc/dumprotate.conf";
}