#ifndef _MS_CONF_H
#define _MS_CONF_H

#ifdef __cpluscplus
extern "C"
{
#endif

#define MS_OK     0
#define MS_ERROR -1
#define MS_BUSY  -2

#define MS_NAME        "lwp"
#define MS_VERSION     "0.1"
#define MS_SERVER_NAME (MS_NAME " " MS_VERSION)

#define MS_MAX_BUF_SIZE  4096
#define MS_MAX_FILE_PATH 4096

#define ms_min(val1, val2) (((val1) > (val2)) ? (val2) : (val1))
#define ms_max(val1, val2) (((val1) < (val2)) ? (val2) : (val1))

#define ms_test(format, ...)                                                   \
    fprintf(stdout, "# test " format " ", ##__VA_ARGS__);

#define ms_test_cond(c)                                                        \
    if(c)                                                                      \
        fprintf(stdout, "\033[42;30mPASSED\033[0m\n");                         \
    else                                                                       \
        fprintf(stdout, "\033[41;30mFAIlED\033[0m\n");

#ifdef DEBUG_SWITCH
    #define debug_log(format, ...)                                             \
        fprintf(stdout, "[%s-%s()-%05d] " format ".\n",                        \
                __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
    #define debug_log(format, ...)
#endif

#ifdef __cpluscplus
}
#endif

#endif
