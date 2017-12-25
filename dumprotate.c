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
        res = load_config(&drd);
        if (res != 0)
            return res;
    }

    res = compute_params(&drd);
    if (res != 0)
        return res;

    return 0;
}

int compute_params(Dumprotate* drd) {
    return 0;
}
