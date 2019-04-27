#include "ms_str.h"

static char *ms_str_sprintf_num(char *buf, const char *last, uint64_t ui64,
        char zero, uintptr_t hexadecimal, uintptr_t width);

/***********************************************************
 * @Func   : ms_str_tolower()
 * @Author : lwp
 * @Brief  : 将 src 中长度为 n 的字符串转换成小写存储到 dst 中。
 * @Param  : [in] dst : 输出缓冲区地址
 * @Param  : [in] src : 输入缓冲区地址
 * @Param  : [in] n   : 输入字符串的长度
 * @Return : NONE
 * @Note   :
 ***********************************************************/
void ms_str_tolower(char *dst, const char *src, size_t n)
{
    while (n)
    {
        *dst = ms_char_tolower(*src);
        dst++;
        src++;
        n--;
    }
}
// @ms_str_tolower() ok

/***********************************************************
 * @Func   : ms_str_toupper()
 * @Author : lwp
 * @Brief  : 将 src 中长度为 n 的字符串转换成大写存储到 dst 中。
 * @Param  : [in] dst : 输出缓冲区地址
 * @Param  : [in] src : 输入缓冲区地址
 * @Param  : [in] n   : 输入字符串的长度
 * @Return : NONE
 * @Note   :
 ***********************************************************/
void ms_str_toupper(char *dst, const char *src, size_t n)
{
    while (n)
    {
        *dst = ms_char_toupper(*src);
        dst++;
        src++;
        n--;
    }
}
// @ms_str_toupper() ok

/***********************************************************
 * @Func   : ms_str_len()
 * @Author : lwp
 * @Brief  : 计算存储空间为 n 的字符串 p 中有效数据的长度。
 * @Param  : [in] p : 缓冲区的起始地址
 * @Param  : [in] n : 缓冲区的长度
 * @Return : p 中有效数据的长度
 * @Note   :
 ***********************************************************/
size_t ms_str_len(const char *p, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (p[i] == '\0')
        {
            return i;
        }
    }

    return n;
}
// @ms_str_len() ok

/***********************************************************
 * @Func   : ms_str_ncmp()
 * @Author : lwp
 * @Brief  : 对比 s1 与 s2 的前 n 个字符是否相同。
 * @Param  : [in] s1
 * @Param  : [in] s2
 * @Param  : [in] n
 * @Return :  0 : s1 的前 n 个字符 = s2 的前 n 个字符
 *           <0 : s1 的前 n 个字符 < s2 的前 n 个字符
 *           >0 : s1 的前 n 个字符 > s2 的前 n 个字符
 * @Note   : 
 ***********************************************************/
int ms_str_ncmp(const char *s1, const char *s2, size_t n)
{
    int c1, c2;

    while (n)
    {
        c1 = (int) *s1++;
        c2 = (int) *s2++;

        c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
        c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

        if (c1 == c2)
        {
            if (c1)
            {
                n--;
                continue;
            }

            return 0;
        }
        return c1 - c2;
    }

    return 0;
}
// @ms_str_ncmp() ok

/***********************************************************
 * @Func   : ms_str_snprintf
 * @Author : lwp
 * @Brief  : 将 fmt 格式的可变参数写入有界 buf 中。
 * @Param  : [in] buf : 缓冲区的起始地址
 * @Param  : [in] max : 缓冲区的长度
 * @Param  : [in] fmt : 输出格式
 * @Param  : [in] ... : 可变参数
 * @Return : buf 写入数据后的地址
 * @Note   : 若 buf 空间不足，数据会被截断
 ***********************************************************/
char *ms_str_snprintf(char *buf, size_t max, const char *fmt, ...)
{
    char *p;
    va_list args;

    va_start(args, fmt);
    p = ms_str_vslprintf(buf, buf + max, fmt, args);
    va_end(args);

    return p;
}
// @ms_str_snprintf() ok

/***********************************************************
 * @Func   : ms_str_slprintf
 * @Author : lwp
 * @Brief  : 将 fmt 格式的可变参数写入有界 buf 中。
 * @Param  : [in] buf : 缓冲区的起始地址
 * @Param  : [in] last : 缓冲区的结束地址
 * @Param  : [in] fmt : 输出格式
 * @Param  : [in] ... : 可变参数
 * @Return : buf 写入数据后的地址
 * @Note   : 若 buf 空间不足，数据会被截断
 ***********************************************************/
char *ms_str_slprintf(char *buf, const char *last, const char *fmt, ...)
{
    char *p;
    va_list args;

    va_start(args, fmt);
    p = ms_str_vslprintf(buf, last, fmt, args);
    va_end(args);

    return p;
}
// @ms_str_slprintf() ok

/* supported formats:
 *#     %[0][width][u][x|X]d    int/u_int
 *#     %[0][width][u][x|X]l    long
 *#     %[0][width][u][x|X]z    ssize_t/size_t
 *#     %[0][width][u][x|X]D    int32_t/uint32_t
 *#     %[0][width][u][x|X]L    int64_t/uint64_t
 *#     %[0][width|m][u][x|X]i  intptr_t/uintptr_t
 *#     %[0][width][.width]f    double, max valid number fits to %18.15f
 *#     %P                      pid_t
 *#     %p                      void *
 *#     %s                      null-terminated string
 *#     %*s                     length and string
 *#     %c                      char
 *#     %%                      %
 */

