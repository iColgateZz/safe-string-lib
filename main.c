#include <stdio.h>
#include "safe_string.c"

int main(void) {
    string s = snewlen("abc", 3);
    printf("%zu\n", sgetlen(s));
    sfree(s);
    return 0;
}