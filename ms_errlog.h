#ifndef _MS_ERRLOG_H
#define _MS_ERRLOG_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_errno.h"
#include "ms_time.h"

#define ms_errlog(level, err, ...)                                             \
    if (ms_errlog_level() >= level)                                            \
    {                                                                          \
        if (ms_errlog_level() == MS_ERRLOG_STDOUT)                             \
        {                                                                      \
            if ((level <= MS_ERRLOG_ERR && level != MS_ERRLOG_STATUS) || err)  \
            {                                                                  \
                ms_errlog_stderr(err, __VA_ARGS__);                            \
            }                                                                  \
            else                                                               \
            {                                                                  \
                ms_errlog_stdout(__VA_ARGS__);                                 \
            }                                                                  \
        }                                                                      \
        ms_errlog_core(level, err, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    }

#define MS_COLOR_HEAD1 "\033[40;33m" // 黄色
#define MS_COLOR_HEAD2 "\033[40;31m" // 红色
#define MS_COLOR_HEAD3 "\033[40;32m" // 绿色
#define MS_COLOR_HEAD4 "\033[40;34m" // 蓝色
#define MS_COLOR_TAIL  "\033[1m"

#ifdef DEBUG_SWITCH
    #define MS_ERRLOG_SHOW_FILE_LINE_FUNC 1
#else
    #define MS_ERRLOG_SHOW_FILE_LINE_FUNC 0
#endif

typedef enum ms_errlog_level_s {
    MS_ERRLOG_STATUS = 0,
    MS_ERRLOG_EMERG  = 1,
    MS_ERRLOG_ALERT  = 2,
    MS_ERRLOG_CRIT   = 3,
    MS_ERRLOG_ERR    = 4,
    MS_ERRLOG_WARN   = 5,
    MS_ERRLOG_NOTICE = 6,
    MS_ERRLOG_INFO   = 7,
    MS_ERRLOG_DEBUG  = 8,
    MS_ERRLOG_STDOUT = 9
} log_level_t;

#define MS_ERRLOG_LEVEL_BEGIN MS_ERRLOG_STATUS
#define MS_ERRLOG_LEVEL_END MS_ERRLOG_STDOUT

void ms_errlog_stderr(int err, const char *fmt, ...);
void ms_errlog_stdout(const char *fmt, ...);

int ms_errlog_init(const char *file, log_level_t level);
void ms_errlog_core(log_level_t level, int err, const char *file,
        int line, const char *func, const char *fmt, ...);
void ms_errlog_close(void);

log_level_t ms_errlog_level(void);
int ms_errlog_reopen(void);

#ifdef __cpluscplus
}
#endif

#endif
