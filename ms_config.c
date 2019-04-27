#include "ms_config.h"

//#define goto printf("return in : %d\n", __LINE__); goto

static ms_conf_item_t *g_sys_conf = NULL;
static int g_conf_size = -1;

/***********************************************************
 * @Func   : ms_config_parse()
 * @Author : lwp
 * @Brief  : 解析 config 配置文件。
 * @Param  : [in] config
 * @Param  : [in] sys_conf
 * @Param  : [in] conf_size
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_config_parse(const char *config, ms_conf_item_t *sys_conf, int conf_size)
{
    int i = 0;
    int len = 0;
    int error = 0;
    char *p = NULL;
    char *pc = NULL;
    FILE *fp = NULL;
    char key[MS_MAX_BUF_SIZE];
    char val[MS_MAX_BUF_SIZE];
    char buff[MS_MAX_BUF_SIZE];

    if (config == NULL)
    {
        ms_errlog_stderr(0, "config file path is NULL");
        return MS_ERROR;
    }

    if (sys_conf == NULL)
    {
        ms_errlog_stderr(0, "sys conf point is NULL");
        return MS_ERROR;
    }

    if (conf_size < 1)
    {
        ms_errlog_stderr(0, "sys conf size is invalid");
        return MS_ERROR;
    }

    fp = fopen(config, "r");
    if (fp == NULL)
    {
        ms_errlog_stderr(errno, "fopen(\"%s\") failed", config);
        return MS_ERROR;
    }

    g_sys_conf = sys_conf;
    g_conf_size = conf_size;

    while (fgets(buff, sizeof(buff) - 1, fp))
    {
        if (strlen(buff) == sizeof(buff) - 2)
        {
            error++;
            ms_errlog_stderr(0, "config line data too long (0, %d)",
                    sizeof(buff) - 2);
            break;
        }
        p = buff;

        // 跳过行首的空白字符
        p += strspn(p, WHITE_SPACE);

        // 若本行存在注释字符'#'，将第一个注释字符'#'置换成'\0'，忽略本行'#'以后的字符
        pc = strchr(p, COMMENT_CHAR);
        if (pc)
        {
            *pc = EOS;
        }

        // 若本行无有效字符，则跳到下一行
        if (*p == EOS)
        {
            memset(buff, 0, sizeof(buff));
            continue;
        }

        // 获取配置项
        memset(&key, 0, sizeof(key));
        len = strcspn(p, WHITE_SPACE);
        strncpy(key, p, len);
        key[len] = EOS;
        p += len;

        // 跳过指令名与指令值之间的字符 ", \t\n\r\f"
        p += strspn(p, DELIMS);
        if (*p == EOS)
        {
            error++;
            ms_errlog_stderr(0, "config item \"%s\" no value", key);
            continue;
        }

        // 获取配置项的值
        memset(&val, 0, sizeof(val));
        len = strcspn(p, DELIMS);
        strncpy(val, p, len);
        val[len] = EOS;

        // 检查配置项的名与值是否合法
        for (i = 0; i < g_conf_size; i++)
        {
            if (strcmp(key, g_sys_conf[i].key) == 0)
            {
                if (g_sys_conf[i].check(&g_sys_conf[i], val) == MS_ERROR)
                {
                    error++;
                }
                break;
            }
        }

        // 无效的配置项
        if (i == g_conf_size)
        {
            error++;
            ms_errlog_stderr(0, "config item \"%s\" unexpect", key);
        }

        memset(buff, 0, sizeof(buff));
    }

    // 配置文件中未包含的配置项
    if (error == 0)
    {
        for (i = 0; i < g_conf_size; i++)
        {
            if (strlen(g_sys_conf[i].val) == 0)
            {
                error++;
                ms_errlog_stderr(0, "config item \"%s\" unset",
                        g_sys_conf[i].key);
            }
        }
    }

    fclose(fp);
    return (error ? MS_ERROR : MS_OK);
}
// @ms_config_parse() ok

/***********************************************************
 * @Func   : ms_config_get_value()
 * @Author : lwp
 * @Brief  : 获取指定配置项 key 对应的值。
 * @Param  : [in] key
 * @Return : NULL : key 不存在
 *           !NULL : 返回 key 对应的 val
 * @Note   : 
 ***********************************************************/
char *ms_config_get_value(const char *key)
{
    for (int i = 0; i < g_conf_size; i++)
    {
        if (strcmp(g_sys_conf[i].key, key) == 0)
        {
            return g_sys_conf[i].val;
        }
    }

    return NULL;
}
// @ms_config_get_value() ok

