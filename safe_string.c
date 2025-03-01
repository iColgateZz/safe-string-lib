/* safe_string.c */

/* Include libs */
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "safe_string.h"

/* Definitions */
#define H_TYPE_8 0
#define H_TYPE_16 1
#define H_TYPE_32 2
#define H_TYPE_64 3
#define H_MASK 3

#define HDR(T, s) ((struct Header##T *)((s)-(sizeof(struct Header##T))))

/* Functions */

static inline
uint8_t getReqType(size_t ilen) {
    if (ilen < 1 << 8)
        return H_TYPE_8;
    if (ilen < 1 << 16)
        return H_TYPE_16;
#if (LONG_MAX == LLONG_MAX)
    if (ilen < 1ll << 32)
        return H_TYPE_32;
    return H_TYPE_64;
#else
    return H_TYPE_32;
#endif
}

static inline
uint8_t getHlen(uint8_t type) {
    switch(type & H_MASK) {
        case H_TYPE_8:
            return sizeof(Header8);
        case H_TYPE_16:
            return sizeof(Header16);
        case H_TYPE_32:
            return sizeof(Header32);
        case H_TYPE_64:
            return sizeof(Header64);
    }
    return 0;
} 

/*
    Create a new null-terminated string with length ilen.
    If input string is NULL, a buffer of length ilen is initialized with zero bytes.
*/
string snewlen(const char* input, size_t ilen) {
    void* h;
    string str;
    uint8_t type = getReqType(ilen);
    uint8_t hlen = getHlen(type);
    uint8_t* flag;
    
    h = malloc(hlen + ilen + 1);
    if (h == NULL) return NULL;
    if (input == NULL) memset(h, 0, ilen);
    str = (string)h + hlen;
    flag = (uint8_t*)str - 1;

    switch(type) {
        case H_TYPE_8: 
        {
            Header8* hdr = (Header8*)h;
            hdr->allocated = ilen;
            hdr->len = ilen;
            *flag = type;
            break;
        }
        case H_TYPE_16:
        {
            Header16* hdr = (Header16*)h;
            hdr->allocated = ilen;
            hdr->len = ilen;
            *flag = type;
            break;
        }
        case H_TYPE_32: 
        {
            Header32* hdr = (Header32*)h;
            hdr->allocated = ilen;
            hdr->len = ilen;
            *flag = type;
            break;
        }
        case H_TYPE_64:
        {
            Header64* hdr = (Header64*)h;
            hdr->allocated = ilen;
            hdr->len = ilen;
            *flag = type;
            break;
        }
    }

    if (input && ilen)
        memcpy(str, input, ilen);
    str[ilen] = 0;
    return str;
}

/*
    Create a new null-terminated string.
    Input should be null-terminated.
    If input string is NULL, NULL is returned.
*/
string snew(const char* input) {
    if (input == NULL) return NULL;
    size_t ilen = strlen(input);
    return snewlen(input, ilen);
}

/*
    Free the allocated memory.
    If input is NULL, do nothing.
*/
void sfree(string s) {
    if (s == NULL) return;
    free(s - getHlen(s[-1]));
    return;
}

/*
    Get the length stored in the header.
    If input is NULL, return 0.
    It might differ from the actual length of the string if
    the string is changed manually or via functions from the
    standard libraries. In order to update the length use supdatelen().
*/
static inline
size_t sgetlen(const string s) {
    if (s == NULL) return 0;
    uint8_t flag = s[-1];
    switch (flag & H_MASK) {
        case H_TYPE_8:
            return HDR(8, s)->len;
        case H_TYPE_16:
            return HDR(16, s)->len;
        case H_TYPE_32:
            return HDR(32, s)->len;
        case H_TYPE_64:
            return HDR(64, s)->len;
    }
    return 0;
}