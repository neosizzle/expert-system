#ifndef __FT_MACROS__H__
#define __FT_MACROS__H__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define EPRINTF(args...) fprintf(stderr, ##args);
#define DIE_OS(status, args...) EPRINTF(args); perror(""); exit(status);
#define DIE(status, args...) EPRINTF(args); exit(status);

#endif  //!__FT_MACROS__H__