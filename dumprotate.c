#include <string.h> // memset

#include "dumprotate.h"

const bool actionConfigDisusage[] = {
    [DUMPROTATE_HELP] = true,
};

typedef int (*DumprotateActionDispatcher_f) (Dumprotate* drd);

const struct {
    DumprotateAction action;
    DumprotateActionDispatcher_f dispatcher;
} actionDispatchers[] = {
    { DUMPROTATE_MAIN, dr_main },
    { DUMPROTATE_HELP, dr_help },
};

int main(int argc, char** argv) {
    Dumprotate drd;
    int res;

    memset(&drd, 0, sizeof (Dumprotate));

    res = parse_args(&drd, argc, argv);
    if (res != 0)
        return res;
    if (!actionConfigDisusage[drd.args.action]) {
        res = load_config(&drd, opt_config_path(&drd));
        if (res != 0)
            return res;
        res = dr_main(&drd);
        if (res != 0)
            return res;
    }
    dr_help();
}

