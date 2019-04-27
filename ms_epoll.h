#ifndef _MS_EPOLL_H
#define _MS_EPOLL_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_errlog.h"

int ms_epoll_create(int size);
int ms_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int ms_epoll_wait(int epfd, struct epoll_event *events, int maxevents,
        int timeout);
void ms_epoll_close(int epfd);

#ifdef __cpluscplus
}
#endif

#endif
