#ifndef _MS_ERRNO_H
#define _MS_ERRNO_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#define MS_ERROR_LIST_SIZE 135

int ms_errno_init(void);
const char *ms_errno_str(int err);
void ms_errno_destory(void);

#ifdef __cpluscplus
}
#endif

#endif
