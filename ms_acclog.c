#include "ms_acclog.h"

typedef struct ms_acclog_s {
    int fd;
    char file[MS_MAX_FILE_PATH];
} ms_acclog_t;

static ms_acclog_t g_ms_acclog_t;

/***********************************************************
 * @Func   : ms_acclog_init()
 * @Author : lwp
 * @Brief  : 初始化访问日志。
 * @Param  : [in] file : 访问日志文件存储路径
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 若 file 文件路径长度大于 MS_MAX_FILE_PATH 则会被截断
 ***********************************************************/
int ms_acclog_init(const char *file)
{
    // memset() 必须放在第一行
    memset(&g_ms_acclog_t, 0, sizeof(g_ms_acclog_t));

    if (NULL == file)
    {
        ms_errlog_stderr(0, "access log file path is \"NULL\"");
        return MS_ERROR;
    }

    if (strlen(file) == 0)
    {
        ms_errlog_stderr(0, "access log file path is empty");
        return MS_ERROR;
    }

    strncpy(g_ms_acclog_t.file, file, ms_min(strlen(file), MS_MAX_FILE_PATH));
    g_ms_acclog_t.fd = open(g_ms_acclog_t.file, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (g_ms_acclog_t.fd == -1)
    {
        ms_errlog_stderr(errno, "open() \"%s\" failed", g_ms_acclog_t.file);
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_acclog_init() ok

/***********************************************************
 * @Func   : ms_acclog()
 * @Author : lwp
 * @Brief  : 将日志信息按照指定格式输出到日志文件
 * @Param  : [in] fmt : 输出格式
 * @Param  : [in] ... : 可变参数列表
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_acclog(const char *fmt, ...)
{
    ssize_t nwrite;
    char *p;
    char *last;
    char accstr[MS_MAX_BUF_SIZE * 2];
    va_list args;

    memset(&accstr, 0, sizeof(accstr));
    last = accstr + sizeof(accstr);

    p = ms_time_stamp(accstr, last);
    p = ms_str_slprintf(p, last, " [%P#%P] ", getpid(), syscall(SYS_gettid));

    // 可变参数
    va_start(args, fmt);
    p = ms_str_vslprintf(p, last, fmt, args);
    va_end(args);

    if (p > last - 1)
    {
        p = last - 1;
    }

    *p++ = '\n';

    do {
        nwrite = write(g_ms_acclog_t.fd, accstr, p - accstr);
    } while (nwrite == -1 && errno == EINTR);
}
// @ms_acclog() ok

/***********************************************************
 * @Func   : ms_acclog_close()
 * @Author : lwp
 * @Brief  : 关闭访问日志文件句柄。
 * @Param  : [in] NONE
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_acclog_close(void)
{
    if (g_ms_acclog_t.fd > 0)
    {
        close(g_ms_acclog_t.fd);
    }
}
// @ms_acclog_close() ok

/***********************************************************
 * @Func   : ms_acclog_reopen()
 * @Author : lwp
 * @Brief  : 关闭旧的文件句柄，重新打开日志文件。
 * @Param  : [in] NONE
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_acclog_reopen(void)
{
    int temp = 0;

    temp = open(g_ms_acclog_t.file, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (temp == -1)
    {
        ms_errlog_stderr(errno, "open() \"%s\" failed", g_ms_acclog_t.file);
        return MS_ERROR;
    }
    ms_acclog_close();
    g_ms_acclog_t.fd = temp;

    return MS_OK;
}
// @ms_acclog_reopen() ok
