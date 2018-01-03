#ifndef DUMPROTATE_H
#define DUMPROTATE_H

#include <stdbool.h> // bool
#include <sys/types.h> // off_t

typedef enum {
    DUMPROTATE_MAIN = 0,
    DUMPROTATE_HELP
} DumprotateAction;

typedef struct Dumprotate {
    struct {
        off_t maxSize;
        int maxCount;
        off_t minEmptySpace;
        const char* configPath;
        const char* dumpDir;
        const char* nameFormat;
        bool woTimestamps;
        DumprotateAction action;
    } args;

    struct {
        off_t maxSize;
        int maxCount;
        off_t minEmptySpace;
        const char* dumpDir;
        const char* nameFormat;
        bool woTimestamps;
    } configFile;
} Dumprotate;

/* Target routines: */
int dr_main(Dumprotate* drd);
int dr_help();
/* ---------------- */

int parse_args(Dumprotate* drd, int argc, char** argv);
int load_config(Dumprotate* drd, const char* configPath);

/* Option getters: */
off_t opt_max_size(Dumprotate* drd);
int opt_max_count(Dumprotate* drd);
off_t opt_min_empty_space(Dumprotate* drd);
const char* opt_dump_dir(Dumprotate* drd);
const char* opt_name_format(Dumprotate* drd);
DumprotateAction opt_action(Dumprotate* drd);
const char* opt_config_path(Dumprotate* drd);
bool opt_wo_timestamps(Dumprotate* drd);
/* --------------- */


#endif
