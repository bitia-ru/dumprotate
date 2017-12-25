#ifndef SSIZE2BYTES_H
#define SSIZE2BYTES_H

#include <sys/types.h> // off_t

int ssize2bytes(const char* str, off_t* numOfBytes);

#endif

