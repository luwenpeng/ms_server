#ifndef _MS_RLIMIT_H
#define _MS_RLIMIT_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_errlog.h"

int ms_rlimit_set(rlim_t max_process_open_files);

#ifdef __cpluscplus
}
#endif

#endif
