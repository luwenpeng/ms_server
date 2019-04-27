#ifndef _MS_STR_H
#define _MS_STR_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#define MS_INT32_LEN        (sizeof("-2147483648") - 1)
#define MS_INT64_LEN        (sizeof("-9223372036854775808") - 1)
#define MS_MAX_UINT32_VALUE (uint32_t) 0xffffffff

#define ms_char_tolower(c) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define ms_char_toupper(c) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

#define ms_str_ncpymem(dst, src, n) ((char *)memcpy(dst, src, n) + (n))

void ms_str_tolower(char *dst, const char *src, size_t n);
void ms_str_toupper(char *dst, const char *src, size_t n);

size_t ms_str_len(const char *p, size_t n);
int ms_str_ncmp(const char *s1, const char *s2, size_t n);

char *ms_str_snprintf(char *buf, size_t max, const char *fmt, ...);
char *ms_str_slprintf(char *buf, const char *last, const char *fmt, ...);

char *ms_str_vslprintf(char *buf, const char *last, const char *fmt,
        va_list args);

#ifdef __cpluscplus
}
#endif

#endif
