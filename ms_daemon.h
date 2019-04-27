#ifndef _MS_DAEMON_H
#define _MS_DAEMON_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_errlog.h"

int ms_daemon(void);
int ms_daemon_set_pid(const char *pid_file, pid_t pid);
pid_t ms_daemon_get_pid(const char *pid_file);
int ms_daemon_clean_pid(const char *pid_file);

#ifdef __cpluscplus
}
#endif

#endif
