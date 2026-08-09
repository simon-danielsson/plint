#ifndef TESTS_H
#define TESTS_H

void _run_tests(void);

#define test_assert(cond)                                                      \
  do {                                                                         \
    char tmp[64];                                                              \
    snprintf(tmp, 64, "%s()", __func__);                                       \
    if (!(cond)) {                                                             \
      fprintf(stderr, "\033[4mFailure -> %-18s%s\033[0m\n", tmp, #cond);       \
    } else {                                                                   \
      fprintf(stderr, "Success -> %-18s%s\n", tmp, #cond);                     \
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
