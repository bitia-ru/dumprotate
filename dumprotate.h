#ifndef DUMPROTATE_H
#define DUMPROTATE_H

#include <stdbool.h> // bool
#include <sys/types.h> // off_t

typedef enum {
    DUMPROTATE_MAIN = 0,
    DUMPROTATE_HELP
} DumprotateAction;

typedef struct Dumprotate {
    off_t maxSize;
    int maxCount;
    off_t minEmptySpace;

    struct {
        off_t maxSize;
        int maxCount;
        off_t minEmptySpace;
        char* configPath;
        char* dumpDir;
        DumprotateAction action;
    } args;

    struct {
        off_t maxSize;
        int maxCount;
        off_t minEmptySpace;
        char* dumpDir;
    } configFile;
} Dumprotate;

int parse_args(Dumprotate* drd, int argc, char** argv);
int load_config(Dumprotate* drd);
int compute_params(Dumprotate* drd);

int dr_main(Dumprotate* drd);
int dr_help(Dumprotate* drd);

#endif
