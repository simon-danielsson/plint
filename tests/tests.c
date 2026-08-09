#include "../src/main.h"

static void test1(void) { test_assert(true); }
static void test2(void) { test_assert(false); }
static void test3(void) { test_assert(true); }

typedef void (*test_fn)(void);

static const test_fn tests[] = {
    test1,
    test2,
    test3,
    NULL,
};

void _run_tests(void) {
    TEST_DIV;
    uint i = 0;
    while (tests[i]) {
        tests[i]();
        i++;
    }
    TEST_DIV;
    exit(0);
}
