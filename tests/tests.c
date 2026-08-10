#include "tests.h"

#ifdef TEST

uint failures = 0, total = 0;

intern_fn void test1(void) { test_assert(true); }
intern_fn void test2(void) { test_assert(false); }
intern_fn void test3(void) { test_assert(true); }

typedef void (*test_fn)(void);

global_var test_fn tests[] = {
    test1,
    test2,
    test3,
    NULL,
};

__attribute__((constructor)) intern_fn void _run_tests(void) {
    TEST_DIV;
    while (tests[total]) {
        tests[total]();
        total++;
    }
    TEST_DIV;
    printf("Failure rate: %.02f%%\n", ((float)failures / total) * 100);
    exit(0);
}
#endif
