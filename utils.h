#ifndef UTILS_H
#define UTILS_H
#include <time.h>

#ifdef  __cplusplus
extern "C" {
#endif
#define TIME_OUTOFDATE 24 * 31 * 24 * 60 * 60

time_t cvt_TIME(char const *Date);


#ifdef  __cplusplus
}
#endif
#endif // UTILS_H
