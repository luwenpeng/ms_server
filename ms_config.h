// 原则：配置文件解析属于系统初始化阶段，为便于用户察觉，出现错误会输出到控制台。

#ifndef _MS_CONFIG_H
#define _MS_CONFIG_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_errlog.h"

#define EOS          '\0'
#define COMMENT_CHAR '#'
#define WHITE_SPACE  " \t\n\r\f"
#define DELIMS       ", \t\n\r\f"

typedef struct ms_conf_item_s ms_conf_item_t;

struct ms_conf_item_s {
    char key[MS_MAX_BUF_SIZE];
    char val[MS_MAX_BUF_SIZE];
    int (*check)(ms_conf_item_t *t, const char *data);
};

int ms_config_parse(const char *config, ms_conf_item_t *sys_conf, int conf_size);
char *ms_config_get_value(const char *key);
void ms_config_debug(void);

int check_ipv4(ms_conf_item_t *t, const char *data);
int check_port(ms_conf_item_t *t, const char *data);
int check_file(ms_conf_item_t *t, const char *data);
int check_dir(ms_conf_item_t *t, const char *data);
int check_level(ms_conf_item_t *t, const char *data);
int check_num(ms_conf_item_t *t, const char *data);
int check_str(ms_conf_item_t *t, const char *data);

#ifdef __cpluscplus
}
#endif

#endif