/***********************************************************
 * @Func   : ms_str_vslprintf()
 * @Author : lwp
 * @Brief  : ms_str_vslprintf() 根据 fmt 格式输出字符串到 buf 中。
 * @Param  : [in] buf : buf 的起始地址
 * @Param  : [in] last : buf 的结束地址
 * @Param  : [in] fmt : 输出格式
 * @Param  : [in] args : 参数
 * @Return : buf 追加数据后的地址
 * @Note   :
 ***********************************************************/
char *ms_str_vslprintf(char *buf, const char *last, const char *fmt,
        va_list args)
{
    char *p, zero;
    int d;
    double f;
    size_t len, slen;
    int64_t i64;
    uint64_t ui64, frac;
    uintptr_t width, sign, hex, max_width, frac_width, scale, n;

    while (*fmt && buf < last)
    {
        /* "buf < last" means that we could copy at least one character:
         * the plain character, "%%", "%c", and minus without the checking
         */
        if (*fmt == '%')
        {
            // 前置补 0 or 前置补 ' '
            zero = (*++fmt == '0') ? '0' : ' ';
            i64 = 0;
            ui64 = 0;
            width = 0;
            sign = 1;
            hex = 0;
            max_width = 0;
            frac_width = 0;
            slen = (size_t) -1;

            // 设置前置补 0 或 ' ' 的个数
            while (*fmt >= '0' && *fmt <= '9')
            {
                width = width * 10 + *fmt++ - '0';
            }

            for ( ;; )
            {
                switch (*fmt)
                {
                    case 'u':
                        sign = 0;
                        fmt++;
                        continue;
                    case 'm':
                        max_width = 1;
                        fmt++;
                        continue;
                    case 'X':
                        hex = 2;
                        sign = 0;
                        fmt++;
                        continue;
                    case 'x':
                        hex = 1;
                        sign = 0;
                        fmt++;
                        continue;
                    case '.':
                        fmt++;
                        // 设置小数点后有效数据的位数
                        while (*fmt >= '0' && *fmt <= '9')
                        {
                            frac_width = frac_width * 10 + *fmt++ - '0';
                        }
                        break;
                    case '*':
                        slen = va_arg(args, size_t);
                        fmt++;
                        continue;
                    default:
                        break;
                }
                break;
            } // end for (;;)

            switch (*fmt)
            {
                case 's':
                    p = va_arg(args, char *);
                    if (slen == (size_t) -1)
                    {
                        while (*p && buf < last)
                        {
                            *buf++ = *p++;
                        }
                    }
                    else
                    {
                        len = ms_min(((size_t) (last - buf)), slen);
                        buf = ms_str_ncpymem(buf, p, len);
                    }
                    fmt++;
                    continue;
                case 'P':
                    i64 = (int64_t) va_arg(args, pid_t);
                    sign = 1;
                    break;
                case 'z':
                    if (sign)
                    {
                        i64 = (int64_t) va_arg(args, ssize_t);
                    }
                    else
                    {
                        ui64 = (uint64_t) va_arg(args, size_t);
                    }
                    break;
                case 'i':
                    if (sign)
                    {
                        i64 = (int64_t) va_arg(args, intptr_t);
                    }
                    else
                    {
                        ui64 = (uint64_t) va_arg(args, uintptr_t);
                    }

                    if (max_width)
                    {
                        width = MS_INT32_LEN;
                    }
                    break;
                case 'd':
                    if (sign)
                    {
                        i64 = (int64_t) va_arg(args, int);
                    }
                    else
                    {
                        ui64 = (uint64_t) va_arg(args, u_int);
                    }
                    break;
                case 'l':
                    if (sign)
                    {
                        i64 = (int64_t) va_arg(args, long);
                    }
                    else
                    {
                        ui64 = (uint64_t) va_arg(args, u_long);
                    }
                    break;
                case 'D':
                    if (sign)
                    {
                        i64 = (int64_t) va_arg(args, int32_t);
                    }
                    else
                    {
                        ui64 = (uint64_t) va_arg(args, uint32_t);
                    }
                    break;
                case 'L':
                    if (sign)
                    {
                        i64 = va_arg(args, int64_t);
                    }
                    else
                    {
                        ui64 = va_arg(args, uint64_t);
                    }
                    break;
                case 'f':
                    f = va_arg(args, double);
                    // 将负数转换成正数再处理
                    if (f < 0)
                    {
                        *buf++ = '-';
                        f = -f;
                    }

                    // 将 double 转换成 int64_t
                    ui64 = (int64_t) f;
                    frac = 0;

                    // width 为整数部分有效数据的位数
                    // frac_width 为小数部分有效数据的位数

                    if (frac_width)
                    {
                        scale = 1;
                        for (n = frac_width; n; n--)
                        {
                            scale *= 10;
                        }

                        // 将小数转换成整数，缓存小数部分
                        frac = (uint64_t) ((f - (double) ui64) * scale + 0.5);
                        if (frac == scale)
                        {
                            ui64++;
                            frac = 0;
                        }
                    }
                    // 将整数部分转换成字符串(width 位有效数字)
                    buf = ms_str_sprintf_num(buf, last, ui64, zero, 0, width);

                    if (frac_width)
                    {
                        if (buf < last)
                        {
                            *buf++ = '.';
                        }
                        // 将小数部分转换成字符串(frac_width 位有效数字且前置填充 0)
                        buf = ms_str_sprintf_num(buf, last, frac, '0', 0, frac_width);
                    }
                    fmt++;
                    continue;
                case 'p':
                    ui64 = (uintptr_t) va_arg(args, void *);
                    hex = 2;
                    sign = 0;
                    zero = '0';
                    width = 2 * sizeof(void *);
                    break;
                case 'c':
                    d = va_arg(args, int);
                    *buf++ = d & 0xff;
                    fmt++;
                    continue;
                case '%':
                    *buf++ = '%';
                    fmt++;
                    continue;
                default:
                    *buf++ = *fmt++;
                    continue;
            }

            if (sign)
            {
                if (i64 < 0)
                {
                    // 将负数转换成正数再处理
                    *buf++ = '-';
                    ui64 = (uint64_t) -i64;
                }
                else
                {
                    // 将 signed 的转换成 unsigned 的再处理
                    ui64 = (uint64_t) i64;
                }
            }
            buf = ms_str_sprintf_num(buf, last, ui64, zero, hex, width);
            fmt++;
        } // end if (*fmt == '%')
        else
        {
            *buf++ = *fmt++;
        }
    } // end while()

    return buf;
}
// @ms_str_vslprintf() ok

