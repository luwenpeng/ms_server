#include "ms_signal.h"

/***********************************************************
 * @Func   : ms_signal_toname()
 * @Author : lwp
 * @Brief  : 获取信号的名称。
 * @Param  : [in] signal
 * @Return : 字符串指针
 * @Note   :
 ***********************************************************/
const char *ms_signal_toname(int signal)
{
    switch (signal)
    {
        case SIGHUP    : return "SIGHUP";
        case SIGINT    : return "SIGINT";
        case SIGQUIT   : return "SIGQUIT";
        case SIGILL    : return "SIGILL";
        case SIGABRT   : return "SIGABRT";
        case SIGFPE    : return "SIGFPE";
        case SIGKILL   : return "SIGKILL";
        case SIGSEGV   : return "SIGSEGV";
        case SIGPIPE   : return "SIGPIPE";
        case SIGALRM   : return "SIGALRM";
        case SIGTERM   : return "SIGTERM";
        case SIGUSR1   : return "SIGUSR1";
        case SIGUSR2   : return "SIGUSR2";
        case SIGCHLD   : return "SIGCHLD";
        case SIGCONT   : return "SIGCONT";
        case SIGSTOP   : return "SIGSTOP";
        case SIGTSTP   : return "SIGTSTP";
        case SIGTTIN   : return "SIGTTIN";
        case SIGTTOU   : return "SIGTTOU";
        case SIGBUS    : return "SIGBUS";
        case SIGPOLL   : return "SIGPOLL";
        case SIGPROF   : return "SIGPROF";
        case SIGTRAP   : return "SIGTRAP";
        case SIGURG    : return "SIGURG";
        case SIGVTALRM : return "SIGVTALRM";
        case SIGXCPU   : return "SIGXCPU";
        case SIGXFSZ   : return "SIGXFSZ";
        case SIGWINCH  : return "SIGWINCH";
        case SIGSTKFLT : return "SIGSTKFLT";
        case SIGPWR    : return "SIGPWR";
        case SIGUNUSED : return "SIGUNUSED";
//      case SIGSYS    : return "SIGSYS";
//      case Synonym   : return "Synonym";
//      case SIGIOT    : return "SIGIOT";
//      case SIGEMT    : return "SIGEMT";
//      case SIGIO     : return "SIGIO";
//      case SIGCLD    : return "SIGCLD";
//      case SIGINFO   : return "SIGINFO";
//      case SIGLOST   : return "SIGLOST";
        default        : return "UNKNOWN";
    }
}
// @ms_signal_toname() ok

/***********************************************************
 * @Func   : ms_signal_init()
 * @Author : lwp
 * @Brief  : 初始化信号 handler。
 * @Param  : [in] signal_t : ms_signal_t 结构体数组
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   :
 ***********************************************************/
int ms_signal_init(ms_signal_t *signal_t)
{
    struct sigaction act;

    // 注册信号所对应的 handler
    for (ms_signal_t *t = signal_t; t->signal != -1; t++)
    {
        memset(&act, 0, sizeof(act));
        act.sa_handler = t->handler;
        if (sigemptyset(&act.sa_mask) == -1)
        {
            ms_errlog(MS_ERRLOG_ERR, errno, "sigemptyset() failed");
            return MS_ERROR;
        }

        if (sigaction(t->signal, &act, NULL) == -1)
        {
            ms_errlog(MS_ERRLOG_ERR, errno, "sigaction(\"%s\") failed",
                    ms_signal_toname(t->signal));
            return MS_ERROR;
        }
    }

    return MS_OK;
}
// @ms_signal_init() ok

/***********************************************************
 * @Func   : ms_signal_register()
 * @Author : lwp
 * @Brief  : 当前进程注册其关心的信号。
 * @Param  : [in] signals : 信号数组
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   :
 ***********************************************************/
int ms_signal_register(int *signals)
{
    sigset_t sigset;

    if (sigfillset(&sigset) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "sigfillset() failed");
        return MS_ERROR;
    }

    for (int *t = signals; *t != -1; t++)
    {
        if (sigdelset(&sigset, *t) == -1)
        {
            ms_errlog(MS_ERRLOG_ERR, errno, "sigdelset(\"%s\") failed",
                    ms_signal_toname(*t));
            return MS_ERROR;
        }
    }

    if (sigprocmask(SIG_BLOCK, &sigset, NULL) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "sigprocmask(SIG_BLOCK) failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_signal_register() ok
