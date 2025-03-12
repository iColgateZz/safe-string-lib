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

#define HDR(T, s) ((Header##T *)(s - sizeof(Header##T)))

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

static inline
size_t sgetalloc(const string s) {
    if (s == NULL) return 0;
    uint8_t flag = s[-1];
    switch (flag & H_MASK) {
        case H_TYPE_8:
            return HDR(8, s)->allocated;
        case H_TYPE_16:
            return HDR(16, s)->allocated;
        case H_TYPE_32:
            return HDR(32, s)->allocated;
        case H_TYPE_64:
            return HDR(64, s)->allocated;
    }
    return 0;
}

static inline
void ssetlen(string s, size_t len) {
    uint8_t flag = s[-1];
    switch (flag & H_MASK) {
        case H_TYPE_8:
        {
            HDR(8, s)->len = len;
            break;
        }
        case H_TYPE_16:
        {
            HDR(16, s)->len = len;
            break;
        }
        case H_TYPE_32:
        {
            HDR(32, s)->len = len;
            break;
        }
        case H_TYPE_64:
        {
            HDR(64, s)->len = len;
            break;
        }
    }
    return;
}

static inline
void ssetalloc(string s, size_t newalloc) {
    uint8_t flag = s[-1];
    switch (flag & H_MASK) {
        case H_TYPE_8:
        {
            HDR(8, s)->allocated = newalloc;
            break;
        }
        case H_TYPE_16:
        {
            HDR(16, s)->allocated = newalloc;
            break;
        }
        case H_TYPE_32:
        {
            HDR(32, s)->allocated = newalloc;
            break;
        }
        case H_TYPE_64:
        {
            HDR(64, s)->allocated = newalloc;
            break;
        }
    }
    return;
}

/*
    Create a new null-terminated string with length ilen.

    If input string is NULL, a buffer of length ilen is initialized with zero bytes.
    Return NULL if malloc fails.
    Return NULL if ilen causes overflow.
    ilen > strlen(input) causes undefined behaviour.
*/
string snewlen(const char* input, size_t ilen) {
    void* h;
    string str;
    uint8_t type = getReqType(ilen);
    uint8_t hlen = getHlen(type);
    uint8_t* flag;
    
    if (hlen + ilen + 1 < ilen) return NULL;

    h = malloc(hlen + ilen + 1);
    if (h == NULL) return NULL;
    if (input == NULL) memset(h, 0, hlen + ilen + 1);
    str = (string)((uint8_t*)h + hlen);
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

    Input must be null-terminated.
    If input string is NULL, NULL is returned 
    and no memory allocation is performed.
    Return NULL if input causes overflow.
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

/*
    Update the lenght of a string in case you changed it manually.

    If NULL is passed as an argument, nothing is done.
*/
void supdatelen(string s) {
    if (s == NULL) return;
    ssetlen(s, strlen(s));
    return;
}

/*
    Create a duplicate of the given null-terminated string.

    Return NULL if malloc fails.
    Return NULL if string causes overflow.
*/
string sdup(const string s) {
    return snewlen(s, sgetlen(s));
}

/*
    Join n C-strings together with separators of length seplen.

    Strings and separator must be null-terminated.
    If n does not match the amount of elements in the char** str array or
    seplen is not equal to the length of sep, behaviour is undefined.
    Return NULL if input causes overflow.
    Return NULL if malloc fails.
    Return a new string in which the given ones are joined together.

    This function is not byte-string safe.
*/
string sjoin(size_t n, const char* str[n], size_t seplen, const char* sep) {
    size_t curlen = 0;
    for (size_t i = 0; i < n; i++) {
        size_t len = strlen(str[i]);
        if (curlen + len + 1 < curlen)
            return NULL;
        curlen += len;
    }
    size_t slen = curlen + (n - 1) * seplen;
    if (slen < curlen)
        return NULL;

    string s = snewlen(NULL, slen);
    if (s == NULL)
        return NULL;

    char* p = s;
    for (size_t i = 0; i < n; i++) {
        size_t len = strlen(str[i]);
        memcpy(p, str[i], len);
        p += len;
        memcpy(p, sep, seplen);
        p += seplen;
    }
    s[slen] = 0;
    return s;
}

string sjoins(size_t n, const string str[n], size_t seplen, const char* sep) {
    return NULL;
}
