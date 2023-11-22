#ifndef HELPER_H
#include <string.h>
#include <stddef.h>
#endif
typedef unsigned char BYTE;

extern const char b64chars[];

int base64_encode(char *, unsigned char *, int);
int base64_decode(BYTE *, unsigned char *, int);