/***********************************************************
 * @Func   : ms_config_debug()
 * @Author : lwp
 * @Brief  : 输出所有配置信息。
 * @Param  : [in] NONE
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_config_debug(void)
{
    for (int i = 0; i < g_conf_size; i++)
    {
        ms_errlog_stdout("get config item \"%s\" : \"%s\"",
                g_sys_conf[i].key, g_sys_conf[i].val);
    }
}
// @ms_config_debug() ok

/***********************************************************
 * @Func   : check_ipv4()
 * @Author : lwp
 * @Brief  : 检查 ipv4 的格式是否正确。
 * @Param  : [in] t : 指向当前配置项的结构体
 * @Param  : [in] data : ip 配置项的值
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int check_ipv4(ms_conf_item_t *t, const char *data)
{
    int len = 0;
    char *ptr = NULL;
    char *ptem = NULL;
    char buff[17] = { 0 };

    if (strlen(t->val))
    {
        ms_errlog_stderr(0, "config item \"%s\" is duplicated", t->key);
        return MS_ERROR;
    }

    // IP 长度检测
    // 1.1.1.1          7位
    // 111.111.111.111 15位
    if (strlen(data) > 15 || strlen(data) < 7)
    {
        goto error;
    }

    // 临时缓存 IP，尾部追加 "."
    strcpy(buff, data);
    buff[strlen(buff)] = '.';
    ptr = buff;

    // 检测 ip[0].ip[1].ip[2].ip[3] 的格式是否正确
    for (int i = 0; i < 4; i++)
    {
        ptem = ptr;
        ptr = strchr(ptr, '.');
        if (ptr)
        {
            *ptr = EOS;
            len = strlen(ptem);
            if (len == 0 || len > 3)
            {
                goto error;
            }

            for (int i = 0; i < len; i++)
            {
                if (ptem[i] < '0' || ptem[i] > '9')
                {
                    goto error;
                }
            }

            if (atoi(ptem) < 0 || atoi(ptem) > 255)
            {
                goto error;
            }
            ptr++;
        }
        else
        {
            goto error;
        }
    }

    memset(t->val, 0, sizeof(t->val));
    strcpy(t->val, data);
    return MS_OK;

error:
    ms_errlog_stderr(0, "config item \"%s\" val \"%s\" is invalied", t->key,
            data);
    return MS_ERROR;
}
// @check_ipv4() ok

/***********************************************************
 * @Func   : check_port()
 * @Author : lwp
 * @Brief  : 检查 port 的格式是否正确。
 * @Param  : [in] t : 指向当前配置项的结构体
 * @Param  : [in] data : port 配置项的值
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int check_port(ms_conf_item_t *t, const char *data)
{
    int len = 0;

    if (strlen(t->val))
    {
        ms_errlog_stderr(0, "config item \"%s\" is duplicated", t->key);
        return MS_ERROR;
    }

    // 长度检测 [1, 5]
    len = strlen(data);
    if (len > 5 || len < 1)
    {
        goto error;
    }

    // 非法字符检测
    for (int i = 0; i < len; i++)
    {
        if (data[i] < '0' || data[i] > '9')
        {
            goto error;
        }
    }

    // 端口范围 [1, 65535]
    if (atoi(data) < 1 || atoi(data) > 65535)
    {
        goto error;
    }

    memset(t->val, 0, sizeof(t->val));
    strcpy(t->val, data);
    return MS_OK;

error:
    ms_errlog_stderr(0, "config item \"%s\" val \"%s\" is invalied", t->key,
            data);
    return MS_ERROR;
}
// @check_port() ok

/***********************************************************
 * @Func   : check_level()
 * @Author : lwp
 * @Brief  : 检查错误日志的级别是否正确。
 * @Param  : [in] t : 指向当前配置项的结构体
 * @Param  : [in] data : error_log_level 配置项的值
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int check_level(ms_conf_item_t *t, const char *data)
{
    int len;
    char level[10][8] = {
        "status", "emerg" , "alert", "crit" , "error",
        "warn"  , "notice", "infor", "debug", "stdout"
    };

    if (strlen(t->val))
    {
        ms_errlog_stderr(0, "config item \"%s\" is duplicated", t->key);
        return MS_ERROR;
    }

    len = strlen(data);
    if (len < 4 || len > 6)
    {
        goto error;
    }

    for (int i = 0; i < 10; i++)
    {
        if (strcmp(data, level[i]) == 0)
        {
            memset(t->val, 0, sizeof(t->val));
            t->val[0] = '0' + i;

            return MS_OK;
        }
    }

error:
    ms_errlog_stderr(0, "config item \"%s\" val \"%s\" is invalied", t->key,
            data);
    return MS_ERROR;
}
// @check_level() ok

/***********************************************************
 * @Func   : check_dir()
 * @Author : lwp
 * @Brief  : 检查目录路径的配置项是否正确。
 * @Param  : [in] t : 指向当前配置项的结构体
 * @Param  : [in] data : 目录路径配置项的值
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int check_dir(ms_conf_item_t *t, const char *data)
{
    int len;
    DIR *dp = NULL;

    if (strlen(t->val))
    {
        ms_errlog_stderr(0, "config item \"%s\" is duplicated", t->key);
        return MS_ERROR;
    }

    // 长度检测
    len = strlen(data);
    if (len < 1)
    {
        ms_errlog_stderr(0, "config item \"%s\" val \"%s\" is invalied",
                t->key, data);
        return MS_ERROR;
    }

    // 绝对路径检测
    if (data[0] != '/')
    {
        ms_errlog_stderr(0, "config item \"%s\" val \"%s\" is invalied, "
                "please use absolute path", t->key, data);
        return MS_ERROR;
    }

    // 是否以 '/' 结尾
    if (data[len - 1] != '/')
    {
        ms_errlog_stderr(0, "config item \"%s\" val \"%s\" is invalied, "
                "please use absolute path and end with \'/\'", t->key, data);
        return MS_ERROR;
    }

    // 是否可以访问目录
    dp = opendir(data);
    if (NULL == dp)
    {
        ms_errlog_stderr(errno, "config item \"%s\" val \"%s\" is invalied, "
                "opendir() failed", t->key, data);
        return MS_ERROR;
    }
    closedir(dp);

    memset(t->val, 0, sizeof(t->val));
    strcpy(t->val, data);

    return MS_OK;
}
// @check_dir() ok

/***********************************************************
 * @Func   : check_file()
 * @Author : lwp
 * @Brief  : 检查文件路径的配置项是否正确。
 * @Param  : [in] t : 指向当前配置项的结构体
 * @Param  : [in] data : 文件路径配置项的值
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int check_file(ms_conf_item_t *t, const char *data)
{
    int len;
    int fd = -1;

    if (strlen(t->val))
    {
        ms_errlog_stderr(0, "config item \"%s\" is duplicated", t->key);
        return MS_ERROR;
    }

    // 长度检测
    len = strlen(data);
    if (len < 2)
    {
        ms_errlog_stderr(0, "config item \"%s\" val \"%s\" is invalied",
                t->key, data);
        return MS_ERROR;
    }

    // 绝对路径检测
    if (data[0] != '/')
    {
        ms_errlog_stderr(0, "config item \"%s\" val \"%s\" is invalied, "
                "please use absolute path", t->key, data);
        return MS_ERROR;
    }

    memset(t->val, 0, sizeof(t->val));
    strcpy(t->val, data);

    // 检查是否可以读写文件
    fd = open(t->val, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1)
    {
        ms_errlog_stderr(errno, "config item \"%s\" val \"%s\" is invalied, "
                "open() failed", t->key, data);
        return MS_ERROR;
    }

    return MS_OK;
}
// @check_file() ok

/***********************************************************
 * @Func   : check_num()
 * @Author : lwp
 * @Brief  : 检查数值配置项是否正确
 * @Param  : [in] t : 指向当前配置项的结构体
 * @Param  : [in] data : 数值配置项的值
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int check_num(ms_conf_item_t *t, const char *data)
{
    unsigned int len;

    if (strlen(t->val))
    {
        ms_errlog_stderr(0, "config item \"%s\" is duplicated", t->key);
        return MS_ERROR;
    }

    len = strlen(data);
    if (len < 1 || len > strlen("2147483647"))
    {
        goto error;
    }

    for (unsigned int i = 0; i < len; i++)
    {
        if (data[i] < '0' || data[i] > '9')
        {
            goto error;
        }
    }

    if (atoll(data) < 0 || atoll(data) > 2147483647)
    {
        goto error;
    }

    memset(t->val, 0, sizeof(t->val));
    strcpy(t->val, data);
    return MS_OK;

error:
    ms_errlog_stderr(0,
            "config item \"%s\" val \"%s\" is invalied (0 ~ 2147483647)",
            t->key, data);
    return MS_ERROR;
}
// @check_num() ok

/***********************************************************
 * @Func   : check_str()
 * @Author : lwp
 * @Brief  : 检查字符串配置项是否正确
 * @Param  : [in] t : 指向当前配置项的结构体
 * @Param  : [in] data : 数值配置项的值
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int check_str(ms_conf_item_t *t, const char *data)
{
    if (strlen(t->val))
    {
        ms_errlog_stderr(0, "config item \"%s\" is duplicated", t->key);
        return MS_ERROR;
    }

    memset(t->val, 0, sizeof(t->val));
    strcpy(t->val, data);

    return MS_OK;
}
// @check_str() ok
