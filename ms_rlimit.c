#include "ms_rlimit.h"

/***********************************************************
 * @Func   : ms_rlimit_set()
 * @Author : lwp
 * @Brief  : 设置进程最大打开文件数。
 * @Param  : [in] max_process_open_files : 进程最大打开文件数
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   :
 ***********************************************************/
int ms_rlimit_set(rlim_t max_process_open_files)
{
    struct rlimit rt;

    memset(&rt, 0, sizeof(rt));
    rt.rlim_max = max_process_open_files; // soft limit
    rt.rlim_cur = max_process_open_files; // hard limit
    if (setrlimit(RLIMIT_NOFILE, &rt) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setrlimit(RLIMIT_NOFILE) failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_rlimit_set() ok
