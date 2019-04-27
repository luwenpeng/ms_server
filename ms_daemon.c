#include "ms_daemon.h"

/***********************************************************
 * @Func   : ms_daemon()
 * @Author : lwp
 * @Brief  : 创建守护进程。
 * @Param  : [in] NONE
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   :
 ***********************************************************/
int ms_daemon(void)
{
    int fd;

    switch (fork())
    {
        // 失败
        case -1:
            ms_errlog(MS_ERRLOG_ERR, errno, "fork() failed");
            return MS_ERROR;
        // 子进程
        case 0:
            break;
        // 父进程
        default:
            exit(0);
    }

    if (setsid() == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsid() failed");
        return MS_ERROR;
    }

    umask(0);

    // 以读写模式打开 /dev/null
    fd = open("/dev/null", O_RDWR);
    if (fd == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "open(\"/dev/null\") failed");
        return MS_ERROR;
    }

    // 将标准输入关联到 /dev/null
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "dup2(\"STDIN_FILENO\") failed");
        return MS_ERROR;
    }

    // 将标准输出关联到 /dev/null
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "dup2(\"STDOUT_FILENO\") failed");
        return MS_ERROR;
    }

    // 将标准错误关联到 /dev/null
    if (dup2(fd, STDERR_FILENO) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "dup2(\"STDERR_FILENO\") failed");
        return MS_ERROR;
    }

    // 关闭 /dev/null 的文件句柄
    if (fd > STDERR_FILENO)
    {
        if (close(fd) == -1)
        {
            ms_errlog(MS_ERRLOG_ERR, errno, "close() failed");
            return MS_ERROR;
        }
    }

    return MS_OK;
}
// @ms_daemon() ok

/***********************************************************
 * @Func   : ms_daemon_set_pid()
 * @Author : lwp
 * @Brief  : 将 pid 计入 pid_file 文件中。
 * @Param  : [in] pid_file
 * @Param  : [in] pid
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   :
 ***********************************************************/
int ms_daemon_set_pid(const char *pid_file, pid_t pid)
{
    FILE *fp = NULL;

    fp = fopen(pid_file, "w+");
    if (NULL == fp)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "fopen(\"%s\") failed", pid_file);
        return MS_ERROR;
    }

    fprintf(fp, "%d\n", pid);
    fclose(fp);

    return MS_OK;
}
// @ms_daemon_set_pid() ok

/***********************************************************
 * @Func   : ms_daemon_get_pid()
 * @Author : lwp
 * @Brief  : 从 pid_file 文件中获取 pid。
 * @Param  : [in] pid_file
 * @Return : MS_ERROR : 失败
 *           pid      : 成功
 * @Note   :
 ***********************************************************/
pid_t ms_daemon_get_pid(const char *pid_file)
{
    pid_t pid = 0;
    FILE *fp = NULL;

    fp = fopen(pid_file, "r");
    if (NULL == fp)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "fopen(\"%s\") failed", pid_file);
        return MS_ERROR;
    }

    if (fscanf(fp, "%d", &pid) != 1)
    {
        fclose(fp);
        return MS_ERROR;
    }

    if (pid <= 0)
    {
        fclose(fp);
        return MS_ERROR;
    }

    if (kill(pid, 0) == -1)
    {
        fclose(fp);
        return MS_ERROR;
    }

    fclose(fp);
    return pid;
}
// @ms_daemon_get_pid() ok

/***********************************************************
 * @Func   : ms_daemon_clean_pid()
 * @Author : lwp
 * @Brief  : 清空 pid_file 文件。
 * @Param  : [in] pid_file
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   :
 ***********************************************************/
int ms_daemon_clean_pid(const char *pid_file)
{
    FILE *fp = NULL;

    fp = fopen(pid_file, "w");
    if (NULL == fp)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "fopen(\"%s\") failed", pid_file);
        return MS_ERROR;
    }

    fclose(fp);
    return MS_OK;
}
// @ms_daemon_clean_pid() ok
