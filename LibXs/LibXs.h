

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdint.h>
#include <time.h>


typedef unsigned int uint;
typedef unsigned int uint32;

typedef int64_t int64;

#ifndef _MSC_VER
typedef unsigned long DWORD;
#endif



/* Have our own assert, so we are sure it does not get optimized away in
* a release build.
*/
#define XX_ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort();                                              \
  }                                                       \
 } while (0)

#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))


/* Die with fatal error. */
#define XX_FATAL(msg)                                        \
  do {                                                    \
    fprintf(stderr,                                       \
            "Fatal error in %s on line %d: %s\n",         \
            __FILE__,                                     \
            __LINE__,                                     \
            msg);                                         \
    fflush(stderr);                                       \
    abort();                                              \
  } while (0)







