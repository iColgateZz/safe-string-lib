/* safe_stringing.h */

/* Include libs */
#include <stdint.h>
#include <stdlib.h>

/* Definitions */
#define H_TYPE_8 0
#define H_TYPE_16 1
#define H_TYPE_32 2
#define H_TYPE_64 3
#define H_MASK 3

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
