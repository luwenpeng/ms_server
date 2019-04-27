#ifndef _MS_TIME_H
#define _MS_TIME_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_str.h"

// 输出格式：20181113-18:46:37-870165
#define TIME_STAMP_FORMAT "%4d%02d%02d-%02d:%02d:%02d-%06d"

// 输出格式：2018/11/13-18:46:05-570066
// #define TIME_STAMP_FORMAT "%4d/%02d/%02d-%02d:%02d:%02d-%06d"

// 输出格式：2018/11/13-18:45:20
// #define TIME_STAMP_FORMAT "%4d/%02d/%02d-%02d:%02d:%02d"

uintptr_t ms_time_sec(void);
uintptr_t ms_time_ms(void);
char *ms_time_stamp(char *buff, const char *last);

#ifdef __cpluscplus
}
#endif

#endif
