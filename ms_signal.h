#ifndef _MS_SIGNAL_H
#define _MS_SIGNAL_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_errlog.h"

typedef struct ms_signal_s {
    int    signal;
    void (*handler)(int signal);
} ms_signal_t;

const char *ms_signal_toname(int signal);
int ms_signal_init(ms_signal_t *signal_t);
int ms_signal_register(int *signals);

#ifdef __cpluscplus
}
#endif

#endif
