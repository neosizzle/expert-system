#ifndef __FT_MACROS__H__
#define __FT_MACROS__H__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define INDENT_SIZE 1024
#define ALPHA_COUNT 26
#define IMPL_RESOLVER " => "
#define IFF_RESOLVER " <=> "

#ifdef __DEBUG__

#define DBG(indent, args...)  \
    do                        \
    {                         \
        printf("%s", indent); \
        printf(args);         \
    } while (0);

#else

#define DBG(indent, args...) \
    do                       \
    {                        \
    } while (0);

#endif //__DEBUG__

#define EPRINTF(args...) fprintf(stderr, ##args);
#define DIE_OS(status, args...) \
    EPRINTF(args);              \
    perror("");                 \
    exit(status);
#define DIE(status, args...) \
    EPRINTF(args);           \
    exit(status);

#endif //!__FT_MACROS__H__