/***********************************************************
 * @Func   : ms_str_sprintf_num()
 * @Author : lwp
 * @Brief  : 将无符号整形按照指定格式转换成字符串存储到 buf 中。
 * @Param  : [in] buf : buf 的起始地址
 * @Param  : [in] last : buf 的结束地址
 * @Param  : [in] ui64 : unsigned int64
 * @Param  : [in] zero : 前置补位的字符，通常位 '0' 或 ' '
 * @Param  : [in] hexadecimal : 0 : 转换成 '10进制' 字符串
 *                              1 : 转换成 '小写16进制' 字符串
 *                              2 : 转换成 '大写16进制' 字符串
 * @Param  : [in] width : 若 ui64 转换成字符串有效数据位不足 width 位，
 ×                        则使用 zero 字符前置补位，补满 width 位
 * @Return : buf 追加数据后的地址
 * @Note   : 若 buf 空间不足数据会被截断
 ***********************************************************/
static char *ms_str_sprintf_num(char *buf, const char *last, uint64_t ui64,
        char zero, uintptr_t hexadecimal, uintptr_t width)
{
    // we need temp[MS_INT64_LEN] only, but icc issues the warning
    char *p, temp[MS_INT64_LEN + 1];
    size_t len;
    uint32_t ui32;
    static char hex[] = "0123456789abcdef";
    static char HEX[] = "0123456789ABCDEF";

    p = temp + MS_INT64_LEN;

    /* 将 ui64 逐位转换成字符写入 temp 中，注意是从 temp 尾部向 temp 首部写入。
     * 例如：将 12345 整形的 ‘5’，‘4’，‘3’，‘2’，‘1’，逐步写入到
     * temp[MS_INT64_LEN], temp[MS_INT64_LEN - 1], temp[MS_INT64_LEN - 2],
     * temp[MS_INT64_LEN - 3], temp[MS_INT64_LEN - 4] 中
     */
    // 转换成 10 进制
    if (hexadecimal == 0)
    {
        // 按照 ui32 处理
        if (ui64 <= (uint64_t) MS_MAX_UINT32_VALUE)
        {
            ui32 = (uint32_t) ui64;
            do {
                *--p = ui32 % 10 + '0';
            } while (ui32 /= 10);
        }
        // 按照 ui64处理
        else
        {
            do {
                *--p = ui64 % 10 + '0';
            } while (ui64 /= 10);
        }
    }
    // 转换成小写 16 进制
    else if (hexadecimal == 1)
    {
        do {
            // (uint32_t) 强制关闭了 BCC 的警告
            *--p = hex[(uint32_t) (ui64 & 0xf)];
        } while (ui64 >>= 4);
    }
    // 转化成大写 16 进制
    else
    {
        do {
            // (uint32_t) 强制关闭了 BCC 的警告
            *--p = HEX[(uint32_t) (ui64 & 0xf)];
        } while (ui64 >>= 4);
    }

    // ui64 转换成字符串所占的内存空间
    len = (temp + MS_INT64_LEN) - p;

    // 填充 (width - len个) 0 或者空格
    while (len++ < width && buf < last)
    {
        *buf++ = zero;
    }

    // 安全拷贝，若空间不足则会被截断
    len = (temp + MS_INT64_LEN) - p;
    if (buf + len > last)
    {
        len = last - buf;
    }

    return ms_str_ncpymem(buf, p, len);
}
// @ms_str_sprintf_num() ok
