/* safe_string.c */

/* Include libs */
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "safe_string.h"
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

/* Definitions */
#define H_TYPE_8 0
#define H_TYPE_16 1
#define H_TYPE_32 2
#define H_TYPE_64 3
#define H_MASK 3

#define HDR(T, s) ((Header##T *)(s - sizeof(Header##T)))

/* Functions */

static inline
uint8_t getReqType(const size_t ilen) {
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
uint8_t getHlen(const uint8_t type) {
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
void ssetlen(const string s, const size_t len) {
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
void ssetalloc(const string s, const size_t newalloc) {
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
string snewlen(const void* input, size_t ilen) {
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

    This function is not binary safe.
*/
string snew(const void* input) {
    if (input == NULL) return NULL;
    size_t ilen = strlen(input);
    return snewlen(input, ilen);
}

/*
    Free the allocated memory.

    If input is NULL, do nothing.
*/
void sfree(const string s) {
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

    This function is not binary safe.
*/
void supdatelen(const string s) {
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

    Return NULL if input causes overflow.
    Return NULL if malloc fails.
    Return NULL if one of the given strings is NULL.
    Return NULL if sep is NULL.
    Return NULL if n < 2.
    Return a new string in which the given ones are joined together.

    Strings and separator must be null-terminated.
    If n does not match the amount of elements in the char* array or
    seplen is not equal to the length of sep, behaviour is undefined.

    This function is not binary safe.
*/
string sjoin(size_t n, const char* str[n], size_t seplen, const char* sep) {
    if (n < 2) return NULL;
    if (sep == NULL) return NULL;
    size_t curlen = 0;
    for (size_t i = 0; i < n; i++) {
        if (str[i] == NULL) return NULL;
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
        if (i < n - 1) {
            memcpy(p, sep, seplen);
            p += seplen;
        }
    }
    s[slen] = 0;
    return s;
}

/*
    Join n strings together with separators of length seplen.

    Return NULL if input causes overflow.
    Return NULL if malloc fails.
    Return NULL if one of the given strings is NULL.
    Return NULL if sep is NULL.
    Return NULL if n < 2.
    Return a new string in which the given ones are joined together.

    Separator must be null-terminated.
    If n does not match the amount of elements in the string array or
    seplen is not equal to the length of sep, behaviour is undefined.
*/
string sjoins(size_t n, const string str[n], size_t seplen, const char* sep) {
    if (n < 2) return NULL;
    if (sep == NULL) return NULL;
    size_t curlen = 0;
    for (size_t i = 0; i < n; i++) {
        if (str[i] == NULL) return NULL;
        size_t len = sgetlen(str[i]);
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
        size_t len = sgetlen(str[i]);
        memcpy(p, str[i], len);
        p += len;
        if (i < n - 1) {
            memcpy(p, sep, seplen);
            p += seplen;
        }
    }
    s[slen] = 0;
    return s;
}

/*
    Concatenate 2 C-strings.

    Return NULL if strings cause overflow.
    Return NULL if malloc fails.
    Return NULL if any of the strings is NULL.
    Return a new concatenated null-terminated string.

    This function is not binary safe.
*/
string scat(const char* s1, const char* s2) {
    if (s1 == NULL || s2 == NULL) return NULL;
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    if (len1 + len2 < len1 || len1 + len2 < len2)
        return NULL;
    string s = snewlen(NULL, len1 + len2);
    if (s == NULL)
        return NULL;
    memcpy(s, s1, len1);
    memcpy(s + len1, s2, len2);
    s[len1 + len2] = 0;
    return s;
}

/*
    Concatenate 2 strings.

    Return NULL if strings cause overflow.
    Return NULL if malloc fails.
    Return NULL if any of the strings is NULL.
    Return a new concatenated null-terminated string.
*/
string scats(const string s1, const string s2) {
    if (s1 == NULL || s2 == NULL) return NULL;
    size_t len1 = sgetlen(s1);
    size_t len2 = sgetlen(s2);
    if (len1 + len2 < len1 || len1 + len2 < len2)
        return NULL;
    string s = snewlen(NULL, len1 + len2);
    if (s == NULL)
        return NULL;
    memcpy(s, s1, len1);
    memcpy(s + len1, s2, len2);
    s[len1 + len2] = 0;
    return s;
}

/*
    Change all characters to upper case.

    Return false if string is NULL.
    Return true on success.
*/
bool supper(string s) {
    if (s == NULL) return false;
    for (size_t i = 0; i < sgetlen(s); i++)
        s[i] = toupper(s[i]);
    return true;
}

/*
    Change all characters to lower case.

    Return false if string is NULL.
    Return true on success.
*/
bool slower(string s) {
    if (s == NULL) return false;
    for (size_t i = 0; i < sgetlen(s); i++)
        s[i] = tolower(s[i]);
    return true;
}

/*
    Check if a string starts with 'pattern'.

    Return false if string or pattern is NULL.
    Return false if plen > len(string).
    Return false if plen is 0.

    Behaviour is undefined if plen != len(pattern).
*/
bool sstartswith(string s, size_t plen, const char* pattern) {
    if (s == NULL || pattern == NULL)
        return false;
    if (plen > sgetlen(s) || plen == 0)
        return false;
    return memcmp(s, pattern, plen) == 0;
}

/*
    Check if a string ends with 'pattern'.

    Return false if string or pattern is NULL.
    Return false if plen > len(string).
    Return false if plen is 0.

    Behaviour is undefined if plen != len(pattern).
*/
bool sendswith(string s, size_t plen, const char* pattern) {
    if (s == NULL || pattern == NULL)
        return false;
    if (plen > sgetlen(s) || plen == 0)
        return false;
    return memcmp(s + sgetlen(s) - plen, pattern, plen) == 0;
}

/*
    Find the starting index of the first substring matching the 'pattern'.

    Return -1 if s or pattern is NULL.
    Return -1 if plen > len(s).
    Return -1 if plen == 0.
    Return -1 if pattern is not found.

    Behaviour is undefined if plen != len(pattern).
*/
ssize_t sfind(string s, size_t plen, const char* pattern) {
    if (s == NULL || pattern == NULL)
        return -1;
    if (plen > sgetlen(s) || plen == 0)
        return -1;
    for (size_t idx = 0; idx <= sgetlen(s) - plen; idx++) {
        if (memcmp(s + idx, pattern, plen) == 0)
            return idx;
    }
    return -1;
}

/*
    Find the starting index of the last (right) substring matching the 'pattern'.

    Return -1 if s or pattern is NULL.
    Return -1 if plen > len(s).
    Return -1 if plen == 0.

    Behaviour is undefined if plen != len(pattern).
*/
ssize_t srfind(string s, size_t plen, const char* pattern) {
    if (s == NULL || pattern == NULL)
        return -1;
    if (plen > sgetlen(s) || plen == 0)
        return -1;
    for (ssize_t idx = sgetlen(s) - plen; idx >= 0; idx--) {
        if (memcmp(s + idx, pattern, plen) == 0)
            return idx;
    }
    return -1;
}

/*
    Count the amount of substrings matching 'pattern'.

    Return -1 if s or pattern is NULL.
    Return -1 if plen > len(s).
    Return -1 if plen == 0.

    Behaviour is undefined if plen != len(pattern).
*/
ssize_t scount(string s, size_t plen, const char* pattern) {
    if (s == NULL || pattern == NULL)
        return -1;
    if (plen > sgetlen(s) || plen == 0)
        return -1;
    size_t count = 0;
    for (size_t idx = 0; idx <= sgetlen(s) - plen; idx++) {
        if (memcmp(s + idx, pattern, plen) == 0)
            count++;
    }
    return count;
}

/*
    Remove the given pattern from the beginning and the end of the string.

    Return false if s or pattern is NULL.
    Return false if plen > len(s) or plen is 0.
    Return true on success.

    Behaviour is undefined if plen != len(pattern).
*/
bool strim(string s, size_t plen, const char* pattern) {
    if (s == NULL || pattern == NULL)
        return false;
    size_t len = sgetlen(s);
    if (plen > len || plen == 0)
        return false;
    size_t newstart = 0;
    while (newstart + plen <= len && memcmp(s + newstart, pattern, plen) == 0)
        newstart += plen;
    size_t rm_end = 1;
    while (newstart < len - rm_end * plen && 
           memcmp(s + len - rm_end * plen, pattern, plen) == 0)
        rm_end++;
    size_t newlen = len - newstart - (rm_end - 1) * plen;
    if (newstart > 0)
        memmove(s, s + newstart, newlen);
    s[newlen] = 0;
    ssetlen(s, newlen);
    return true;
}

/*
    Remove the given pattern from the string.

    Return false if s or pattern is NULL.
    Return false if plen > len(s) or plen is 0.
    Return true on success.

    Behaviour is undefined if plen != len(pattern).
*/
bool sremove(string s, size_t plen, const char* pattern) {
    if (s == NULL || pattern == NULL)
        return false;
    size_t slen = sgetlen(s);
    if (plen > slen || plen == 0)
        return false;
    size_t idx = 0;
    size_t i = 0;
    while (i < slen) {
        if (slen - i >= plen && memcmp(s + i, pattern, plen) == 0)
            i += plen;
        else
            s[idx++] = s[i++];
    }
    ssetlen(s, idx);
    s[idx] = 0;
    return true;
}

string sslice(string s, size_t start, size_t end) {
    if (s == NULL) return NULL;
    if (start >= end) return NULL;

    string new = snewlen(NULL, end - start);
    if (new == NULL) return NULL;
    for (size_t i = 0; i < end - start; i++)
        new[i] = s[start + i];
    new[end - start] = 0;
    return new;
}

string sbite(string s, size_t plen, const char* pattern) {
    ssize_t idx = sfind(s, plen, pattern);
    if (idx == -1) return NULL;
    return NULL;
}

void sreplace(void);
string* ssplit(const string s, size_t seplen, char* sep);
