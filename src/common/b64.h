#ifndef BASE_64_H
#include <string.h>
#include <stddef.h>
#endif
extern const char b64chars[];

int base64_encode(char *, unsigned char *, int);
int base64_decode(char *, unsigned char *, int);
