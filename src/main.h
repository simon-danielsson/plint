#ifndef MAIN_H
#define MAIN_H

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* to make use of the 'static' keyword more explicit */
#define global_var static
#define intern_fn static

/* for use with counters and such */
typedef uint32_t uint;

#if defined(NDEBUG)
#define BUILD_RELEASE 1
#define BUILD_DEBUG 0
#else
#define BUILD_RELEASE 0
#define BUILD_DEBUG 1
#endif

#ifndef ENV_PROJECT
#define ENV_PROJECT "UNDEFINED"
#endif
#ifndef ENV_CONTACT
#define ENV_CONTACT "UNDEFINED"
#endif
#ifndef ENV_AUTHOR
#define ENV_AUTHOR "UNDEFINED"
#endif
#ifndef ENV_DESCR
#define ENV_DESCR "UNDEFINED"
#endif
#ifndef ENV_WEBSITE
#define ENV_WEBSITE "UNDEFINED"
#endif
#ifndef ENV_GIT_TAG
#define ENV_GIT_TAG "UNDEFINED"
#endif
#ifndef ENV_GIT_HASH
#define ENV_GIT_HASH "UNDEFINED"
#endif
#ifndef ENV_GIT_REPO_URL
#define ENV_GIT_REPO_URL "UNDEFINED"
#endif
#endif
