#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "safe_string.h"

int main(void) {
    // Header8 obj;
    // Header16 obj1;
    // Header32 obj2;
    // Header64 obj3;

    // printf("%zu\n", sizeof(obj));
    // printf("%zu\n", sizeof(obj1));
    // printf("%zu\n", sizeof(obj2));
    // printf("%zu\n", sizeof(obj3));

    string s = snewlen("abc", 3);
    printf("%c\n", s[1]);

    return 0;
}

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

string snewlen(const char* input, size_t ilen) {
    void* h;
    string str;
    uint8_t type = getReqType(ilen);
    // do something if ilen == 0 ?
    uint8_t hlen = getHlen(type);
    uint8_t* flag;
    
    h = malloc(hlen + ilen + 1);
    if (h == NULL) return NULL;
    // do something if input is NULL ?
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

string snew(const char* input) {
    // handle input == NULL
    size_t ilen = strlen(input);
    return snewlen(input, ilen);
}