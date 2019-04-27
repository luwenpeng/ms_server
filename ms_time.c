#include "ms_time.h"

static void ms_time_gmtime(const time_t t, struct tm *tp);

/***********************************************************
 * @Func   : ms_time_sec()
 * @Author : lwp
 * @Brief  : 获取秒时间戳。
 * @Param  : [in] NONE
 * @Return : NONE
 * @Note   :
 ***********************************************************/
uintptr_t ms_time_sec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec;
}
// @ms_time_sec() ok

/***********************************************************
 * @Func   : ms_time_ms()
 * @Author : lwp
 * @Brief  : 获取毫秒时间戳。
 * @Param  : [in] NONE
 * @Return : NONE
 * @Note   :
 ***********************************************************/
uintptr_t ms_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
// @ms_time_ms() ok

/***********************************************************
 * @Func   : ms_time_stamp()
 * @Author : lwp
 * @Brief  : 获取当前时间戳。
 * @Param  : [in] buff : buff 的起始地址
 * @Param  : [in] last : buff 的结束地址
 * @Return : buff 写入数据后的地址
 * @Note   :
 ***********************************************************/
char *ms_time_stamp(char *buff, const char *last)
{
    char *p = NULL;
    time_t sec;
    struct tm gmt;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec;          // 秒

    ms_time_gmtime(sec, &gmt);
    p = ms_str_slprintf(buff, last, TIME_STAMP_FORMAT,
            gmt.tm_year, gmt.tm_mon, gmt.tm_mday,
            gmt.tm_hour, gmt.tm_min, gmt.tm_sec, tv.tv_usec);

    return p;
}
// @ms_time_stamp() ok

/***********************************************************
 * @Func   : ms_time_gmtime()
 * @Author : lwp
 * @Brief  : 时间格式转换。
 * @Param  : [in] t : time_t 格式的秒数
 * @Param  : [in] tp : struct tm 格式的时间
 * @Return : NONE
 * @Note   :
 ***********************************************************/
static void ms_time_gmtime(const time_t t, struct tm *tp)
{
    intptr_t yday;
    uintptr_t days, leap, n, year, mon, mday, hour, min, sec;

    n = t;

    // 总天数
    days = n / 86400;

    // days 天 + n 秒
    n %= 86400;

    // days 天 + hour 小时
    hour = n / 3600;

    // days 天 + hour 小时 + n 秒
    n %= 3600;

    // days 天 + hour 小时 + min 分钟
    min = n / 60;

    // days 天 + hour 小时 + min 分钟 + sec 秒
    sec = n % 60;

    // the algorithm based on Gauss' formula
    days = days - (31 + 28) + 719527;
    year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);
    yday = days - (365 * year + year / 4 - year / 100 + year / 400);
    if (yday < 0)
    {
        // 闰年
        leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));
        yday = 365 + leap + yday;
        year--;
    }

    mon = (yday + 31) * 10 / 306;
    mday = yday - (367 * mon / 12 - 30) + 1;
    if (yday >= 306)
    {
        year++;
        mon -= 10;
    }
    else
    {
        mon += 2;
    }
    // end algorithm of Gauss' formula

    tp->tm_year = year;
    tp->tm_mon = mon;
    tp->tm_mday = mday;

    tp->tm_hour = hour + 8;
    tp->tm_min = min;
    tp->tm_sec = sec;
}
// @ms_time_gmtime() ok
