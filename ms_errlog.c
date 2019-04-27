#include "ms_errlog.h"

static char level_str[10][8] = {
    "status",
    "emerg",
    "alert",
    "crit",
    "error",
    "warn",
    "notice",
    "infor",
    "debug",
    "stdout"
};

typedef struct ms_errlog_s {
    int         fd;
    log_level_t level;
    char        file[MS_MAX_FILE_PATH];
} ms_errlog_t;

static ms_errlog_t ms_errlog;

static char *ms_errlog_errno(int err, char *buf, const char *last);

/***********************************************************
 * @Func   : ms_errlog_stderr()
 * @Author : lwp
 * @Brief  : 将错误信息输出到标准错误输出。
 * @Param  : [in] err : 错误码
 * @Param  : [in] fmt : 输出格式
 * @Param  : [in] ... : 可变参数
 * @Return : NONE
 * @Note   :
 ***********************************************************/
void ms_errlog_stderr(int err, const char *fmt, ...)
{
    ssize_t nwrite;
    va_list args;

    char *p;
    char *last;
    char  errstr[MS_MAX_BUF_SIZE * 2] = {0};

    last = errstr + sizeof(errstr);

    p = ms_str_slprintf(errstr, last, "%s%s%s %s[stderr]%s %s",
            MS_COLOR_HEAD1, MS_SERVER_NAME, MS_COLOR_TAIL,
            MS_COLOR_HEAD2, MS_COLOR_TAIL,
            MS_COLOR_HEAD4);

    va_start(args, fmt);
    p = ms_str_vslprintf(p, last, fmt, args);
    va_end(args);

    p = ms_str_slprintf(p, last, "%s", MS_COLOR_TAIL);

    if (err)
    {
        p = ms_str_slprintf(p, last, "%s", MS_COLOR_HEAD3);
        p = ms_errlog_errno(err, p, last);
        p = ms_str_slprintf(p, last, "%s", MS_COLOR_TAIL);
    }

    if (p > last - 1)
    {
        p = last - 1;
    }

    *p++ = '\n';

    do {
        nwrite = write(STDERR_FILENO, errstr, p - errstr);
    } while (nwrite == -1 && errno == EINTR);
}
// @ms_errlog_stderr() ok

/***********************************************************
 * @Func   : ms_errlog_stdout()
 * @Author : lwp
 * @Brief  : 将信息输出到标准输出。
 * @Param  : [in] fmt : 输出格式
 * @Param  : [in] ... : 可变参数
 * @Return : NONE
 * @Note   :
 ***********************************************************/
void ms_errlog_stdout(const char *fmt, ...)
{
    ssize_t nwrite;
    va_list args;

    char *p;
    char *last;
    char  errstr[MS_MAX_BUF_SIZE * 2] = {0};

    last = errstr + sizeof(errstr);

    p = ms_str_slprintf(errstr, last, "%s%s%s %s[stdout]%s %s",
            MS_COLOR_HEAD1, MS_SERVER_NAME, MS_COLOR_TAIL,
            MS_COLOR_HEAD3, MS_COLOR_TAIL,
            MS_COLOR_HEAD4);

    va_start(args, fmt);
    p = ms_str_vslprintf(p, last, fmt, args);
    va_end(args);

    p = ms_str_slprintf(p, last, "%s", MS_COLOR_TAIL);

    if (p > last - 1)
    {
        p = last - 1;
    }

    *p++ = '\n';

    do {
        nwrite = write(STDOUT_FILENO, errstr, p - errstr);
    } while (nwrite == -1 && errno == EINTR);
}
// @ms_errlog_stdout() ok

/***********************************************************
 * @Func   : ms_errlog_init()
 * @Author : lwp
 * @Brief  : 初始化错误日志。
 * @Param  : [in] file : 错误日志文件存储路径
 * @Param  : [in] level : 错误日志级别
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 若 file 文件路径长度大于 MS_MAX_FILE_PATH 则会被截断
 ***********************************************************/
