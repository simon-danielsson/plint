#include "main.h"

int main(int argc, char **argv) {
#ifdef TEST
    _run_tests();
#endif

    printf("Hello, world!");
    return 0;
}
