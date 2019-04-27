#include "ms_errno.h"

static char ms_errno_unknown[] = "Unknown error";
static char *ms_errno_list[MS_ERROR_LIST_SIZE] = { NULL };

/***********************************************************
 * @Func   : ms_errno_init()
 * @Author : lwp
 * @Brief  : 初始化 ms_errno_list。
 * @Param  : [in] NONE
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   :
 ***********************************************************/
int ms_errno_init(void)
{
    char  *p = NULL;
    char  *msg = NULL;
    size_t len = 0;

    for (int i = 0; i < MS_ERROR_LIST_SIZE; i++)
    {
        msg = strerror(i);
        len = strlen(msg);

        // 只需要 len 就够了，1 为 '\0'，防止输出时越界
        p = (char *)malloc(len + 1);
        if (NULL == p)
        {
            printf("[error] malloc(\"%lu\") failed, (\"%d\" : \"%s\")\n", len,
                    errno, strerror(errno));
            return MS_ERROR;
        }

        memset(p, 0, len + 1);
        memcpy(p, msg, len);
        ms_errno_list[i] = p;
    }

    return MS_OK;
}
// @ms_errno_init() ok

/***********************************************************
 * @Func   : ms_errno_str()
 * @Author : lwp
 * @Brief  : 返回 err 错误码对应的错误描述信息。
 * @Param  : [in] err : 错误码
 * @Return : err 错误码对应的错误描述信息
 * @Note   :
 ***********************************************************/
const char *ms_errno_str(int err)
{
    char *msg = NULL;

    if (0 <= err && err < MS_ERROR_LIST_SIZE)
    {
        msg = ms_errno_list[err];
    }
    else
    {
        msg = ms_errno_unknown;
    }

    return msg;
}
// @ms_errno_str() ok

/***********************************************************
 * @Func   : ms_errno_destory()
 * @Author : lwp
 * @Brief  : 销毁 ms_errno_list。
 * @Param  : [in] NONE
 * @Return : NONE
 * @Note   :
 ***********************************************************/
void ms_errno_destory(void)
{
    if (NULL == ms_errno_list)
    {
        return;
    }

    for (int err = 0; err < MS_ERROR_LIST_SIZE; err++)
    {
        if (ms_errno_list[err])
        {
            free(ms_errno_list[err]);
            ms_errno_list[err] = NULL;
        }
    }
}
// @ms_errno_destory() ok
