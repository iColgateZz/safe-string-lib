/* safe_stringing.h */

/* Include libs */
#include <stdint.h>

/* Definitions */
#ifndef SAFE_STRING_H
#define SAFE_STRING_H

typedef char* string;

typedef struct Header8 {
    uint8_t len;
    uint8_t allocated;
    uint8_t type;
    char buf[];
} Header8;

typedef struct Header16 {
    uint16_t len;
    uint16_t allocated;
    uint8_t type;
    char buf[];
} Header16;

typedef struct Header32 {
    uint32_t len;
    uint32_t allocated;
    uint8_t type;
    char buf[];
} Header32;

typedef struct Header64 {
    uint64_t len;
    uint64_t allocated;
    uint8_t type;
    char buf[];
} Header64;

/* Exposed functions */

string snew(const char*);
string snewlen(const char*, size_t);
void sfree(const string);
size_t sgetlen(const string);
void supdatelen(const string);
string sdup(const string);
string sjoin(size_t n, const char* str[n], size_t, const char*);
string sjoins(size_t n, const string str[n], size_t, const char*);
string scat(const char*, const char*);
string scats(const string s1, const string s2);

#endif 