int ms_errlog_init(const char *file, log_level_t level)
{
    // memset() 必须放在第一行
    memset(&ms_errlog, 0, sizeof(ms_errlog));

    if (NULL == file)
    {
        ms_errlog_stderr(0, "error log file path is \"NULL\"");
        return MS_ERROR;
    }

    if (strlen(file) == 0)
    {
        ms_errlog_stderr(0, "error log file path is empty");
        return MS_ERROR;
    }

    if (level < MS_ERRLOG_LEVEL_BEGIN || level > MS_ERRLOG_LEVEL_END)
    {
        ms_errlog_stderr(0, "invalid error log level \"%d\"", level);
        return MS_ERROR;
    }

    strncpy(ms_errlog.file, file, ms_min(strlen(file), MS_MAX_FILE_PATH));
    ms_errlog.level = level;

    ms_errlog.fd = open(ms_errlog.file, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (ms_errlog.fd == -1)
    {
        ms_errlog_stderr(errno, "open() \"%s\" failed", ms_errlog.file);
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_errlog_init() ok

/***********************************************************
 * @Func   : ms_errlog_core()
 * @Author : lwp
 * @Brief  : 将日志信息按照指定格式输出到标准输出/错误日志文件
 * @Param  : [in] level : 日志级别
 * @Param  : [in] err : 错误码
 * @Param  : [in] file : 产生当前日志的代码文件
 * @Param  : [in] line : 产生当前日志的代码行号
 * @Param  : [in] func : 产生当前日志的代码函数
 * @Param  : [in] fmt : 输出格式
 * @Param  : [in] ... : 可变参数列表
 * @Return : NONE
 * @Note   :
 ***********************************************************/
void ms_errlog_core(log_level_t level, int err, const char *file, int line,
        const char *func, const char *fmt, ...)
{
    ssize_t nwrite;
    char *p;
    char *last;
    char errstr[MS_MAX_BUF_SIZE * 2];
    va_list args;

    memset(&errstr, 0, sizeof(errstr));
    last = errstr + MS_MAX_BUF_SIZE * 2;

    // 获取时间戳
    p = ms_time_stamp(errstr, last);

#if (MS_ERRLOG_SHOW_FILE_LINE_FUNC)
    p = ms_str_slprintf(p, last, " [%s] [%P#%P] [%s-%05d-%s()] ",
            level_str[level],     // 日志级别
            getpid(),             // 进程 ID
            syscall(SYS_gettid),  // 线程 ID
            file,                 // 文件
            line,                 // 行号
            func);                // 函数
#else
    p = ms_str_slprintf(p, last, " [%s] [%P#%P] ",
            level_str[level],     // 日志级别
            getpid(),             // 进程 ID
            syscall(SYS_gettid)); // 线程 ID
#endif

    // 可变参数
    va_start(args, fmt);
    p = ms_str_vslprintf(p, last, fmt, args);
    va_end(args);

    // 错误码对应的描述
    if (err)
    {
        p = ms_errlog_errno(err, p, last);
    }

    if (p > last - 1)
    {
        p = last - 1;
    }

    *p++ = '\n';

    do {
        nwrite = write(ms_errlog.fd, errstr, p - errstr);
    } while (nwrite == -1 && errno == EINTR);
}
// @ms_errlog_core() ok

/***********************************************************
 * @Func   : ms_errlog_close()
 * @Author : lwp
 * @Brief  : 关闭错误日志文件句柄。
 * @Param  : [in] NONE
 * @Return : NONE
 * @Note   :
 ***********************************************************/
void ms_errlog_close(void)
{
    if (ms_errlog.fd > 0)
    {
        close(ms_errlog.fd);
    }
}
// @ms_errlog_close() ok

/***********************************************************
 * @Func   : ms_errlog_level()
 * @Author : lwp
 * @Brief  : 返回错误日志设置的日志过滤级别。
 * @Param  : [in] NONE
 * @Return : NONE
 * @Note   :
 ***********************************************************/
log_level_t ms_errlog_level(void)
{
    return ms_errlog.level;
}
// @ms_errlog_level() ok

/***********************************************************
 * @Func   : ms_errlog_reopen()
 * @Author : lwp
 * @Brief  : 关闭旧的文件句柄，重新打开日志文件。
 * @Param  : [in] NONE
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   :
 ***********************************************************/
int ms_errlog_reopen(void)
{
    int temp = 0;

    temp = open(ms_errlog.file, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (temp == -1)
    {
        ms_errlog_stderr(errno, "open() \"%s\" failed", ms_errlog.file);
        return MS_ERROR;
    }
    ms_errlog_close();
    ms_errlog.fd = temp;

    return MS_OK;
}
// @ms_errlog_reopen() ok

/***********************************************************
 * @Func   : ms_errlog_errno()
 * @Author : lwp
 * @Brief  : 根据错误码 err，将错误信息写入 buf 中，若空间不足，则截断。
 * @Param  : [in] err : 错误码
 * @Param  : [in] buf : buf 的起始地址
 * @Param  : [in] last : buf 的结束地址
 * @Return : buf 写入错误信息后的地址
 * @Note   :
 ***********************************************************/
static char *ms_errlog_errno(int err, char *buf, const char *last)
{
    char *p = NULL;

    // 最长的 errno_str 为 49
    if (buf > last - 50)
    {
        // 腾出空间记录错误码
        buf = (char *)last - 50;
        *buf++ = '.';
        *buf++ = '.';
        *buf++ = '.';
    }

    // 记录错误码和错误描述
    p = ms_str_slprintf(buf, last, " (%d: %s)", err, ms_errno_str(err));

    return p;
}
// @ms_errlog_errno() ok
