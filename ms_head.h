#ifndef _MS_HEAD_H
#define _MS_HEAD_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include <openssl/evp.h>
#include <openssl/hmac.h>

#ifdef __cpluscplus
}
#endif

#endif
