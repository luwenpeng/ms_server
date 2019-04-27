#ifndef _MS_ACCLOG_H
#define _MS_ACCLOG_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_errlog.h"

int ms_acclog_init(const char *file);
void ms_acclog(const char *fmt, ...);
void ms_acclog_close(void);
int ms_acclog_reopen(void);

#ifdef __cpluscplus
}
#endif

#endif
