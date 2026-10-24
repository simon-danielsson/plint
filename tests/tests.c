#include "../src/main.h"

// global ---------------------------------------------------------------------

typedef void (*test_fn)(const char *title);
typedef struct {
    test_fn f;
    const char *title;
} _TEST_FUNC;
uint _tests_passed = 0, _tests_total = 0;
void test_assert(const char *name, bool assertion);

// tests ----------------------------------------------------------------------

internal void test1(const char *title) { test_assert(title, 1); }

internal void test2(const char *title) { test_assert(title, 0); }

internal void test3(const char *title) { test_assert(title, 1); }

// test array -----------------------------------------------------------------

#ifdef TEST

internal void _test_header(const char *title) {
    fprintf(stderr, "\n%s\n", title);
}

#define _append_test(func, title)                                                  (_TEST_FUNC) { (func), (title) }

global _TEST_FUNC tests[] = {
    _append_test(_test_header, "generic tests"),
    _append_test(test1, "first test"),
    _append_test(test2, "the second test"),
    _append_test(test3, "a last test"),
    _append_test(NULL, NULL),
};

// backend --------------------------------------------------------------------

__attribute__((constructor)) internal void _run_tests(void) {
    int i = 0;
    while (tests[i].f)
        tests[i].f(tests[i].title), i++;
    if (_tests_passed == _tests_total) {
        fprintf(stderr, "\nAll tests passed!\n");
        exit(EXIT_SUCCESS);
    } else {
        fprintf(stderr, "\n%d/%d tests passed.\n", _tests_passed, _tests_total);
        exit(EXIT_FAILURE);
    }
}

#endif // #ifdef TEST

void test_assert(const char *name, bool assertion) {
    _tests_total++;
    size_t name_len = strlen(name);
    fprintf(stderr, "|   %s ", name);
    for (size_t i = 0; i < 32 - name_len; i++)
        fprintf(stderr, ".");
    if (assertion) {
        fprintf(stderr, " Ok\n");
        _tests_passed += 1;
    } else {
        fprintf(stderr, " Failed\n");
    }
}
