#include "b64.h"

const char b64chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Helper function to find the index of a character in the Base64 character set
// (using b64chars constant)
int char_index(unsigned char c)
{
    const char *ptr = strchr(b64chars, c);
    if (ptr != NULL) {
        return ptr - b64chars;
    }
    return -1; // Character not found in the Base64 character set
}

int base64_encode(char *out, unsigned char *in, int ilen)
{
    int olen, i, j, v;

    olen = ilen % 3 ? ilen + 3 - ilen % 3 : ilen;
    olen = (olen / 3) * 4;
    out[olen] = '\0';
    for (i = 0, j = 0; i < ilen; i += 3, j += 4) {
        v = in[i];
        v = i + 1 < ilen ? v << 8 | in[i + 1] : v << 8;
        v = i + 2 < ilen ? v << 8 | in[i + 2] : v << 8;
        out[j] = b64chars[(v >> 18) & 0x3F];
        out[j + 1] = b64chars[(v >> 12) & 0x3F];
        if (i + 1 < ilen)
            out[j + 2] = b64chars[(v >> 6) & 0x3F];
        else
            out[j + 2] = '=';
        if (i + 2 < ilen)
            out[j + 3] = b64chars[v & 0x3F];
        else
            out[j + 3] = '=';
    }
    return (olen);
}

int base64_decode(char *out, unsigned char *in, int ilen)
{
    int olen = 0, i = 0, j = 0;
    unsigned char b[4];

    olen = (ilen / 4) * 3;
    // Handle padding

    if (in[ilen - 1] == '=')
        olen -= 1;
    if (in[ilen - 2] == '=')
        olen -= 1;

    while (ilen > 0 && in[j] != '=') {
        b[0] = in[j++];
        b[1] = in[j++];
        b[2] = in[j++];
        b[3] = in[j++];

        // Decode the Base64 characters manually using the provided b64chars
        // constant
        out[i++] = (char_index(b[0]) << 2) | (char_index(b[1]) >> 4);
        out[i++] = (char_index(b[1]) << 4) | (char_index(b[2]) >> 2);
        out[i++] = (char_index(b[2]) << 6) | char_index(b[3]);

        ilen -= 4;
    }

    return olen;
}
