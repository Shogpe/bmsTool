#include "utils.h"
#include <stdio.h>
#include <string.h>
time_t cvt_TIME(char const *Date) {
    char s_month[5];
    int month, day, year;
    struct tm t = {0};
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(Date, "%3s %d %d", s_month, &day, &year);
    month = (strstr(month_names, s_month)-month_names)/3;
    t.tm_mon = month;
    t.tm_mday = day;
    t.tm_year = year - 1900;
    t.tm_isdst = -1;

    return mktime(&t);
}
