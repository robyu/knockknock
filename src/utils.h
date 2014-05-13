
#ifndef UTILS_H
#define UTILS_H

#include <assert.h>


void utils_log(const char *fmt, ... );
void utils_print_float(float value, int places);


#define UTILS_INT_ABS(x) ((x >= 0) ? x : -x)

void utils_assert(char* pfilename, int line_number, int arg);
#define UTILS_ASSERT(arg)  utils_assert(__FILE__,__LINE__,arg)


#endif

