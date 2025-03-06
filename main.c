#include <stdio.h>
#include "safe_string.h"
#include <string.h>

int test_count = 0;
int fail_count = 0;

void assert_equal(int condition, const char *message, const char *function_name) {
    test_count++;
    if (!condition) {
        printf("Test failed in %s: %s\n", function_name, message);
        fail_count++;
    }
}

void test_snewlen_as_intended() {
    string new = snewlen("abc", 3);
    assert_equal(new != NULL, "Expected non-NULL string", __func__);
    assert_equal(new[0] == 'a', "Expected first character to be 'a'", __func__);
    assert_equal(new[3] == 0, "Expected fourth character to be null terminator", __func__);
    assert_equal(strncmp(new, "abc", 3) == 0, "Expected string to match 'abc'", __func__);
    sfree(new);
}

void test_snewlen_null_string_passed() {
    string new = snewlen(NULL, 100);
    assert_equal(new != NULL, "Expected non-NULL string", __func__);
    assert_equal(new[0] == 0, "First byte is not zero", __func__);
    assert_equal(new[100] == 0, "Last byte is not zero", __func__);
    assert_equal(new[50] == 0, "Mid byte is not zero", __func__);
    assert_equal(sgetlen(new) == 100, "Returned length is not 100", __func__);
    sfree(new);
}

void test_snewlen_different_lengths() {
    string f = snewlen("abcde", 0);
    assert_equal(f != NULL, "Expected non-NULL string", __func__);
    assert_equal(strncmp("abcde", f, 0) == 0, "The strings must be equal 1", __func__);
    assert_equal(sgetlen(f) == 0, "Length must be 0", __func__);
    assert_equal(f[0] == 0, "Must be an empty string", __func__);
    sfree(f);

    string s = snewlen("abcde", 300);
    assert_equal(s != NULL, "Expected non-NULL string", __func__);
    // Because strncmp() is designed for comparing strings rather 
    // than binary data, characters that appear after a ‘\0’ character are not compared
    assert_equal(strncmp("abcde", s, 300) == 0, "The strings must be equal 2", __func__);
    assert_equal(sgetlen(s) == 300, "Length must be 300", __func__);
    assert_equal(f[300] == 0, "Must be a null-terminated string", __func__);
    sfree(s);

    string t = snewlen("abcde", 1ll << 15);
    assert_equal(t != NULL, "Expected non-NULL string", __func__);
    assert_equal(sgetlen(t) == 32768, "Lengths do not match", __func__);
    sfree(t);
}

int main(void) {
    printf("Running tests...\n");

    test_snewlen_as_intended();
    test_snewlen_null_string_passed();
    test_snewlen_different_lengths();

    printf("\nTests run: %d, Failures: %d\n", test_count, fail_count);
    if (fail_count == 0) {
        printf("All tests passed!\n");
    } else {
        printf("Some tests failed.\n");
    }
    return fail_count > 0 ? 1 : 0;
}
