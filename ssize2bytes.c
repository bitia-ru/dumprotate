#define K (1024UL)
#define M (1024UL*1024UL)
#define G (1024UL*1024UL*1024UL)

#include <stdio.h> // sscanf

#include "dumprotate.h"

int ssize2bytes(const char* str, off_t* numOfBytes) {
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
                    *numOfBytes = i * K;
                    return 0;
                case 'm':
                case 'M':
                    *numOfBytes = i * M;
                    return 0;
                case 'g':
                case 'G':
                    *numOfBytes = i * G;
                    return 0;
                default:
                    return 1;
            }
        default:
            return 1;
    }
}