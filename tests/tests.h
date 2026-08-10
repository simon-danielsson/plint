#ifndef TESTS_H
#define TESTS_H

#include "../src/main.h"

extern uint failures, total;

#define test_assert(cond)                                                      \
  do {                                                                         \
    char tmp[64];                                                              \
    snprintf(tmp, 64, "%s", __func__);                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "\033[4mFailure -> %-24s%s\033[0m\n", tmp, #cond);       \
      failures += 1;                                                           \
    } else {                                                                   \
      fprintf(stderr, "Success -> %-24s%s\n", tmp, #cond);                     \
    }                                                                          \
  } while (0)

#define TEST_DIV                                                               \
  do {                                                                         \
    for (uint j = 0; j < 3; j++) {                                             \
      fprintf(stderr, "┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄");                                    \
    }                                                                          \
    fprintf(stderr, "\n");                                                     \
  } while (0)

#endif
