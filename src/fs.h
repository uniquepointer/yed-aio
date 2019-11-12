#include "internal.h"

#define FT_UNKNOWN (0)
#define FT_TXT     (1)
#define FT_C       (2)
#define FT_CXX     (3)
#define FT_SH      (4)
#define FT_LATEX   (5)

typedef struct {
    int ft;
} yed_file;

int yed_get_ft(char *path);
