#include <stdio.h> // sscanf
#include <errno.h> // EINVAL

#define K (1024UL)
#define M (1024UL*1024UL)
#define G (1024UL*1024UL*1024UL)

int ssize2bytes(const char* str, off_t* numOfBytes) {
    off_t i;
    char a, c;
    int res;
    res = sscanf(str, "%lld%c%c", &i, &a, &c);
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
            }
    }
    
    return EINVAL;
}